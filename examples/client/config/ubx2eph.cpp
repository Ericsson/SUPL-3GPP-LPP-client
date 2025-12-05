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

namespace ubx2eph {

static args::Group gGroup{"UBX to Ephemeris:"};
static args::Flag  gDisable{
    gGroup,
    "disable",
    "Disable UBX to Ephemeris conversion",
     {"disable-ubx2eph"},
};

static args::Flag gNoGps{
    gGroup,
    "gps",
    "Do not convert GPS ephemeris",
    {"u2e-no-gps"},
};
static args::Flag gNoGalileo{
    gGroup,
    "gal",
    "Do not convert Galileo ephemeris",
    {"u2e-no-gal"},
};
static args::Flag gNoBeidou{
    gGroup,
    "bds",
    "Do not convert BeiDou ephemeris",
    {"u2e-no-bds"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    auto& ubx2eph   = config->ubx2eph;
    ubx2eph.enabled = true;
    ubx2eph.gps     = true;
    ubx2eph.galileo = true;
    ubx2eph.beidou  = true;

    if (gDisable) ubx2eph.enabled = false;
    if (gNoGps) ubx2eph.gps = false;
    if (gNoGalileo) ubx2eph.galileo = false;
    if (gNoBeidou) ubx2eph.beidou = false;
}

void dump(Ubx2EphConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
}

}  // namespace ubx2eph
