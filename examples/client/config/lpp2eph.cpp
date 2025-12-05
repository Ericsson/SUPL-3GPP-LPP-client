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

namespace lpp2eph {

static args::Group gGroup{"LPP to Ephemeris:"};
static args::Flag  gDisable{
    gGroup,
    "disable",
    "Disable LPP to Ephemeris conversion",
     {"disable-lpp2eph"},
};

static args::Flag gNoGps{
    gGroup,
    "gps",
    "Do not convert GPS ephemeris",
    {"l2e-no-gps"},
};
static args::Flag gNoGalileo{
    gGroup,
    "gal",
    "Do not convert Galileo ephemeris",
    {"l2e-no-gal"},
};
static args::Flag gNoBeidou{
    gGroup,
    "bds",
    "Do not convert BeiDou ephemeris",
    {"l2e-no-bds"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    auto& lpp2eph   = config->lpp2eph;
    lpp2eph.enabled = true;
    lpp2eph.gps     = true;
    lpp2eph.galileo = true;
    lpp2eph.beidou  = true;

    if (gDisable) lpp2eph.enabled = false;
    if (gNoGps) lpp2eph.gps = false;
    if (gNoGalileo) lpp2eph.galileo = false;
    if (gNoBeidou) lpp2eph.beidou = false;
}

void dump(Lpp2EphConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
}

}  // namespace lpp2eph
