#include "output.hpp"

namespace io {
Output::Output() NOEXCEPT : mScheduler(nullptr) {}
Output::~Output() = default;

bool Output::schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    if (mScheduler) {
        return false;
    }

    if (do_schedule(scheduler)) {
        mScheduler = &scheduler;
        return true;
    } else {
        return false;
    }
}

bool Output::cancel() NOEXCEPT {
    if (!mScheduler) {
        return false;
    }

    if (do_cancel(*mScheduler)) {
        mScheduler = nullptr;
        return true;
    } else {
        return false;
    }
}

}  // namespace io
