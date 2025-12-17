#include <io/stream/tcp_client.hpp>
#include <scheduler/socket.hpp>

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, tcp_client);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, tcp_client)

namespace io {

TcpClientStream::TcpClientStream(std::string id, TcpClientConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", host=\"%s\", port=%u, path=\"%s\", reconnect=%d", mId.c_str(),
                     mConfig.host.c_str(), mConfig.port, mConfig.path.c_str(), mConfig.reconnect);
}

TcpClientStream::~TcpClientStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool TcpClientStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;
    mState     = State::Connecting;

    if (!mConfig.path.empty()) {
        INFOF("connecting to unix socket: %s", mConfig.path.c_str());
        mConnectTask.reset(new scheduler::TcpConnectTask(mConfig.path, mConfig.reconnect));
    } else {
        INFOF("connecting to %s:%u", mConfig.host.c_str(), mConfig.port);
        mConnectTask.reset(
            new scheduler::TcpConnectTask(mConfig.host, mConfig.port, mConfig.reconnect));
    }

    mConnectTask->on_connected = [this](scheduler::TcpConnectTask&) {
        INFOF("tcp client connected");
        mState = State::Connected;
    };

    mConnectTask->on_disconnected = [this](scheduler::TcpConnectTask&) {
        INFOF("tcp client disconnected");
        flush_read_buffer();
        if (mConfig.reconnect) {
            mState = State::Connecting;
        } else {
            set_disconnected();
        }
    };

    mConnectTask->on_read = [this](scheduler::TcpConnectTask& task) {
        auto result = ::read(task.fd(), mReadBuf, sizeof(mReadBuf));
        VERBOSEF("::read(%d, %p, %zu) = %zd", task.fd(), mReadBuf, sizeof(mReadBuf), result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result == 0) {
            DEBUGF("tcp connection closed by peer");
            flush_read_buffer();
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ERRORF("failed to read from socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };

    mConnectTask->on_write = [this](scheduler::TcpConnectTask& task) {
        while (!mWriteBuffer.empty()) {
            auto [data, len] = mWriteBuffer.peek();
            auto result      = ::write(task.fd(), data, len);
            VERBOSEF("::write(%d, %p, %zu) = %zd", task.fd(), data, len, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
            mWriteBuffer.consume(result);
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mScheduler->update_epoll_fd(task.fd(), EPOLLIN, nullptr);
            mWriteRegistered = false;
        }
    };

    if (!mConnectTask->schedule(scheduler)) {
        ERRORF("failed to schedule TCP connect task");
        set_error(errno, "failed to schedule TCP connect");
        return false;
    }

    return schedule_read_timeout(scheduler);
}

bool TcpClientStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mConnectTask) {
        mConnectTask->cancel();
        mConnectTask.reset();
    }
    return true;
}

void TcpClientStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    if (!mConnectTask || mState != State::Connected) {
        WARNF("cannot write: not connected");
        return;
    }

    int fd = mConnectTask->fd();
    if (mWriteBuffer.empty()) {
        auto result = ::write(fd, data, length);
        VERBOSEF("::write(%d, %p, %zu) = %zd", fd, data, length, result);
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
        mScheduler->update_epoll_fd(fd, EPOLLIN | EPOLLOUT, nullptr);
        mWriteRegistered = true;
    }
}

}  // namespace io
