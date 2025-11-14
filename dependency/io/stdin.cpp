#include "stdin.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <scheduler/file_descriptor.hpp>
#include <scheduler/scheduler.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, stdio);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, stdio)

namespace io {
StdinInput::StdinInput() NOEXCEPT {
    VSCOPE_FUNCTION();
}

StdinInput::~StdinInput() NOEXCEPT {
    VSCOPE_FUNCTION();
    cancel();
}

bool StdinInput::do_schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    mFdTask.reset(new scheduler::FileDescriptorTask());
    // Return value indicates rescheduling need, not failure - safe to ignore for new task
    (void)mFdTask->set_fd(STDIN_FILENO);
    mFdTask->on_read = [this](int) {
        auto result = ::read(STDIN_FILENO, mBuffer, sizeof(mBuffer));
        VERBOSEF("::read(%d, %p, %zu) = %d", STDIN_FILENO, mBuffer, sizeof(mBuffer), result);
        if (result < 0) {
            ERRORF("failed to read from stdin: " ERRNO_FMT, ERRNO_ARGS(errno));
            cancel();
            if (on_complete) on_complete();
            return;
        }

        if (result == 0) {
            cancel();
            if (on_complete) on_complete();
            return;
        }

        if (callback) {
            callback(*this, mBuffer, static_cast<size_t>(result));
        }
    };
    mFdTask->on_error = [this](int) {
        cancel();
        if (on_complete) on_complete();
    };

    return mFdTask->schedule(scheduler);
}

bool StdinInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mFdTask) {
        return mFdTask->cancel();
    }

    return true;
}
}  // namespace io
