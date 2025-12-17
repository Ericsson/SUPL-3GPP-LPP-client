#include <io/stream/udp_client.hpp>
#include <scheduler/socket.hpp>

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, udp_client);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, udp_client)

namespace io {

UdpClientStream::UdpClientStream(std::string id, UdpClientConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", host=\"%s\", port=%u, path=\"%s\"", mId.c_str(), mConfig.host.c_str(),
                     mConfig.port, mConfig.path.c_str());
}

UdpClientStream::~UdpClientStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool UdpClientStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    if (!mConfig.path.empty()) {
        INFOF("connecting to unix datagram socket: %s", mConfig.path.c_str());

        mFd = ::socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0);
        VERBOSEF("::socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0) = %d", mFd);
        if (mFd < 0) {
            ERRORF("failed to create socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, "failed to create socket");
            return false;
        }

        struct sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, mConfig.path.c_str(), sizeof(addr.sun_path) - 1);

        auto result = ::connect(mFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        VERBOSEF("::connect(%d, %p, %zu) = %d", mFd, &addr, sizeof(addr), result);
        if (result < 0) {
            ERRORF("failed to connect: " ERRNO_FMT, ERRNO_ARGS(errno));
            auto close_result = ::close(mFd);
            VERBOSEF("::close(%d) = %d", mFd, close_result);
            mFd = -1;
            set_error(errno, "failed to connect");
            return false;
        }
    } else {
        INFOF("connecting to %s:%u", mConfig.host.c_str(), mConfig.port);

        struct addrinfo hints{};
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        struct addrinfo* res      = nullptr;
        auto             port_str = std::to_string(mConfig.port);
        auto result = ::getaddrinfo(mConfig.host.c_str(), port_str.c_str(), &hints, &res);
        VERBOSEF("::getaddrinfo(\"%s\", \"%s\", %p, %p) = %d", mConfig.host.c_str(),
                 port_str.c_str(), &hints, &res, result);
        if (result != 0) {
            ERRORF("failed to resolve host: %s", gai_strerror(result));
            set_error(result, "failed to resolve host");
            return false;
        }

        mFd = ::socket(res->ai_family, res->ai_socktype | SOCK_NONBLOCK, res->ai_protocol);
        VERBOSEF("::socket(%d, %d, %d) = %d", res->ai_family, res->ai_socktype | SOCK_NONBLOCK,
                 res->ai_protocol, mFd);
        if (mFd < 0) {
            ERRORF("failed to create socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            ::freeaddrinfo(res);
            set_error(errno, "failed to create socket");
            return false;
        }

        result = ::connect(mFd, res->ai_addr, res->ai_addrlen);
        VERBOSEF("::connect(%d, %p, %u) = %d", mFd, res->ai_addr, res->ai_addrlen, result);
        ::freeaddrinfo(res);

        if (result < 0) {
            ERRORF("failed to connect: " ERRNO_FMT, ERRNO_ARGS(errno));
            auto close_result = ::close(mFd);
            VERBOSEF("::close(%d) = %d", mFd, close_result);
            mFd = -1;
            set_error(errno, "failed to connect");
            return false;
        }
    }

    mSocketTask.reset(new scheduler::SocketTask(mFd));
    mSocketTask->set_event_name("udp-client:" + mId);
    mSocketTask->on_read = [this](scheduler::SocketTask&) {
        auto result = ::recv(mFd, mReadBuf, sizeof(mReadBuf), 0);
        VERBOSEF("::recv(%d, %p, %zu, 0) = %zd", mFd, mReadBuf, sizeof(mReadBuf), result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };
    mSocketTask->on_write = [this](scheduler::SocketTask&) {
        while (!mWriteBuffer.empty()) {
            auto [data, len] = mWriteBuffer.peek();
            auto result      = ::send(mFd, data, len, MSG_NOSIGNAL);
            VERBOSEF("::send(%d, %p, %zu, MSG_NOSIGNAL) = %zd", mFd, data, len, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
            mWriteBuffer.consume(result);
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mScheduler->update_epoll_fd(mFd, EPOLLIN, nullptr);
            mWriteRegistered = false;
        }
    };
    mSocketTask->on_error = [this](scheduler::SocketTask&) {
        ERRORF("socket error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, strerror(errno));
    };

    if (!mSocketTask->schedule(scheduler)) {
        ERRORF("failed to schedule socket task");
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    if (!schedule_read_timeout(scheduler)) {
        mSocketTask->cancel();
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
        return false;
    }

    mState = State::Connected;
    DEBUGF("udp client connected");
    return true;
}

bool UdpClientStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mSocketTask) {
        mSocketTask->cancel();
        mSocketTask.reset();
    }
    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
    return true;
}

void UdpClientStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    if (mWriteBuffer.empty()) {
        auto result = ::send(mFd, data, length, MSG_NOSIGNAL);
        VERBOSEF("::send(%d, %p, %zu, MSG_NOSIGNAL) = %zd", mFd, data, length, result);
        if (result < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                result = 0;
            } else {
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
        }
        if (static_cast<size_t>(result) == length) return;
        data += result;
        length -= result;
    }

    mWriteBuffer.enqueue(data, length);
    if (!mWriteRegistered && mScheduler) {
        mScheduler->update_epoll_fd(mFd, EPOLLIN | EPOLLOUT, nullptr);
        mWriteRegistered = true;
    }
}

}  // namespace io
