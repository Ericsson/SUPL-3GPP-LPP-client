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

namespace gnss {

static args::Group gGroup{"GNSS:"};
static args::Flag  gGps{
    gGroup,
    "gps",
    "Disable GPS usage",
     {"no-gps"},
};
static args::Flag gGlonass{
    gGroup,
    "glonass",
    "Disable GLONASS usage",
    {"no-glonass", "no-glo"},
};
static args::Flag gGalileo{
    gGroup,
    "galileo",
    "Disable Galileo usage",
    {"no-galileo", "no-gal"},
};
static args::Flag gBds{
    gGroup,
    "bds",
    "Disable BeiDou usage",
    {"no-beidou", "no-bds"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    auto& gnss   = config->gnss;
    gnss.gps     = true;
    gnss.glonass = true;
    gnss.galileo = true;
    gnss.beidou  = true;

    if (gGps) gnss.gps = false;
    if (gGlonass) gnss.glonass = false;
    if (gGalileo) gnss.galileo = false;
    if (gBds) gnss.beidou = false;
}

void dump(GnssConfig const& config) {
    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
}

}  // namespace gnss
