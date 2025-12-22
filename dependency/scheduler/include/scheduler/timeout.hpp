#pragma once
#include <scheduler/scheduler.hpp>

#include <chrono>
#include <functional>

namespace scheduler {

class Timer {
public:
    EXPLICIT Timer(std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~Timer() NOEXCEPT;

    Timer(Timer&& other) NOEXCEPT;
    Timer& operator=(Timer&& other) NOEXCEPT;
    Timer(Timer const&)            = delete;
    Timer& operator=(Timer const&) = delete;

    void arm(bool repeat = false) NOEXCEPT;
    void disarm() NOEXCEPT;

    NODISCARD int fd() const NOEXCEPT { return mTimerFd; }

    void      set_duration(std::chrono::steady_clock::duration d) NOEXCEPT { mDuration = d; }
    NODISCARD std::chrono::steady_clock::duration duration() const NOEXCEPT { return mDuration; }

private:
    std::chrono::steady_clock::duration mDuration;
    int                                 mTimerFd;
};

class TimeoutTask {
public:
    TimeoutTask(std::chrono::steady_clock::duration duration,
                std::function<void()>               callback) NOEXCEPT;
    ~TimeoutTask() NOEXCEPT;

    TimeoutTask(TimeoutTask&& other) NOEXCEPT;
    TimeoutTask& operator=(TimeoutTask&& other) NOEXCEPT;
    TimeoutTask(TimeoutTask const&)            = delete;
    TimeoutTask& operator=(TimeoutTask const&) = delete;

    NODISCARD bool is_scheduled() const NOEXCEPT { return mEvent.valid(); }

    void cancel() NOEXCEPT;

private:
    void schedule() NOEXCEPT;
    void on_event(EventInterest triggered) NOEXCEPT;

    Timer                 mTimer;
    ScheduledEvent        mEvent;
    std::function<void()> mCallback;
};

class RepeatableTimeoutTask {
public:
    EXPLICIT RepeatableTimeoutTask(std::chrono::steady_clock::duration duration) NOEXCEPT;
    ~RepeatableTimeoutTask() NOEXCEPT;

    RepeatableTimeoutTask(RepeatableTimeoutTask&& other) NOEXCEPT;
    RepeatableTimeoutTask& operator=(RepeatableTimeoutTask&& other) NOEXCEPT;
    RepeatableTimeoutTask(RepeatableTimeoutTask const&)            = delete;
    RepeatableTimeoutTask& operator=(RepeatableTimeoutTask const&) = delete;

    void schedule() NOEXCEPT;
    void cancel() NOEXCEPT;
    void restart() NOEXCEPT;

    NODISCARD bool is_scheduled() const NOEXCEPT { return mEvent.valid(); }

    void set_duration(std::chrono::steady_clock::duration d) NOEXCEPT { mTimer.set_duration(d); }

    std::function<void()> callback;

private:
    void on_event(EventInterest triggered) NOEXCEPT;

    Timer          mTimer;
    ScheduledEvent mEvent;
};

}  // namespace scheduler
