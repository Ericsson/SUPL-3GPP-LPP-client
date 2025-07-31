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

namespace lpp2rtcm {

static args::Group gGroup{"LPP to RTCM:"};
static args::Flag  gEnable{
    gGroup,
    "enable",
    "Enable LPP to RTCM conversion",
     {"lpp2rtcm"},
};

static args::Flag gNoGps{
    gGroup,
    "gps",
    "Do not generate RTCM messages for GPS",
    {"l2r-no-gps"},
};
static args::Flag gNoGlonass{
    gGroup,
    "glo",
    "Do not generate RTCM messages for GLONASS",
    {"l2r-no-glo"},
};
static args::Flag gNoGalileo{
    gGroup,
    "gal",
    "Do not generate RTCM messages for Galileo",
    {"l2r-no-gal"},
};
static args::Flag gNoBeidou{
    gGroup,
    "bds",
    "Do not generate RTCM messages for BDS",
    {"l2r-no-bds"},
};

static args::ValueFlag<std::string> gMsmType{
    gGroup,
    "type",
    "Which MSM type to generate",
    {"l2r-msm"},
};

static void setup() {
    gMsmType.HelpDefault("any");
    gMsmType.HelpChoices({"any", "4", "5", "6", "7"});
}

static void parse(Config* config) {
    auto& lpp2rtcm            = config->lpp2rtcm;
    lpp2rtcm.enabled          = false;
    lpp2rtcm.generate_gps     = true;
    lpp2rtcm.generate_glonass = true;
    lpp2rtcm.generate_galileo = true;
    lpp2rtcm.generate_beidou  = true;
    lpp2rtcm.msm_type         = Lpp2RtcmConfig::MsmType::ANY;

    if (gEnable) lpp2rtcm.enabled = true;
    if (gNoGps) lpp2rtcm.generate_gps = false;
    if (gNoGlonass) lpp2rtcm.generate_glonass = false;
    if (gNoGalileo) lpp2rtcm.generate_galileo = false;
    if (gNoBeidou) lpp2rtcm.generate_beidou = false;

    if (gMsmType) {
        auto type = gMsmType.Get();
        if (type == "any")
            lpp2rtcm.msm_type = Lpp2RtcmConfig::MsmType::ANY;
        else if (type == "4")
            lpp2rtcm.msm_type = Lpp2RtcmConfig::MsmType::MSM4;
        else if (type == "5")
            lpp2rtcm.msm_type = Lpp2RtcmConfig::MsmType::MSM5;
        else if (type == "6")
            lpp2rtcm.msm_type = Lpp2RtcmConfig::MsmType::MSM6;
        else if (type == "7")
            lpp2rtcm.msm_type = Lpp2RtcmConfig::MsmType::MSM7;
        else
            throw args::ParseError("--lpp2rtcm-msm not recognized: `" + type + "`");
    }
}

static void dump(Lpp2RtcmConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.generate_gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.generate_glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.generate_galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.generate_beidou ? "enabled" : "disabled");

    DEBUGF("msm type: %s", [&]() {
        switch (config.msm_type) {
        case Lpp2RtcmConfig::MsmType::ANY: return "any";
        case Lpp2RtcmConfig::MsmType::MSM4: return "4";
        case Lpp2RtcmConfig::MsmType::MSM5: return "5";
        case Lpp2RtcmConfig::MsmType::MSM6: return "6";
        case Lpp2RtcmConfig::MsmType::MSM7: return "7";
        }
        return "unknown";
    }());
}

}  // namespace lpp2rtcm
