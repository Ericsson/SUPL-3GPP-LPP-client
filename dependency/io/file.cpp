#include "file.hpp"

#include <fcntl.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <scheduler/file_descriptor.hpp>
#include <scheduler/scheduler.hpp>
#include <scheduler/stream.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, file);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, file)

namespace io {
FileInput::FileInput(std::string path, size_t bytes_per_tick, Duration tick_interval) NOEXCEPT
    : mPath(std::move(path)),
      mBytesPerTick(bytes_per_tick),
      mTickInterval(tick_interval),
      mFileFd(-1),
      mForwardFd(-1) {
    VSCOPE_FUNCTIONF("\"%s\", %zu, %zu ms", mPath.c_str(), mBytesPerTick,
                     std::chrono::duration_cast<std::chrono::milliseconds>(mTickInterval).count());

    std::stringstream ss;
    ss << "file:" << mPath;
    mEventName = ss.str();
}

FileInput::~FileInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool FileInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mFileFd = ::open(mPath.c_str(), O_RDONLY);
    VERBOSEF("::open(\"%s\", O_RDONLY) = %d", mPath.c_str(), mFileFd);
    if (mFileFd < 0) {
        ERRORF("failed to open file \"%s\": " ERRNO_FMT, mPath.c_str(), ERRNO_ARGS(errno));
        return false;
    }

    mStreamTask.reset(new scheduler::ForwardStreamTask(mFileFd, mBytesPerTick, mTickInterval));
    mStreamTask->set_event_name("fst/" + mEventName);
    mForwardFd = mStreamTask->fd();

    mFdTask.reset(new scheduler::FileDescriptorTask());
    mFdTask->set_fd(mForwardFd);
    mFdTask->set_event_name("fd/" + mEventName);
    mFdTask->on_read = [this](int) {
        auto result = ::read(mForwardFd, mBuffer, sizeof(mBuffer));
        VERBOSEF("::read(%d, %p, %zu) = %d", mForwardFd, mBuffer, sizeof(mBuffer), result);
        if (result < 0) {
            ERRORF("failed to read from file: " ERRNO_FMT, ERRNO_ARGS(errno));
            cancel();
            return;
        }

        if (callback) {
            callback(*this, mBuffer, static_cast<size_t>(result));
        }
    };
    mFdTask->on_error = [this](int) {
        // NOTE(ewasjon): I am not sure what to do here.
        cancel();
    };

    mStreamTask->schedule(scheduler);
    mFdTask->schedule(scheduler);
    return true;
}

bool FileInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mForwardFd = -1;

    if (mFdTask) {
        mFdTask->cancel();
        mFdTask.reset();
    }

    if (mStreamTask) {
        mStreamTask->cancel();
        mStreamTask.reset();
    }

    if (mFileFd >= 0) {
        auto result = ::close(mFileFd);
        VERBOSEF("::close(%d) = %d", mFileFd, result);
        mFileFd = -1;
    }

    return true;
}

//
//
//

FileOutput::FileOutput(std::string path, bool truncate, bool append, bool create) NOEXCEPT
    : mPath(std::move(path)),
      mTruncate(truncate),
      mAppend(append),
      mCreate(create),
      mFd(-1) {
    VSCOPE_FUNCTIONF("\"%s\"%s%s%s", mPath.c_str(), truncate ? " TRUNCATE" : "",
                     append ? " APPEND" : "", create ? " CREATE" : "");
}

FileOutput::~FileOutput() NOEXCEPT {
    VSCOPE_FUNCTION();
    close();
}

void FileOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mFd == -1) {
        open();
    }

    if (mFd != -1) {
        auto result = ::write(mFd, buffer, length);
        VERBOSEF("::write(%d, %p, %zu) = %d", mFd, buffer, length, result);
        if (result < 0) {
            WARNF("failed to write to file: " ERRNO_FMT, ERRNO_ARGS(errno));
        }
    }
}

void FileOutput::open() {
    VSCOPE_FUNCTION();

    int flags = O_WRONLY | O_CLOEXEC;
    if (mTruncate) {
        flags |= O_TRUNC;
    }
    if (mAppend) {
        flags |= O_APPEND;
    }
    if (mCreate) {
        flags |= O_CREAT;
    }

    mFd = ::open(mPath.c_str(), flags, 0666);
    VERBOSEF("::open(\"%s\", %d, 0666) = %d", mPath.c_str(), flags, mFd);
    if (mFd < 0) {
        ERRORF("failed to open file \"%s\": " ERRNO_FMT, mPath.c_str(), ERRNO_ARGS(errno));
    }
}

void FileOutput::close() {
    VSCOPE_FUNCTION();

    if (mFd != -1) {
        auto result = ::close(mFd);
        VERBOSEF("::close(%d) = %d", mFd, result);
        mFd = -1;
    }
}

}  // namespace io
