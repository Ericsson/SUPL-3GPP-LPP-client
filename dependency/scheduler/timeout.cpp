#include "timeout.hpp"

#include <cerrno>
#include <cstring>
#include <sys/timerfd.h>
#include <unistd.h>
#include <utility>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, timeout);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, timeout)

namespace scheduler {

//
// Timer
//

Timer::Timer(std::chrono::steady_clock::duration duration) NOEXCEPT : mDuration{duration},
                                                                      mTimerFd{-1} {
    VSCOPE_FUNCTION();
    mTimerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    VERBOSEF("::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK) = %d", mTimerFd);
    if (mTimerFd < 0) {
        ERRORF("failed to create timerfd: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

Timer::~Timer() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mTimerFd >= 0) {
        auto result = ::close(mTimerFd);
        VERBOSEF("::close(%d) = %d", mTimerFd, result);
    }
}

Timer::Timer(Timer&& other) NOEXCEPT : mDuration{other.mDuration}, mTimerFd{other.mTimerFd} {
    other.mTimerFd = -1;
}

Timer& Timer::operator=(Timer&& other) NOEXCEPT {
    if (this != &other) {
        if (mTimerFd >= 0) ::close(mTimerFd);
        mDuration      = other.mDuration;
        mTimerFd       = other.mTimerFd;
        other.mTimerFd = -1;
    }
    return *this;
}

void Timer::arm(bool repeat) NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mTimerFd < 0) return;

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(mDuration).count();
    auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(mDuration).count() % 1000000000;

    struct itimerspec its{};
    its.it_value.tv_sec  = seconds;
    its.it_value.tv_nsec = nanoseconds;
    if (repeat) {
        its.it_interval.tv_sec  = seconds;
        its.it_interval.tv_nsec = nanoseconds;
    }

    auto result = ::timerfd_settime(mTimerFd, 0, &its, nullptr);
    VERBOSEF("::timerfd_settime(%d, 0, {%ld.%09ld}, nullptr) = %d", mTimerFd, seconds, nanoseconds,
             result);
    if (result < 0) {
        ERRORF("failed to arm timer: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}

void Timer::disarm() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mTimerFd < 0) return;

    struct itimerspec its{};
    auto              result = ::timerfd_settime(mTimerFd, 0, &its, nullptr);
    VERBOSEF("::timerfd_settime(%d, 0, {0}, nullptr) = %d", mTimerFd, result);
}

//
// TimeoutTask
//

TimeoutTask::TimeoutTask(std::chrono::steady_clock::duration duration,
                         std::function<void()>               callback) NOEXCEPT
    : mTimer{duration},
      mEvent{ScheduledEvent::invalid()},
      mCallback{std::move(callback)} {
    VSCOPE_FUNCTION();
    schedule();
}

TimeoutTask::~TimeoutTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

TimeoutTask::TimeoutTask(TimeoutTask&& other) NOEXCEPT : mTimer{std::move(other.mTimer)},
                                                         mEvent{other.mEvent},
                                                         mCallback{std::move(other.mCallback)} {
    other.mEvent.invalidate();
    mEvent.callback([this](EventInterest i) {
        on_event(i);
    });
}

TimeoutTask& TimeoutTask::operator=(TimeoutTask&& other) NOEXCEPT {
    if (this != &other) {
        cancel();
        mTimer    = std::move(other.mTimer);
        mEvent    = other.mEvent;
        mCallback = std::move(other.mCallback);
        other.mEvent.invalidate();
        mEvent.callback([this](EventInterest i) {
            on_event(i);
        });
    }
    return *this;
}

void TimeoutTask::schedule() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEvent.valid() || mTimer.fd() < 0) return;

    mTimer.arm();
    mEvent = current().register_fd(
        mTimer.fd(), EventInterest::Read,
        [this](EventInterest i) {
            on_event(i);
        },
        "timeout");

    if (mEvent.valid()) {
        DEBUGF("timeout in %lu ms",
               std::chrono::duration_cast<std::chrono::milliseconds>(mTimer.duration()).count());
    }
}

void TimeoutTask::on_event(EventInterest triggered) NOEXCEPT {
    if (!(triggered & EventInterest::Read)) return;

    uint64_t expirations = 0;
    ::read(mTimer.fd(), &expirations, sizeof(expirations));

    if (mCallback) mCallback();

    defer([this](Scheduler&) {
        cancel();
    });
}

void TimeoutTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    mEvent.unregister();
    mTimer.disarm();
}

//
// RepeatableTimeoutTask
//

RepeatableTimeoutTask::RepeatableTimeoutTask(std::chrono::steady_clock::duration duration) NOEXCEPT
    : callback{},
      mTimer{duration},
      mEvent{ScheduledEvent::invalid()} {
    VSCOPE_FUNCTION();
}

RepeatableTimeoutTask::~RepeatableTimeoutTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

RepeatableTimeoutTask::RepeatableTimeoutTask(RepeatableTimeoutTask&& other) NOEXCEPT
    : callback{std::move(other.callback)},
      mTimer{std::move(other.mTimer)},
      mEvent{other.mEvent} {
    other.mEvent.invalidate();
    mEvent.callback([this](EventInterest i) {
        on_event(i);
    });
}

RepeatableTimeoutTask& RepeatableTimeoutTask::operator=(RepeatableTimeoutTask&& other) NOEXCEPT {
    if (this != &other) {
        cancel();
        mTimer   = std::move(other.mTimer);
        mEvent   = other.mEvent;
        callback = std::move(other.callback);
        other.mEvent.invalidate();
        mEvent.callback([this](EventInterest i) {
            on_event(i);
        });
    }
    return *this;
}

void RepeatableTimeoutTask::schedule() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEvent.valid() || mTimer.fd() < 0) {
        VERBOSEF("repeatable timeout already scheduled");
        return;
    }

    mTimer.arm();
    mEvent = current().register_fd(
        mTimer.fd(), EventInterest::Read,
        [this](EventInterest i) {
            on_event(i);
        },
        "repeatable-timeout");

    if (mEvent.valid()) {
        DEBUGF("repeatable timeout in %lu ms",
               std::chrono::duration_cast<std::chrono::milliseconds>(mTimer.duration()).count());
    }
}

void RepeatableTimeoutTask::on_event(EventInterest triggered) NOEXCEPT {
    if (!(triggered & EventInterest::Read)) return;

    uint64_t expirations = 0;
    ::read(mTimer.fd(), &expirations, sizeof(expirations));

    if (callback) callback();
}

void RepeatableTimeoutTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    mEvent.unregister();
    mTimer.disarm();
}

void RepeatableTimeoutTask::restart() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mEvent.valid()) {
        mTimer.arm();
        DEBUGF("repeatable timeout restarted, %lu ms",
               std::chrono::duration_cast<std::chrono::milliseconds>(mTimer.duration()).count());
    } else {
        schedule();
    }
}

}  // namespace scheduler
