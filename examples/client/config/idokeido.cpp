#include "config.hpp"

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

namespace idokeido {

static args::Group gGroup{"Idokeido:"};

static args::Flag gEnable{
    gGroup,
    "enable",
    "Enable Idokeido engine",
    {"idokeido"},
};

static args::Flag gNoGPS{
    gGroup,
    "no-gps",
    "Disable GPS",
    {"ido-no-gps"},
};

static args::Flag gNoGLONASS{
    gGroup,
    "no-glonass",
    "Disable GLONASS",
    {"ido-no-glonass"},
};

static args::Flag gNoGalileo{
    gGroup,
    "no-galileo",
    "Disable Galileo",
    {"ido-no-galileo"},
};

static args::Flag gNoBeiDou{
    gGroup,
    "no-beidou",
    "Disable BeiDou",
    {"ido-no-beidou"},
};

static args::ValueFlag<double> gUpdateRate{
    gGroup,
    "update-rate",
    "Navigation solution update rate",
    {"ido-update-rate"},
    1.0
};

static args::ValueFlag<std::string> gEphemerisCache{
    gGroup,
    "ephemeris-cache",
    "Ephemeris cache",
    {"ido-ephemeris-cache"}
};

static void setup() {

}

static void parse(Config* config) {
    auto& cfg            = config->idokeido;
    cfg.enabled          = false;
    cfg.gps              = true;
    cfg.glonass          = true;
    cfg.galileo          = true;
    cfg.beidou           = true;
    cfg.update_rate      = 1.0;
    cfg.ephemeris_cache = "";

    if (gEnable) cfg.enabled = true;
    if (gNoGPS) cfg.gps = false;
    if (gNoGLONASS) cfg.glonass = false;
    if (gNoGalileo) cfg.galileo = false;
    if (gNoBeiDou) cfg.beidou = false;

    cfg.update_rate = gUpdateRate.Get();

    if (gEphemerisCache) {
        cfg.ephemeris_cache = gEphemerisCache.Get();
    }
}

static void dump(IdokeidoConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
    DEBUGF("update_rate: %f", config.update_rate);
    DEBUGF("ephemeris_cache: %s", config.ephemeris_cache.c_str());
}

}  // namespace tokoro
