#include <loglet/loglet.hpp>
#include "../config.hpp"

#ifdef INCLUDE_GENERATOR_IDOKEIDO

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

#include <generator/idokeido/idokeido.hpp>

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
    gGroup, "update-rate", "Navigation solution update rate", {"ido-update-rate"}, 1.0};

static args::ValueFlag<std::string> gEphemerisCache{
    gGroup, "ephemeris-cache", "Ephemeris cache", {"ido-ephemeris-cache"}};

static args::ValueFlag<std::string> gRelativistic{
    gGroup, "relativistic", "Relativistic clock correction model", {"ido-rel"}};

static args::ValueFlag<std::string> gEpoch{gGroup, "epoch", "Epoch selection mode", {"ido-epoch"}};

static args::ValueFlag<double> gObservationWindow{
    gGroup,
    "ms",
    "The maximum time difference (in ms) between observations and epochs to be selected",
    {"ido-epoch-window"},
    100.0,
};

static args::ValueFlag<std::string> gIonosphericModel{
    gGroup,
    "ionospheric",
    "Ionospheric correction model",
    {"ido-iono"},
};

static args::ValueFlag<std::string> gWeightModel{
    gGroup,
    "weight",
    "Observation weight model",
    {"ido-weight"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions globals{parser, gGroup};
    gRelativistic.HelpChoices({"none", "brdc", "dotrv"});
    gRelativistic.HelpDefault("brdc");

    gEpoch.HelpChoices({"first", "last", "mean"});
    gEpoch.HelpDefault("last");

    gIonosphericModel.HelpChoices({"none", "brdc", "dual", "ssr"});
    gIonosphericModel.HelpDefault("none");

    gWeightModel.HelpChoices({"none", "snr", "elevation", "variance"});
    gWeightModel.HelpDefault("none");
}

void parse(Config* config) {
    auto& cfg              = config->idokeido;
    cfg.enabled            = false;
    cfg.gps                = true;
    cfg.glonass            = true;
    cfg.galileo            = true;
    cfg.beidou             = true;
    cfg.update_rate        = 1.0;
    cfg.ephemeris_cache    = "";
    cfg.relativistic_model = ::idokeido::RelativisticModel::Broadcast;
    cfg.ionospheric_mode   = ::idokeido::IonosphericMode::None;
    cfg.weight_function    = ::idokeido::WeightFunction::None;
    cfg.epoch_selection    = ::idokeido::EpochSelection::LastObservation;
    cfg.observation_window = 0.1;

    if (gEnable) cfg.enabled = true;
    if (gNoGPS) cfg.gps = false;
    if (gNoGLONASS) cfg.glonass = false;
    if (gNoGalileo) cfg.galileo = false;
    if (gNoBeiDou) cfg.beidou = false;

    cfg.update_rate = gUpdateRate.Get();

    if (gEphemerisCache) {
        cfg.ephemeris_cache = gEphemerisCache.Get();
    }

    if (gRelativistic) {
        if (gRelativistic.Get() == "none") {
            cfg.relativistic_model = ::idokeido::RelativisticModel::None;
        } else if (gRelativistic.Get() == "brdc") {
            cfg.relativistic_model = ::idokeido::RelativisticModel::Broadcast;
        } else if (gRelativistic.Get() == "dotrv") {
            cfg.relativistic_model = ::idokeido::RelativisticModel::Dotrv;
        } else {
            throw std::runtime_error("unknown relativistic model: " + gRelativistic.Get());
        }
    }

    if (gEpoch) {
        if (gEpoch.Get() == "first") {
            cfg.epoch_selection = ::idokeido::EpochSelection::FirstObservation;
        } else if (gEpoch.Get() == "last") {
            cfg.epoch_selection = ::idokeido::EpochSelection::LastObservation;
        } else if (gEpoch.Get() == "mean") {
            cfg.epoch_selection = ::idokeido::EpochSelection::MeanObservation;
        } else {
            throw std::runtime_error("unknown epoch selection mode: " + gEpoch.Get());
        }
    }

    if (gIonosphericModel) {
        if (gIonosphericModel.Get() == "none") {
            cfg.ionospheric_mode = ::idokeido::IonosphericMode::None;
        } else if (gIonosphericModel.Get() == "brdc") {
            cfg.ionospheric_mode = ::idokeido::IonosphericMode::Broadcast;
        } else if (gIonosphericModel.Get() == "dual") {
            cfg.ionospheric_mode = ::idokeido::IonosphericMode::Dual;
        } else if (gIonosphericModel.Get() == "ssr") {
            cfg.ionospheric_mode = ::idokeido::IonosphericMode::Ssr;
        } else {
            throw std::runtime_error("unknown ionospheric model: " + gIonosphericModel.Get());
        }
    }

    if (gWeightModel) {
        if (gWeightModel.Get() == "none") {
            cfg.weight_function = ::idokeido::WeightFunction::None;
        } else if (gWeightModel.Get() == "snr") {
            cfg.weight_function = ::idokeido::WeightFunction::Snr;
        } else if (gWeightModel.Get() == "elevation") {
            cfg.weight_function = ::idokeido::WeightFunction::Elevation;
        } else if (gWeightModel.Get() == "variance") {
            cfg.weight_function = ::idokeido::WeightFunction::Variance;
        } else {
            throw std::runtime_error("unknown weight model: " + gWeightModel.Get());
        }
    }

    if (gObservationWindow) {
        cfg.observation_window = gObservationWindow.Get() / 1000.0;
    }
}

void dump(IdokeidoConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
    DEBUGF("update_rate: %f", config.update_rate);
    DEBUGF("ephemeris_cache: %s", config.ephemeris_cache.c_str());
    DEBUGF("relativistic_model: %s", [&]() {
        switch (config.relativistic_model) {
        case ::idokeido::RelativisticModel::None: return "none";
        case ::idokeido::RelativisticModel::Broadcast: return "broadcast";
        case ::idokeido::RelativisticModel::Dotrv: return "dotrv";
        }
        return "unknown";
    }());
    DEBUGF("ionospheric_mode: %s", [&]() {
        switch (config.ionospheric_mode) {
        case ::idokeido::IonosphericMode::None: return "none";
        case ::idokeido::IonosphericMode::Broadcast: return "broadcast";
        case ::idokeido::IonosphericMode::Dual: return "dual";
        case ::idokeido::IonosphericMode::Ssr: return "ssr";
        }
        return "unknown";
    }());
    DEBUGF("weight_function: %s", [&]() {
        switch (config.weight_function) {
        case ::idokeido::WeightFunction::None: return "none";
        case ::idokeido::WeightFunction::Snr: return "snr";
        case ::idokeido::WeightFunction::Elevation: return "elevation";
        case ::idokeido::WeightFunction::Variance: return "variance";
        }
        return "unknown";
    }());
    DEBUGF("epoch_selection: %s", [&]() {
        switch (config.epoch_selection) {
        case ::idokeido::EpochSelection::FirstObservation: return "first";
        case ::idokeido::EpochSelection::LastObservation: return "last";
        case ::idokeido::EpochSelection::MeanObservation: return "mean";
        }
        return "unknown";
    }());
    DEBUGF("observation_window: %fs", config.observation_window);
}

}  // namespace idokeido

#endif
