#pragma once
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

namespace scheduler {
class FileDescriptorTask {
public:
    EXPLICIT FileDescriptorTask() NOEXCEPT;
    ~FileDescriptorTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;

    /// Set the file descriptor to be monitored. Will cancel on-going tasks and return true if the
    /// task must be rescheduled.
    NODISCARD bool set_fd(int fd) NOEXCEPT;
    NODISCARD int  fd() const NOEXCEPT { return mFd; }

    std::function<void(int)> on_read;
    std::function<void(int)> on_error;
    std::function<void(int)> on_write;

private:
    Scheduler* mScheduler;
    EpollEvent mEvent;
    int        mFd;
};
}  // namespace scheduler
