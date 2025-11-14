#include <loglet/loglet.hpp>
#include "../config.hpp"

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
#include <args.hpp>
#pragma GCC diagnostic pop

namespace rtcm2eph {

static args::Group gGroup{"RTCM to Ephemeris:"};
static args::Flag  gEnable{
    gGroup,
    "enable",
    "Enable RTCM to Ephemeris conversion",
     {"rtcm2eph"},
};

static args::Flag gNoGps{
    gGroup,
    "gps",
    "Do not convert GPS ephemeris",
    {"r2e-no-gps"},
};
static args::Flag gNoGalileo{
    gGroup,
    "gal",
    "Do not convert Galileo ephemeris",
    {"r2e-no-gal"},
};
static args::Flag gNoBeidou{
    gGroup,
    "bds",
    "Do not convert BeiDou ephemeris",
    {"r2e-no-bds"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    auto& rtcm2eph   = config->rtcm2eph;
    rtcm2eph.enabled = false;
    rtcm2eph.gps     = true;
    rtcm2eph.galileo = true;
    rtcm2eph.beidou  = true;

    if (gEnable) rtcm2eph.enabled = true;
    if (gNoGps) rtcm2eph.gps = false;
    if (gNoGalileo) rtcm2eph.galileo = false;
    if (gNoBeidou) rtcm2eph.beidou = false;
}

void dump(Rtcm2EphConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
}

}  // namespace rtcm2eph
