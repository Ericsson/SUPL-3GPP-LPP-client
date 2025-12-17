#include <loglet/loglet.hpp>
#include "../config.hpp"

#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#include <args.hxx>
#pragma GCC diagnostic pop

namespace scheduler {

static args::Group          gGroup{"Scheduler:"};
static args::ValueFlag<int> gMaxEventsPerWait{
    gGroup,
    "max_events",
    "Maximum number of events to process per wait",
    {"scheduler-max-events"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    auto& scheduler               = config->scheduler;
    scheduler.max_events_per_wait = 1;

    if (gMaxEventsPerWait) {
        scheduler.max_events_per_wait = args::get(gMaxEventsPerWait);
    }
}

void dump(SchedulerConfig const& config) {
    DEBUGF("max_events_per_wait: %d", config.max_events_per_wait);
}

}  // namespace scheduler
