#include <cxx11_compat.hpp>
#include <loglet/loglet.hpp>
#include "../config.hpp"

#include <inttypes.h>

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

namespace agnss {

static args::Group gGroup{"A-GNSS:"};
static args::Flag  gEnable{
    gGroup,
    "enable",
    "Enable A-GNSS client",
     {"agnss-enable"},
};
static args::ValueFlag<std::string> gHost{
    gGroup, "host", "A-GNSS server hostname or IP address", {"agnss-host"}, args::Options::Single,
};
static args::ValueFlag<uint16_t> gPort{
    gGroup, "port", "A-GNSS server port", {"agnss-port"}, args::Options::Single,
};
static args::ValueFlag<std::string> gInterface{
    gGroup,
    "interface",
    "Interface to use for A-GNSS server connection",
    {"agnss-interface"},
    args::Options::Single,
};
static args::Flag gGps{
    gGroup,
    "gps",
    "Disable GPS assistance data",
    {"agnss-no-gps"},
};
static args::Flag gGlonass{
    gGroup,
    "glonass",
    "Disable GLONASS assistance data",
    {"agnss-no-glonass", "agnss-no-glo"},
};
static args::Flag gGalileo{
    gGroup,
    "galileo",
    "Disable Galileo assistance data",
    {"agnss-no-galileo", "agnss-no-gal"},
};
static args::Flag gBeidou{
    gGroup,
    "beidou",
    "Disable BeiDou assistance data",
    {"agnss-no-beidou", "agnss-no-bds"},
};
static args::ValueFlag<long> gInterval{
    gGroup, "interval", "Request interval in seconds (periodic mode)", {"agnss-interval"}, 60,
};
static args::ValueFlag<std::string> gMode{
    gGroup, "mode", "A-GNSS mode: periodic, triggered, both", {"agnss-mode"}, "periodic",
};
static args::ValueFlag<long> gTriggeredCooldown{
    gGroup,
    "cooldown",
    "Cooldown in seconds before re-requesting same ephemeris (triggered mode)",
    {"agnss-triggered-cooldown"},
    300,
};
static args::ValueFlag<uint64_t> gMsisdn{
    gGroup, "msisdn", "MSISDN identity", {"agnss-msisdn"}, args::Options::Single,
};
static args::ValueFlag<uint64_t> gImsi{
    gGroup, "imsi", "IMSI identity", {"agnss-imsi"}, args::Options::Single,
};
static args::ValueFlag<std::string> gIpv4{
    gGroup, "ipv4", "IPv4 address identity", {"agnss-ipv4"}, args::Options::Single,
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions globals{parser, gGroup};
}

void parse(Config* config) {
    auto& agnss   = config->agnss;
    agnss.enabled = gEnable.Get();

    if (!agnss.enabled) {
        return;
    }

    if (!gHost) {
        throw args::RequiredError("`--agnss-host` is required when A-GNSS is enabled");
    }
    if (!gPort) {
        throw args::RequiredError("`--agnss-port` is required when A-GNSS is enabled");
    }

    agnss.host = gHost.Get();
    agnss.port = gPort.Get();

    if (gInterface) {
        agnss.interface = std::unique_ptr<std::string>(new std::string(gInterface.Get()));
    }

    agnss.gps     = true;
    agnss.glonass = true;
    agnss.galileo = true;
    agnss.beidou  = true;

    if (gGps) agnss.gps = false;
    if (gGlonass) agnss.glonass = false;
    if (gGalileo) agnss.galileo = false;
    if (gBeidou) agnss.beidou = false;

    agnss.interval_seconds           = gInterval.Get();
    agnss.triggered_cooldown_seconds = gTriggeredCooldown.Get();

    auto mode_str = gMode.Get();
    if (mode_str == "periodic") {
        agnss.mode = AGnssMode::Periodic;
    } else if (mode_str == "triggered") {
        agnss.mode = AGnssMode::Triggered;
    } else if (mode_str == "both") {
        agnss.mode = AGnssMode::Both;
    } else {
        throw args::ValidationError("invalid A-GNSS mode: " + mode_str);
    }

    if (gMsisdn) {
        agnss.msisdn = std::unique_ptr<uint64_t>(new uint64_t(gMsisdn.Get()));
    }
    if (gImsi) {
        agnss.imsi = std::unique_ptr<uint64_t>(new uint64_t(gImsi.Get()));
    }
    if (gIpv4) {
        agnss.ipv4 = std::unique_ptr<std::string>(new std::string(gIpv4.Get()));
    }
}

void dump(AGnssConfig const& config) {
    DEBUGF("enabled: %s", config.enabled ? "true" : "false");
    if (!config.enabled) return;

    DEBUGF("host: \"%s\"", config.host.c_str());
    DEBUGF("port: %d", config.port);
    if (config.interface) {
        DEBUGF("interface: \"%s\"", config.interface.get()->c_str());
    }
    DEBUGF("gps: %s", config.gps ? "true" : "false");
    DEBUGF("glonass: %s", config.glonass ? "true" : "false");
    DEBUGF("galileo: %s", config.galileo ? "true" : "false");
    DEBUGF("beidou: %s", config.beidou ? "true" : "false");
    DEBUGF("mode: %s", config.mode == AGnssMode::Periodic  ? "periodic" :
                       config.mode == AGnssMode::Triggered ? "triggered" :
                                                             "both");
    DEBUGF("interval: %ld seconds", config.interval_seconds);
    DEBUGF("triggered_cooldown: %ld seconds", config.triggered_cooldown_seconds);
    if (config.msisdn) {
        DEBUGF("msisdn: %" PRIu64, *config.msisdn);
    }
    if (config.imsi) {
        DEBUGF("imsi: %" PRIu64, *config.imsi);
    }
    if (config.ipv4) {
        DEBUGF("ipv4: \"%s\"", config.ipv4->c_str());
    }
}

}  // namespace agnss
