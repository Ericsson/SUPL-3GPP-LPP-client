#include <io/stream/file.hpp>
#include <scheduler/file_descriptor.hpp>
#include <scheduler/periodic.hpp>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(io, stream, file);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(io, stream, file)

namespace io {

FileStream::FileStream(std::string id, FileConfig config) NOEXCEPT
    : Stream(std::move(id), config.read_config),
      mConfig(std::move(config)) {
    VSCOPE_FUNCTIONF("\"%s\", \"%s\", read=%d, write=%d", mId.c_str(), mConfig.path.c_str(),
                     mConfig.read, mConfig.write);
}

FileStream::~FileStream() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool FileStream::schedule(scheduler::Scheduler& scheduler) {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    mScheduler = &scheduler;

    int flags = O_CLOEXEC;
    if (mConfig.read && mConfig.write) {
        flags |= O_RDWR;
    } else if (mConfig.write) {
        flags |= O_WRONLY;
    } else {
        flags |= O_RDONLY;
    }

    if (mConfig.write) {
        if (mConfig.truncate) flags |= O_TRUNC;
        if (mConfig.append) flags |= O_APPEND;
        if (mConfig.create) flags |= O_CREAT;
    }

    mFd = ::open(mConfig.path.c_str(), flags, 0666);
    VERBOSEF("::open(\"%s\", %d, 0666) = %d", mConfig.path.c_str(), flags, mFd);
    if (mFd < 0) {
        ERRORF("failed to open file \"%s\": " ERRNO_FMT, mConfig.path.c_str(), ERRNO_ARGS(errno));
        set_error(errno, "failed to open file");
        return false;
    }

    if (mConfig.read) {
        if (mConfig.bytes_per_tick > 0 && mConfig.tick_interval.count() > 0) {
            mReadTask.reset(new scheduler::PeriodicTask(mConfig.tick_interval));
            mReadTask->callback = [this]() {
                auto to_read = std::min(mConfig.bytes_per_tick, sizeof(mReadBuf));
                auto result  = ::read(mFd, mReadBuf, to_read);
                VERBOSEF("::read(%d, %p, %zu) = %zd", mFd, mReadBuf, to_read, result);
                if (result > 0) {
                    on_raw_read(mReadBuf, result);
                } else if (result == 0) {
                    DEBUGF("file read complete (EOF)");
                    set_disconnected();
                    mReadTask->cancel();
                } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    ERRORF("failed to read from file: " ERRNO_FMT, ERRNO_ARGS(errno));
                    set_error(errno, strerror(errno));
                }
            };

            if (!mReadTask->schedule(scheduler)) {
                ERRORF("failed to schedule read task");
                auto result = ::close(mFd);
                VERBOSEF("::close(%d) = %d", mFd, result);
                mFd = -1;
                return false;
            }
        } else {
            mFdTask.reset(new scheduler::FileDescriptorTask());
            (void)mFdTask->set_fd(mFd);
            mFdTask->on_read = [this](int) {
                auto result = ::read(mFd, mReadBuf, sizeof(mReadBuf));
                VERBOSEF("::read(%d, %p, %zu) = %zd", mFd, mReadBuf, sizeof(mReadBuf), result);
                if (result > 0) {
                    on_raw_read(mReadBuf, result);
                } else if (result == 0) {
                    DEBUGF("file read complete (EOF)");
                    set_disconnected();
                } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    ERRORF("failed to read from file: " ERRNO_FMT, ERRNO_ARGS(errno));
                    set_error(errno, strerror(errno));
                }
            };
            mFdTask->on_error = [this](int) {
                ERRORF("file read error");
                set_error(errno, strerror(errno));
            };

            if (!mFdTask->schedule(scheduler)) {
                ERRORF("failed to schedule file task");
                auto result = ::close(mFd);
                VERBOSEF("::close(%d) = %d", mFd, result);
                mFd = -1;
                return false;
            }
        }
    }

    if (!schedule_read_timeout(scheduler)) {
        cancel();
        return false;
    }

    mState = State::Connected;
    DEBUGF("file stream connected: %s", mConfig.path.c_str());
    return true;
}

bool FileStream::cancel() {
    VSCOPE_FUNCTION();
    cancel_read_timeout();

    if (mReadTask) {
        mReadTask->cancel();
        mReadTask.reset();
    }

    if (mFdTask) {
        mFdTask->cancel();
        mFdTask.reset();
    }

    if (mFd >= 0) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }

    return true;
}

void FileStream::write(uint8_t const* data, size_t length) NOEXCEPT {
    auto result = ::write(mFd, data, length);
    VERBOSEF("::write(%d, %p, %zu) = %zd", mFd, data, length, result);
    if (result < 0) {
        WARNF("failed to write to file: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

}  // namespace io
