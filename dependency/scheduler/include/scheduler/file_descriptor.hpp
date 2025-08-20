#pragma once
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>
#include <string>

namespace scheduler {
class FileDescriptorTask {
public:
    EXPLICIT FileDescriptorTask() NOEXCEPT;
    ~FileDescriptorTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool           cancel() NOEXCEPT;

    /// Set the file descriptor to be monitored. Will cancel on-going tasks and return true if the
    /// task must be rescheduled.
    NODISCARD bool set_fd(int fd) NOEXCEPT;
    NODISCARD int  fd() const NOEXCEPT { return mFd; }

    void set_event_name(std::string const& name) {
        mEventName  = name;
        mEvent.name = mEventName.c_str();
    }

    std::function<void(int)> on_read;
    std::function<void(int)> on_error;
    std::function<void(int)> on_write;

private:
    Scheduler*  mScheduler;
    EpollEvent  mEvent;
    int         mFd;
    std::string mEventName;
};
}  // namespace scheduler
