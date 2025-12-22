#include "periodic.hpp"

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(sched, periodic);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(sched, periodic)

namespace scheduler {
PeriodicTask::PeriodicTask(std::chrono::steady_clock::duration duration) NOEXCEPT
    : callback{},
      mEvent{ScheduledEvent::invalid()},
      mTimer{duration},
      mEventName{"periodic"} {
    VSCOPE_FUNCTION();
}

PeriodicTask::~PeriodicTask() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool PeriodicTask::schedule(Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);
    if (mEvent.valid()) {
        WARNF("already scheduled");
        return false;
    }

    if (mTimer.fd() < 0) {
        ERRORF("invalid timer fd");
        return false;
    }

    mTimer.arm(true);
    mEvent = scheduler.register_fd(
        mTimer.fd(), EventInterest::Read,
        [this](EventInterest triggered) {
            if (!(triggered & EventInterest::Read)) return;

            VERBOSEF("periodic task: event");
            TRACE_INDENT_SCOPE();

            uint64_t expirations = 0;
            auto     result      = ::read(mTimer.fd(), &expirations, sizeof(expirations));
            VERBOSEF("::read(%d, %p, %zu) = %zd", mTimer.fd(), &expirations, sizeof(expirations),
                     result);

            if (this->callback) {
                this->callback();
            }
        },
        mEventName);

    if (mEvent.valid()) {
        DEBUGF("periodic timeout in %lu ms",
               std::chrono::duration_cast<std::chrono::milliseconds>(mTimer.duration()).count());
        return true;
    } else {
        mTimer.disarm();
        return false;
    }
}

bool PeriodicTask::cancel() NOEXCEPT {
    VSCOPE_FUNCTION();
    if (!mEvent.valid()) return false;

    mEvent.unregister();
    mTimer.disarm();
    return true;
}
}  // namespace scheduler
