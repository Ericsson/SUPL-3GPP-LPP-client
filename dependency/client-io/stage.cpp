#include <client-io/stage.hpp>
#include <loglet/loglet.hpp>

LOGLET_MODULE(client_io_stage);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(client_io_stage)

OutputStage::OutputStage() NOEXCEPT : mScheduler(nullptr) {}
OutputStage::~OutputStage() = default;

bool OutputStage::schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    mScheduler = &scheduler;
    return do_schedule(scheduler);
}

bool OutputStage::cancel() NOEXCEPT {
    if (!mScheduler) return true;
    return do_cancel(*mScheduler);
}

InputStage::InputStage() NOEXCEPT : mScheduler(nullptr) {}
InputStage::~InputStage() = default;

bool InputStage::schedule(scheduler::Scheduler& scheduler) NOEXCEPT {
    mScheduler = &scheduler;
    return do_schedule(scheduler);
}

bool InputStage::cancel() NOEXCEPT {
    if (!mScheduler) return true;
    return do_cancel(*mScheduler);
}
