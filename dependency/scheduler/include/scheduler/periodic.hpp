#pragma once
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

namespace scheduler {
class PeriodicTask {
public:
    EXPLICIT
    PeriodicTask(std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~PeriodicTask() NOEXCEPT;

    PeriodicTask(PeriodicTask const&) = delete;
    PeriodicTask(PeriodicTask&&)      = delete;
    PeriodicTask& operator=(PeriodicTask const&) = delete;
    PeriodicTask& operator=(PeriodicTask&&) = delete;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    bool                     cancel() NOEXCEPT;

    std::function<void()> callback;

private:
    Scheduler*                          mScheduler;
    EpollEvent                          mEvent;
    std::chrono::steady_clock::duration mDuration;
    int                                 mTimerFd;
};
}  // namespace scheduler
