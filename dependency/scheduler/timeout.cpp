#include "timeout.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, timeout);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, timeout)

namespace scheduler {
TimeoutTask::TimeoutTask(std::chrono::steady_clock::duration duration) NOEXCEPT
    : callback{},
      mScheduler{nullptr},
      mEvent{},
      mDuration{duration},
      mTimerFd{-1} {
    VSCOPE_FUNCTION();
    mEvent.name  = "timeout";
    mEvent.event = [this](struct epoll_event* event) {
        if ((event->events & EPOLLIN) == 0) return;

        VERBOSEF("timeout task: event");
        TRACE_INDENT_SCOPE();

        uint64_t expirations = 0;
        auto     result      = ::read(mTimerFd, &expirations, sizeof(expirations));
        VERBOSEF("::read(%d, %p, %zu) = %d", mTimerFd, &expirations, sizeof(expirations), result);
        if (result < 0) {
            ERRORF("failed to read timerfd: " ERRNO_FMT, ERRNO_ARGS(errno));
            return;
        }

        // TODO(ewasjon): Should callback be called for each expiration?
        if (this->callback) {
            this->callback();
        }

        mScheduler->defer([this](scheduler::Scheduler&) {
            cancel();
        });
    };
}

TimeoutTask::~TimeoutTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (mScheduler) {
        cancel();
    }

    if (mTimerFd >= 0) {
        auto result = ::close(mTimerFd);
        VERBOSEF("::close(%d) = %d", mTimerFd, result);
        mTimerFd = -1;
    }
}

bool TimeoutTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mScheduler) {
        WARNF("already scheduled");
        return false;
    }

    mTimerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    VERBOSEF("::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK) = %d", mTimerFd);
    if (mTimerFd < 0) {
        ERRORF("failed to create timerfd: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(mDuration).count();
    auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(mDuration).count() % 1000000000;

    struct itimerspec its{};
    its.it_value.tv_sec     = seconds;
    its.it_value.tv_nsec    = nanoseconds;
    its.it_interval.tv_sec  = 0;
    its.it_interval.tv_nsec = 0;

    auto result = ::timerfd_settime(mTimerFd, 0, &its, nullptr);
    VERBOSEF("::timerfd_settime(%d, 0, %p, %p) = %d", mTimerFd, &its, nullptr, result);
    if (result < 0) {
        ERRORF("failed to set timerfd: " ERRNO_FMT, ERRNO_ARGS(errno));
        return false;
    }

    if (scheduler.add_epoll_fd(mTimerFd, EPOLLIN, &mEvent)) {
        DEBUGF("timeout in %lu ms",
               std::chrono::duration_cast<std::chrono::milliseconds>(mDuration).count());
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}

bool TimeoutTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mScheduler) {
        WARNF("not scheduled");
        return false;
    }

    if (mScheduler->remove_epoll_fd(mTimerFd)) {
        mScheduler = nullptr;

        auto result = ::close(mTimerFd);
        VERBOSEF("::close(%d) = %d", mTimerFd, result);
        mTimerFd = -1;
        return true;
    } else {
        return false;
    }
}
}  // namespace scheduler
