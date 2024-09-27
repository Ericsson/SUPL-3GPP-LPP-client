#pragma once
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

namespace scheduler {
class TimeoutTask {
public:
    EXPLICIT TimeoutTask(std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~TimeoutTask() NOEXCEPT;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool cancel() NOEXCEPT;
    NODISCARD bool is_scheduled() const NOEXCEPT {
        return mScheduler != nullptr;
    }
    NODISCARD Scheduler& scheduler() const NOEXCEPT { return *mScheduler; }

    std::function<void()> callback;

private:
    Scheduler*                          mScheduler;
    EpollEvent                          mEvent;
    std::chrono::steady_clock::duration mDuration;
    int                                 mTimerFd;
};
}  // namespace scheduler
