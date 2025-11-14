#include "test.hpp"

#include <loglet/loglet.hpp>
#include <scheduler/periodic.hpp>
#include <scheduler/scheduler.hpp>

LOGLET_MODULE2(p, test);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(p, test)

static std::unique_ptr<scheduler::PeriodicTask> gPeriodicTask = nullptr;

void test_outputer(scheduler::Scheduler& scheduler, OutputConfig const& output, uint64_t tag) {
    VSCOPE_FUNCTION();

    ASSERT(gPeriodicTask == nullptr, "periodic task already exists");
    gPeriodicTask = std::unique_ptr<scheduler::PeriodicTask>(
        new scheduler::PeriodicTask(std::chrono::milliseconds(1000)));
    gPeriodicTask->set_event_name("test output");
    gPeriodicTask->callback = [&output, tag]() {
        VSCOPE_FUNCTION();

        uint8_t data[16 + 1] = "TESTTESTTESTTEST";
        DEBUGF("test: \"%s\"", data);

        for (auto& out : output.outputs) {
            if (!out.test_support()) continue;
            if (!out.accept_tag(tag)) {
                XVERBOSEF(OUTPUT_PRINT_MODULE, "tag %llX not accepted", tag);
                continue;
            }
            XDEBUGF(OUTPUT_PRINT_MODULE, "test: (%zd bytes) tag=%llX", sizeof(data), tag);

            ASSERT(out.stage, "stage is null");
            out.stage->write(OUTPUT_FORMAT_TEST, data, sizeof(data));
        }
    };

    if (!gPeriodicTask->schedule(scheduler)) {
        ERRORF("failed to schedule periodic task for test output");
    }
}
