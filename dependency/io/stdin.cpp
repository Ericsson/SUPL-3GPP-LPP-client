#include "stdin.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <scheduler/file_descriptor.hpp>
#include <scheduler/scheduler.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, stdio);
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
    mFdTask->set_fd(STDIN_FILENO);
    mFdTask->on_read = [this](int) {
        auto result = ::read(STDIN_FILENO, mBuffer, sizeof(mBuffer));
        VERBOSEF("::read(%d, %p, %zu) = %d", STDIN_FILENO, mBuffer, sizeof(mBuffer), result);
        if (result < 0) {
            ERRORF("failed to read from stdin: " ERRNO_FMT, ERRNO_ARGS(errno));
            cancel();
            return;
        }

        if (callback) {
            callback(*this, mBuffer, static_cast<size_t>(result));
        }
    };
    mFdTask->on_error = [this](int) {
        // NOTE(ewasjon): I am not sure what to do here.
        cancel();
    };

    mFdTask->schedule(scheduler);
    return true;
}

bool StdinInput::do_cancel(scheduler::Scheduler& scheduler) NOEXCEPT {
    VSCOPE_FUNCTIONF("%p", &scheduler);

    if (mFdTask) {
        mFdTask->cancel();
        mFdTask.reset();
    }

    return true;
}
}  // namespace io
