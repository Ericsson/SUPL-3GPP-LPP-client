#include "test.hpp"

#include <loglet/loglet.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE2(p, test);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, test)

static std::unique_ptr<scheduler::PeriodicTask> periodic_task = nullptr;

void test_outputer(scheduler::Scheduler& scheduler, OutputConfig const& output, uint64_t tag) {
    VSCOPE_FUNCTION();

    ASSERT(periodic_task == nullptr, "periodic task already exists");
    periodic_task = std::unique_ptr<scheduler::PeriodicTask>(
        new scheduler::PeriodicTask(std::chrono::milliseconds(1000)));
    periodic_task->set_event_name("test output");
    periodic_task->callback = [&output, tag]() {
        VSCOPE_FUNCTION();

        uint8_t data[16 + 1] = "TESTTESTTESTTEST";
        DEBUGF("test: \"%s\"", data);

        for (auto& output : output.outputs) {
            if (!output.test_support()) continue;
            if (!output.accept_tag(tag)) {
                XDEBUGF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
                continue;
            }
            if (output.print) {
                XINFOF(OUTPUT_PRINT_MODULE, "test: %zd bytes", sizeof(data));
            }

            ASSERT(output.stage, "stage is null");
            output.stage->write(OUTPUT_FORMAT_TEST, data, sizeof(data));
        }
    };
    periodic_task->schedule(scheduler);
}
