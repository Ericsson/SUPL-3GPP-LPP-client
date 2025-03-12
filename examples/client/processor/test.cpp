#include "test.hpp"

#include <loglet/loglet.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

#define LOGLET_CURRENT_MODULE "p/test"

static std::unique_ptr<scheduler::PeriodicTask> periodic_task = nullptr;

void test_outputer(scheduler::Scheduler& scheduler, OutputConfig const& output) {
    VSCOPE_FUNCTION();

    ASSERT(periodic_task == nullptr, "periodic task already exists");
    periodic_task = std::unique_ptr<scheduler::PeriodicTask>(
        new scheduler::PeriodicTask(std::chrono::milliseconds(1000)));
    periodic_task->set_event_name("test output");
    periodic_task->callback = [&]() {
        VSCOPE_FUNCTION();

        uint8_t data[16 + 1] = "TESTTESTTESTTEST";
        DEBUGF("test: \"%s\"", data);

        for (auto& out : output.outputs) {
            if (out.test_support()) {
                out.interface->write(data, sizeof(data));
            }
        }
    };
    periodic_task->schedule(scheduler);
}
