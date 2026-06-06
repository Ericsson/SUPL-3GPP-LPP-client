#include "../config.hpp"

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace lpp_static_repeat {

static args::Group          gGroup{"LPP Static Repeat:"};
static args::ValueFlag<int> gInterval{
    gGroup,
    "seconds",
    "Re-push cached RTK reference station / SSR correction point set every N seconds "
    "(0 or negative to disable)",
    {"lsr-interval"},
    0};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    auto& c    = config->lpp_static_repeat;
    c.interval = args::get(gInterval);
    c.enabled  = c.interval > 0;
}

}  // namespace lpp_static_repeat
