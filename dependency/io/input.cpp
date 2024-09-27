#include "input.hpp"

namespace io {
Input::Input() NOEXCEPT : mScheduler(nullptr) {}
Input::~Input() NOEXCEPT = default;

bool Input::schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
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

bool Input::cancel() NOEXCEPT {
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
