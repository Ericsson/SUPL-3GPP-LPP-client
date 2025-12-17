#include <io/stream/pty.hpp>
#include <scheduler/socket.hpp>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, pty);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, pty)

namespace io {

PtyStream::PtyStream(std::string id, PtyConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", link=\"%s\", raw=%d", mId.c_str(), mConfig.link_path.c_str(),
                     mConfig.raw);
}

PtyStream::~PtyStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool PtyStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    mMasterFd = ::posix_openpt(O_RDWR | O_NOCTTY);
    VERBOSEF("::posix_openpt(O_RDWR | O_NOCTTY) = %d", mMasterFd);
    if (mMasterFd < 0) {
        ERRORF("failed to open pty master: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "failed to open pty master");
        return false;
    }

    auto result = ::grantpt(mMasterFd);
    VERBOSEF("::grantpt(%d) = %d", mMasterFd, result);
    if (result < 0) {
        ERRORF("failed to grant pty: " ERRNO_FMT, ERRNO_ARGS(errno));
        auto close_result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, close_result);
        mMasterFd = -1;
        set_error(errno, "failed to grant pty");
        return false;
    }

    result = ::unlockpt(mMasterFd);
    VERBOSEF("::unlockpt(%d) = %d", mMasterFd, result);
    if (result < 0) {
        ERRORF("failed to unlock pty: " ERRNO_FMT, ERRNO_ARGS(errno));
        auto close_result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, close_result);
        mMasterFd = -1;
        set_error(errno, "failed to unlock pty");
        return false;
    }

    char* slave_name = ::ptsname(mMasterFd);
    VERBOSEF("::ptsname(%d) = \"%s\"", mMasterFd, slave_name ? slave_name : "(null)");
    if (!slave_name) {
        ERRORF("failed to get pty slave name: " ERRNO_FMT, ERRNO_ARGS(errno));
        auto close_result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, close_result);
        mMasterFd = -1;
        set_error(errno, "failed to get pty slave name");
        return false;
    }
    mSlavePath = slave_name;

    if (!mConfig.raw && !configure_termios()) {
        auto close_result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, close_result);
        mMasterFd = -1;
        return false;
    }

    auto flags = ::fcntl(mMasterFd, F_GETFL, 0);
    VERBOSEF("::fcntl(%d, F_GETFL, 0) = %d", mMasterFd, flags);
    result = ::fcntl(mMasterFd, F_SETFL, flags | O_NONBLOCK);
    VERBOSEF("::fcntl(%d, F_SETFL, %d) = %d", mMasterFd, flags | O_NONBLOCK, result);

    if (!mConfig.link_path.empty()) {
        ::unlink(mConfig.link_path.c_str());
        result = ::symlink(mSlavePath.c_str(), mConfig.link_path.c_str());
        VERBOSEF("::symlink(\"%s\", \"%s\") = %d", mSlavePath.c_str(), mConfig.link_path.c_str(),
                 result);
        if (result < 0) {
            WARNF("failed to create symlink: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
    }

    mSocketTask.reset(new scheduler::SocketTask(mMasterFd));
    mSocketTask->set_event_name("pty:" + mId);
    mSocketTask->on_read = [this](scheduler::SocketTask&) {
        auto result = ::read(mMasterFd, mReadBuf, sizeof(mReadBuf));
        VERBOSEF("::read(%d, %p, %zu) = %zd", mMasterFd, mReadBuf, sizeof(mReadBuf), result);
        if (result > 0) {
            on_raw_read(mReadBuf, result);
        } else if (result == 0) {
            DEBUGF("pty closed");
            set_disconnected();
        } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EIO) {
            ERRORF("failed to read from pty: " ERRNO_FMT, ERRNO_ARGS(errno));
            set_error(errno, strerror(errno));
        }
    };
    mSocketTask->on_write = [this](scheduler::SocketTask&) {
        while (!mWriteBuffer.empty()) {
            auto [data, len] = mWriteBuffer.peek();
            auto result      = ::write(mMasterFd, data, len);
            VERBOSEF("::write(%d, %p, %zu) = %zd", mMasterFd, data, len, result);
            if (result < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                WARNF("write error: " ERRNO_FMT, ERRNO_ARGS(errno));
                return;
            }
            mWriteBuffer.consume(result);
        }
        if (mWriteBuffer.empty() && mWriteRegistered) {
            mScheduler->update_epoll_fd(mMasterFd, EPOLLIN, nullptr);
            mWriteRegistered = false;
        }
    };
    mSocketTask->on_error = [this](scheduler::SocketTask&) {
        ERRORF("pty error: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, strerror(errno));
    };

    if (!mSocketTask->schedule(scheduler)) {
        ERRORF("failed to schedule socket task");
        auto close_result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, close_result);
        mMasterFd = -1;
        return false;
    }

    if (!schedule_read_timeout(scheduler)) {
        mSocketTask->cancel();
        auto close_result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, close_result);
        mMasterFd = -1;
        return false;
    }

    mState = State::Connected;
    DEBUGF("pty stream connected: master=%d, slave=%s", mMasterFd, mSlavePath.c_str());
    return true;
}

bool PtyStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();
    if (mSocketTask) {
        mSocketTask->cancel();
        mSocketTask.reset();
    }
    if (!mConfig.link_path.empty()) {
        auto result = ::unlink(mConfig.link_path.c_str());
        VERBOSEF("::unlink(\"%s\") = %d", mConfig.link_path.c_str(), result);
    }
    if (mMasterFd >= 0) {
        auto result = ::close(mMasterFd);
        VERBOSEF("::close(%d) = %d", mMasterFd, result);
        mMasterFd = -1;
    }
    return true;
}

void PtyStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    if (mWriteBuffer.empty()) {
        auto result = ::write(mMasterFd, data, length);
        VERBOSEF("::write(%d, %p, %zu) = %zd", mMasterFd, data, length, result);
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
        mScheduler->update_epoll_fd(mMasterFd, EPOLLIN | EPOLLOUT, nullptr);
        mWriteRegistered = true;
    }
}

bool PtyStream::configure_termios() NOEXCEPT {
    VSCOPE_FUNCTION();

    struct termios tty;
    auto           result = ::tcgetattr(mMasterFd, &tty);
    VERBOSEF("::tcgetattr(%d, %p) = %d", mMasterFd, &tty, result);
    if (result != 0) {
        ERRORF("failed to get pty attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "failed to get pty attributes");
        return false;
    }

    ::cfmakeraw(&tty);
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN]  = 0;

    result = ::tcsetattr(mMasterFd, TCSANOW, &tty);
    VERBOSEF("::tcsetattr(%d, TCSANOW, %p) = %d", mMasterFd, &tty, result);
    if (result != 0) {
        ERRORF("failed to set pty attributes: " ERRNO_FMT, ERRNO_ARGS(errno));
        set_error(errno, "failed to set pty attributes");
        return false;
    }

    return true;
}

}  // namespace io
