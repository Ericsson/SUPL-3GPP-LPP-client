#pragma once
#include <scheduler/scheduler.hpp>
#include <scheduler/timeout.hpp>

#include <chrono>
#include <functional>
#include <string>

namespace scheduler {
class PeriodicTask {
public:
    EXPLICIT
    PeriodicTask(std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~PeriodicTask() NOEXCEPT;

    PeriodicTask(PeriodicTask const&)            = delete;
    PeriodicTask(PeriodicTask&&)                 = delete;
    PeriodicTask& operator=(PeriodicTask const&) = delete;
    PeriodicTask& operator=(PeriodicTask&&)      = delete;

    NODISCARD bool schedule(Scheduler& scheduler) NOEXCEPT;
    NODISCARD bool schedule() NOEXCEPT { return schedule(current()); }
    bool           cancel() NOEXCEPT;

    std::function<void()> callback;

    void set_event_name(std::string name) { mEventName = std::move(name); }

private:
    ScheduledEvent mEvent;
    Timer          mTimer;
    std::string    mEventName;
};
}  // namespace scheduler
