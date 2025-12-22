#include <cxx11_compat.hpp>
#include <io/stream/tcp_server.hpp>
#include <scheduler/socket.hpp>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, tcp_server);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, tcp_server)

namespace io {

TcpServerStream::Client::Client(TcpServerStream& server, int fd) NOEXCEPT : mServer(server),
                                                                            mFd(fd),
                                                                            mTask(fd) {
    mTask.set_event_name("tcp-server-client:" + server.mId);
    mTask.on_read = [this](scheduler::OwnedFileDescriptorTask&) {
        if (!mDestroying) on_read();
    };
    mTask.on_write = [this](scheduler::OwnedFileDescriptorTask&) {
        if (!mDestroying) on_write();
    };
    mTask.on_error = [this](scheduler::OwnedFileDescriptorTask&) {
        if (!mDestroying) on_error();
    };
    (void)mTask.schedule();
}

TcpServerStream::Client::~Client() NOEXCEPT {
    FUNCTION_SCOPEF("fd=%d", mFd);
    destroy();
}

void TcpServerStream::Client::destroy() NOEXCEPT {
    FUNCTION_SCOPEF("fd=%d", mFd);
    if (mDestroying) return;
    mDestroying = true;

    auto* server = &mServer;
    auto  fd     = mFd;
    scheduler::defer([server, fd](scheduler::Scheduler&) {
        server->remove_client(fd);
    });
}

void TcpServerStream::Client::on_read() NOEXCEPT {
    FUNCTION_SCOPEF("fd=%d", mFd);
    auto result = ::read(mFd, mServer.mReadBuf, sizeof(mServer.mReadBuf));
    VERBOSEF("::read(%d, %p, %zu) = %zd", mFd, mServer.mReadBuf, sizeof(mServer.mReadBuf), result);
    if (result > 0) {
        mServer.on_raw_read(mServer.mReadBuf, result);
    } else if (result == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
        if (result == 0) {
            DEBUGF("client fd=%d disconnected", mFd);
        } else {
            WARNF("client fd=%d read error: " ERRNO_FMT, mFd, ERRNO_ARGS(errno));
        }
        destroy();
    }
}

void TcpServerStream::Client::on_write() NOEXCEPT {
    FUNCTION_SCOPEF("fd=%d", mFd);
    while (!mWriteBuffer.empty()) {
        auto [data, len] = mWriteBuffer.peek();
        auto result      = ::write(mFd, data, len);
        VERBOSEF("::write(%d, %p, %zu) = %zd", mFd, data, len, result);
        if (result < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            WARNF("client fd=%d write error: " ERRNO_FMT, mFd, ERRNO_ARGS(errno));
            destroy();
            return;
        }
        mWriteBuffer.consume(result);
    }
    if (mWriteBuffer.empty() && mWriteRegistered) {
        VERBOSEF("client fd=%d write buffer drained", mFd);
        mTask.update_interests(scheduler::EventInterest::Read | scheduler::EventInterest::Error |
                               scheduler::EventInterest::Hangup);
        mWriteRegistered = false;
    }
}

void TcpServerStream::Client::on_error() NOEXCEPT {
    FUNCTION_SCOPEF("fd=%d", mFd);
    int       err    = 0;
    socklen_t errlen = sizeof(err);
    auto      result = ::getsockopt(mFd, SOL_SOCKET, SO_ERROR, &err, &errlen);
    VERBOSEF("::getsockopt(%d, SOL_SOCKET, SO_ERROR, %p, %p) = %d", mFd, &err, &errlen, result);
    WARNF("client fd=%d socket error: %d (%s)", mFd, err, strerror(err));
    destroy();
}

void TcpServerStream::Client::write(uint8_t const* data, size_t length) NOEXCEPT {
    FUNCTION_SCOPEF("fd=%d, length=%zu", mFd, length);
    if (mWriteBuffer.empty()) {
        auto result = ::write(mFd, data, length);
        VERBOSEF("::write(%d, %p, %zu) = %zd", mFd, data, length, result);
        if (result < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                result = 0;
            } else {
                WARNF("client fd=%d write error: " ERRNO_FMT, mFd, ERRNO_ARGS(errno));
                return;
            }
        }
        if (static_cast<size_t>(result) < length) {
            mWriteBuffer.enqueue(data + result, length - result);
            if (!mWriteRegistered) {
                mTask.update_interests(
                    scheduler::EventInterest::Read | scheduler::EventInterest::Write |
                    scheduler::EventInterest::Error | scheduler::EventInterest::Hangup);
                mWriteRegistered = true;
            }
        }
    } else {
        mWriteBuffer.enqueue(data, length);
    }
}

TcpServerStream::TcpServerStream(std::string                                    id,
                                 std::unique_ptr<scheduler::SocketListenerTask> listener,
                                 ReadBufferConfig read_config) NOEXCEPT
    : Stream(std::move(id), read_config),
      mListenerTask(std::move(listener)) {
    FUNCTION_SCOPEF("\"%s\"", mId.c_str());
}

TcpServerStream::~TcpServerStream() NOEXCEPT {
    FUNCTION_SCOPE();
    cancel();
}

bool TcpServerStream::schedule(scheduler::Scheduler& scheduler) {
    FUNCTION_SCOPEF("%p", &scheduler);
    mScheduler = &scheduler;

    mListenerTask->on_accept = [this](scheduler::SocketListenerTask&, int fd, sockaddr_storage*,
                                      socklen_t) {
        DEBUGF("accepted new client connection, fd=%d", fd);
        mClients.push_back(std::make_unique<Client>(*this, fd));
        VERBOSEF("total clients: %zu", mClients.size());
    };

    mListenerTask->on_error = [this](scheduler::SocketListenerTask& task) {
        int       err    = 0;
        socklen_t errlen = sizeof(err);
        auto      result = ::getsockopt(task.fd(), SOL_SOCKET, SO_ERROR, &err, &errlen);
        VERBOSEF("::getsockopt(%d, SOL_SOCKET, SO_ERROR, %p, %p) = %d", task.fd(), &err, &errlen,
                 result);
        ERRORF("listener error: %d (%s)", err, strerror(err));
        set_error(err, strerror(err));
    };

    mListenerTask->schedule(scheduler);
    if (!mListenerTask->is_scheduled()) {
        ERRORF("failed to schedule TCP listener");
        set_error(errno, "failed to schedule TCP listener");
        return false;
    }

    mState = State::Connected;
    schedule_read_timeout(scheduler);
    return true;
}

bool TcpServerStream::cancel() {
    FUNCTION_SCOPE();
    cancel_read_timeout();
    mClients.clear();
    if (mListenerTask) {
        mListenerTask->cancel();
    }
    return true;
}

uint16_t TcpServerStream::port() const NOEXCEPT {
    return mListenerTask ? mListenerTask->port() : 0;
}

void TcpServerStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    FUNCTION_SCOPEF("%p, %zu to %zu clients", data, length, mClients.size());
    for (auto& client : mClients) {
        client->write(data, length);
    }
}

void TcpServerStream::remove_client(int fd) NOEXCEPT {
    FUNCTION_SCOPE();
    auto it = std::find_if(mClients.begin(), mClients.end(), [fd](auto& c) {
        return c->fd() == fd;
    });
    if (it == mClients.end()) {
        VERBOSEF("client fd=%d not found", fd);
    } else {
        DEBUGF("removing client fd=%d", fd);
        mClients.erase(it);
        VERBOSEF("remaining clients: %zu", mClients.size());
    }
}

}  // namespace io
