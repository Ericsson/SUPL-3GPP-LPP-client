#include "file_descriptor.hpp"

#include <unistd.h>
#include <utility>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, task);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, task)

namespace scheduler {

FileDescriptorTask::FileDescriptorTask() NOEXCEPT : mEvent{ScheduledEvent::invalid()},
                                                    mName{"fd"},
                                                    mFd{-1} {
    VSCOPE_FUNCTION();
}

FileDescriptorTask::~FileDescriptorTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEvent.valid()) {
        cancel();
    }
}

FileDescriptorTask::FileDescriptorTask(FileDescriptorTask&& other) NOEXCEPT
    : on_read{std::move(other.on_read)},
      on_write{std::move(other.on_write)},
      on_error{std::move(other.on_error)},
      mEvent{other.mEvent},
      mName{std::move(other.mName)},
      mFd{other.mFd} {
    other.mEvent.invalidate();
    other.mFd = -1;
    mEvent.callback([this](EventInterest triggered) {
        if ((triggered & EventInterest::Read) && on_read) on_read(*this);
        if ((triggered & EventInterest::Write) && on_write) on_write(*this);
        if ((triggered & (EventInterest::Error | EventInterest::Hangup)) && on_error)
            on_error(*this);
    });
}

FileDescriptorTask& FileDescriptorTask::operator=(FileDescriptorTask&& other) NOEXCEPT {
    if (this != &other) {
        if (mEvent.valid()) cancel();

        on_read  = std::move(other.on_read);
        on_write = std::move(other.on_write);
        on_error = std::move(other.on_error);
        mEvent   = other.mEvent;
        mName    = std::move(other.mName);
        mFd      = other.mFd;
        other.mEvent.invalidate();
        other.mFd = -1;
        mEvent.callback([this](EventInterest triggered) {
            if ((triggered & EventInterest::Read) && on_read) on_read(*this);
            if ((triggered & EventInterest::Write) && on_write) on_write(*this);
            if ((triggered & (EventInterest::Error | EventInterest::Hangup)) && on_error)
                on_error(*this);
        });
    }
    return *this;
}

bool FileDescriptorTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mEvent.valid()) {
        WARNF("already scheduled");
        return false;
    } else if (mFd < 0) {
        WARNF("invalid file descriptor");
        return false;
    }

    EventInterest interests = EventInterest::Error | EventInterest::Hangup;
    if (on_read) interests = interests | EventInterest::Read;

    mEvent = scheduler.register_fd(
        mFd, interests,
        [this](EventInterest triggered) {
            VERBOSEF("fd task: event: %d %s%s%s%s", mFd,
                     (triggered & EventInterest::Read) ? "read " : "",
                     (triggered & EventInterest::Write) ? "write " : "",
                     (triggered & EventInterest::Error) ? "error " : "",
                     (triggered & EventInterest::Hangup) ? "hangup " : "");
            TRACE_INDENT_SCOPE();

            if ((triggered & EventInterest::Read) && on_read) {
                on_read(*this);
            }
            if ((triggered & EventInterest::Write) && on_write) {
                on_write(*this);
            }
            if ((triggered & (EventInterest::Error | EventInterest::Hangup)) && on_error) {
                on_error(*this);
            }
        },
        mName);

    return mEvent.valid();
}

bool FileDescriptorTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mEvent.valid()) {
        WARNF("not scheduled");
        return false;
    }

    mEvent.unregister();
    return true;
}

void FileDescriptorTask::set_fd(int fd) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d", fd);
    if (mFd == fd) return;

    bool was_scheduled = is_scheduled();

    if (was_scheduled) {
        cancel();
    }

    mFd = fd;

    if (was_scheduled && mFd >= 0) {
        schedule();
    }
}

void FileDescriptorTask::update() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mEvent.valid()) {
        WARNF("not scheduled");
        return;
    }

    EventInterest interests = EventInterest::Error | EventInterest::Hangup;
    if (on_read) interests = interests | EventInterest::Read;

    mEvent.interests(interests);
}

void FileDescriptorTask::update_interests(EventInterest interests) NOEXCEPT {
    VSCOPE_FUNCTIONF("%u", static_cast<uint32_t>(interests));
    if (!mEvent.valid()) {
        WARNF("not scheduled");
        return;
    }

    mEvent.interests(interests);
}

//
// OwnedFileDescriptorTask
//

OwnedFileDescriptorTask::OwnedFileDescriptorTask(int fd) NOEXCEPT {
    VSCOPE_FUNCTION();
    mTask.set_fd(fd);
    mTask.set_name("owned_fd");

    mTask.on_read = [this](FileDescriptorTask&) {
        if (on_read) on_read(*this);
    };
    mTask.on_write = [this](FileDescriptorTask&) {
        if (on_write) on_write(*this);
    };
    mTask.on_error = [this](FileDescriptorTask&) {
        if (on_error) on_error(*this);
    };
}

OwnedFileDescriptorTask::~OwnedFileDescriptorTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    int fd = mTask.fd();
    if (fd >= 0) {
        mTask.cancel();
        auto result = ::close(fd);
        VERBOSEF("::close(%d) = %d", fd, result);
    }
}

OwnedFileDescriptorTask::OwnedFileDescriptorTask(OwnedFileDescriptorTask&& other) NOEXCEPT
    : on_read{std::move(other.on_read)},
      on_write{std::move(other.on_write)},
      on_error{std::move(other.on_error)},
      mTask{std::move(other.mTask)} {
    // Re-bind callbacks to this instance
    mTask.on_read = [this](FileDescriptorTask&) {
        if (on_read) on_read(*this);
    };
    mTask.on_write = [this](FileDescriptorTask&) {
        if (on_write) on_write(*this);
    };
    mTask.on_error = [this](FileDescriptorTask&) {
        if (on_error) on_error(*this);
    };
}

OwnedFileDescriptorTask&
OwnedFileDescriptorTask::operator=(OwnedFileDescriptorTask&& other) NOEXCEPT {
    if (this != &other) {
        // Close current fd
        int fd = mTask.fd();
        if (fd >= 0) {
            mTask.cancel();
            ::close(fd);
        }

        on_read  = std::move(other.on_read);
        on_write = std::move(other.on_write);
        on_error = std::move(other.on_error);
        mTask    = std::move(other.mTask);

        // Re-bind callbacks to this instance
        mTask.on_read = [this](FileDescriptorTask&) {
            if (on_read) on_read(*this);
        };
        mTask.on_write = [this](FileDescriptorTask&) {
            if (on_write) on_write(*this);
        };
        mTask.on_error = [this](FileDescriptorTask&) {
            if (on_error) on_error(*this);
        };
    }
    return *this;
}

void OwnedFileDescriptorTask::set_fd(int fd) NOEXCEPT {
    VSCOPE_FUNCTIONF("%d", fd);
    int old_fd = mTask.fd();
    if (old_fd >= 0 && old_fd != fd) {
        ::close(old_fd);
    }
    mTask.set_fd(fd);
}

int OwnedFileDescriptorTask::release() NOEXCEPT {
    VSCOPE_FUNCTION();
    int fd = mTask.fd();
    mTask.set_fd(-1);
    return fd;
}

}  // namespace scheduler
