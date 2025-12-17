#include <io/stream/stdio.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/socket.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, stdio);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, stdio)

namespace io {

StdioStream::StdioStream(std::string id, StdioConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", stderr=%d", mId.c_str(), mConfig.use_stderr);
}

StdioStream::~StdioStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool StdioStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    auto flags = ::fcntl(STDIN_FILENO, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", STDIN_FILENO, flags);
    if (flags >= 0 && !(flags & O_NONBLOCK)) {
        auto result = ::fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", STDIN_FILENO, flags | O_NONBLOCK, result);
    }

    int write_fd = mConfig.use_stderr ? STDERR_FILENO : STDOUT_FILENO;
    flags        = ::fcntl(write_fd, F_GETFL, 0);
    if (flags >= 0 && !(flags & O_NONBLOCK)) {
        ::fcntl(write_fd, F_SETFL, flags | O_NONBLOCK);
    }

    mSocketTask.reset(new scheduler::SocketTask(STDIN_FILENO));
    mSocketTask->set_event_name("stdio:" + mId);
    mSocketTask->on_read = [this](scheduler::SocketTask&) {
        auto result = ::read(STDIN_FILENO, mReadBuf, sizeof(mReadBuf));
        VERBOSEF("::read(%d, %p, %zu) = %zd", STDIN_FILENO, mReadBuf, sizeof(mReadBuf), result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result == 0) {
            DEBUGF("stdin closed");
            set_disconnected();
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ERRORF("failed to read from stdin: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };
    mSocketTask->on_error = [this](scheduler::SocketTask&) {
        ERRORF("stdin error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, strerror(errno));
    };

    if (!mSocketTask->schedule(scheduler)) {
        ERRORF("failed to schedule socket task for stdin");
        return false;
    }

    mWriteTask.reset(new scheduler::SocketTask(write_fd));
    mWriteTask->set_event_name("stdio-write:" + mId);
    mWriteTask->on_write = [this, write_fd](scheduler::SocketTask&) {
        while (!mWriteBuffer.empty()) {
            auto [data, len] = mWriteBuffer.peek();
            auto result      = ::write(write_fd, data, len);
            VERBOSEF("::write(%d, %p, %zu) = %zd", write_fd, data, len, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
            mWriteBuffer.consume(result);
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mWriteTask->cancel();
            mWriteRegistered = false;
        }
    };

    if (!schedule_read_timeout(scheduler)) {
        mSocketTask->cancel();
        return false;
    }

    mState = State::Connected;
    DEBUGF("stdio stream connected: write_fd=%d", write_fd);
    return true;
}

bool StdioStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mWriteTask) {
        mWriteTask->cancel();
        mWriteTask.reset();
    }
    if (mSocketTask) {
        mSocketTask->cancel();
        mSocketTask.reset();
    }
    return true;
}

void StdioStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);
    int write_fd = mConfig.use_stderr ? STDERR_FILENO : STDOUT_FILENO;

    if (mWriteBuffer.empty()) {
        auto result = ::write(write_fd, data, length);
        VERBOSEF("::write(%d, %p, %zu) = %zd", write_fd, data, length, result);
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
        mWriteTask->schedule(*mScheduler);
        mWriteRegistered = true;
    }
}

}  // namespace io
