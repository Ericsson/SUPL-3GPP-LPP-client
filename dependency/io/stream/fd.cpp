#include <io/stream/fd.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, fd);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, fd)

namespace io {

FdStream::FdStream(std::string id, FdConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", fd=%d, owns=%d", mId.c_str(), mConfig.fd, mConfig.owns_fd);
}

FdStream::~FdStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool FdStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    auto flags = ::fcntl(mConfig.fd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mConfig.fd, flags);
    if (flags >= 0 && !(flags & O_NONBLOCK)) {
        auto result = ::fcntl(mConfig.fd, F_SETFL, flags | O_NONBLOCK);
        VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mConfig.fd, flags | O_NONBLOCK, result);
    }

    mSocketTask.reset(new scheduler::SocketTask(mConfig.fd));
    mSocketTask->set_event_name("fd:" + mId);
    mSocketTask->on_read = [this](scheduler::SocketTask&) {
        auto result = ::read(mConfig.fd, mReadBuf, sizeof(mReadBuf));
        VERBOSEF("::read(%d, %p, %zu) = %zd", mConfig.fd, mReadBuf, sizeof(mReadBuf), result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result == 0) {
            DEBUGF("fd %d closed", mConfig.fd);
            set_disconnected();
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ERRORF("failed to read from fd %d: " ERRNO_FMT, mConfig.fd, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };
    mSocketTask->on_write = [this](scheduler::SocketTask&) {
        while (!mWriteBuffer.empty()) {
            auto  peek   = mWriteBuffer.peek();
            auto& data   = peek.first;
            auto& len    = peek.second;
            auto  result = ::write(mConfig.fd, data, len);
            VERBOSEF("::write(%d, %p, %zu) = %zd", mConfig.fd, data, len, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
            mWriteBuffer.consume(result);
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mScheduler->update_epoll_fd(mConfig.fd, EPOLLIN, nullptr);
            mWriteRegistered = false;
        }
    };
    mSocketTask->on_error = [this](scheduler::SocketTask&) {
        ERRORF("fd %d error: " ERRNO_FMT, mConfig.fd, ERRNO_ARGS(errno));
        set_error(errno, strerror(errno));
    };

    if (!mSocketTask->schedule(scheduler)) {
        ERRORF("failed to schedule socket task for fd %d", mConfig.fd);
        return false;
    }

    if (!schedule_read_timeout(scheduler)) {
        mSocketTask->cancel();
        return false;
    }

    mState = State::Connected;
    DEBUGF("fd stream connected: fd=%d", mConfig.fd);
    return true;
}

bool FdStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mSocketTask) {
        mSocketTask->cancel();
        mSocketTask.reset();
    }
    if (mConfig.owns_fd && mConfig.fd >= 0) {
        auto result = ::close(mConfig.fd);
        VERBOSEF("::close(%d) = %d", mConfig.fd, result);
        mConfig.fd = -1;
    }
    return true;
}

void FdStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    if (mWriteBuffer.empty()) {
        auto result = ::write(mConfig.fd, data, length);
        VERBOSEF("::write(%d, %p, %zu) = %zd", mConfig.fd, data, length, result);
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
        mScheduler->update_epoll_fd(mConfig.fd, EPOLLIN | EPOLLOUT, nullptr);
        mWriteRegistered = true;
    }
}

}  // namespace io
