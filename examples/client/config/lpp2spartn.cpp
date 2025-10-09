#include "../config.hpp"
#include <loglet/loglet.hpp>

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

namespace lpp2spartn {

static args::Group gGroup{"LPP to SPARTN:"};
static args::Flag  gEnable{
    gGroup,
    "enable",
    "Enable LPP to SPARTN conversion",
     {"lpp2spartn"},
};

static args::Flag gNoGPS{
    gGroup,
    "gps",
    "Skip generating GPS messages",
    {"l2s-no-gps"},
};

static args::Flag gNoGLONASS{
    gGroup,
    "glo",
    "Skip generating GLONASS messages",
    {"l2s-no-glo"},
};

static args::Flag gNoGalileo{
    gGroup,
    "gal",
    "Skip generating Galileo messages",
    {"l2s-no-gal"},
};

static args::Flag gNoBeiDou{
    gGroup,
    "bds",
    "Skip generating BeiDou messages",
    {"l2s-no-bds"},
};

//
//
//

static args::Group          gSfGroup{gGroup, "SF???:"};
static args::ValueFlag<int> gUraOverride{
    gSfGroup,
    "0-7",
    "Override URA (SF024). If the value is unknown, set to 0.",
    {"l2s-ura-override", "l2s-sf024-override"},
    args::Options::Single,
};
static args::ValueFlag<int> gUraDefault{
    gSfGroup,
    "0-7",
    "Default URA (SF024), used when none is included in the LPP message. If the value is unknown, "
    "set to 0.",
    {"l2s-ura-default", "l2s-sf024-default"},
    args::Options::Single,
};

static args::ValueFlag<int> gSf042Override{
    gSfGroup,
    "0-7",
    "Override SF042. If the value is unknown, set to 0.",
    {"l2s-sf042-override"},
    args::Options::Single,
};
static args::ValueFlag<int> gSf042Default{
    gSfGroup,
    "0-7",
    "Default SF042, used when none is included in the LPP message. If the value is unknown, set to "
    "0.",
    {"l2s-sf042-default"},
    args::Options::Single,
};

static args::ValueFlag<int> gSf055Override{
    gSfGroup,
    "0-15",
    "Override SF055. If the value is invalid, set to 0.",
    {"l2s-sf055-override"},
    args::Options::Single,
};
static args::ValueFlag<int> gSf055Default{
    gSfGroup,
    "0-15",
    "Default SF055, used when none is included in the LPP message. If the value is invalid, set to "
    "0.",
    {"l2s-sf055-default"},
    args::Options::Single,
};

static args::Group gCommonGroup{gGroup, "Common:"};
static args::Flag  gNoGenerateGAD{
    gCommonGroup,
    "no-generate-gad",
    "Skip generating GAD messages",
     {"l2s-no-generate-gad"},
};
static args::Flag gNoGenerateOCB{
    gCommonGroup,
    "no-generate-ocb",
    "Skip generating OCB messages",
    {"l2s-no-generate-ocb"},
};
static args::Flag gNoGenerateHPAC{
    gCommonGroup,
    "no-generate-hpac",
    "Skip generating HPAC messages",
    {"l2s-no-generate-hpac"},
};
static args::Flag gFlipBitmask{
    gCommonGroup,
    "flip-grid-bitmask",
    "Flip the grid bitmask for incoming LPP messages",
    {"l2s-flip-grid-bitmask"},
};
static args::Flag gFlipClockCorrection{
    gCommonGroup,
    "flip-clock-correction",
    "Flip the sign of the clock correction",
    {"l2s-flip-clock-correction"},
};
static args::Flag gFlipOrbitCorrection{
    gCommonGroup,
    "flip-orbit-correction",
    "Flip the sign of the orbit correction",
    {"l2s-flip-orbit-correction"},
};
static args::Flag gNoAverageZenithDelay{
    gCommonGroup,
    "no-average-zenith-delay",
    "Do not compute the average zenith delay",
    {"l2s-no-average-zenith-delay"},
};
static args::Flag gIncreasingSiou{
    gCommonGroup,
    "increasing-siou",
    "Discard incoming LPP IoD values and use an increasing SIoU instead per HPAC",
    {"l2s-increasing-siou"},
};
static args::Flag gFilterByResiduals{
    gCommonGroup,
    "filter-by-residuals",
    "Filter out satellites in ionosphere corrections that do not have residuals for all grid "
    "points",
    {"l2s-filter-by-residuals"},
};
static args::Flag gFilterByOcb{
    gCommonGroup,
    "filter-by-ocb",
    "Filter out satellites in ionosphere corrections that do not have OCB corrections",
    {"l2s-filter-by-ocb"},
};
static args::Flag gDisableDoNotUseSatellite{
    gGroup,
    "disable-do-not-use-satellite",
    "Disable the do not use satellite flag",
    {"l2s-disable-dnu"},
};

static args::Group gSignalGroup{gGroup, "Signal:"};
static args::Flag  gIgnoreL2L{
    gSignalGroup,
    "ignore-l2l",
    "Ignore biases from L2L signals",
     {"l2s-ignore-l2l"},
};
static args::Flag gCodeBiasNoTranslate{
    gSignalGroup,
    "no-code-bias-translate",
    "Do not translate between code biases",
    {"l2s-no-code-bias-translate"},
};
static args::Flag gCodeBiasNoCorrectionShift{
    gSignalGroup,
    "no-code-bias-correction-shift",
    "Do not apply correction shift to code biases when translating",
    {"l2s-no-code-bias-correction-shift"},
};
static args::Flag gPhaseBiasNoTranslate{
    gSignalGroup,
    "no-phase-bias-translate",
    "Do not translate between phase biases",
    {"l2s-no-phase-bias-translate"},
};
static args::Flag gPhaseBiasNoCorrectionShift{
    gSignalGroup,
    "no-phase-bias-correction-shift",
    "Do not apply correction shift to phase biases when translating",
    {"l2s-no-phase-bias-correction-shift"},
};

static args::Group gTroposphereGroup{gGroup, "Troposphere:"};
static args::Flag  gHydrostaticInZenith{
    gTroposphereGroup,
    "hydrostatic-in-zenith",
    "Use the remaining hydrostatic delay residual in the per grid-point zenith residual",
     {"l2s-hydrostatic-in-zenith"},
};

static args::Group                  gIonosphereGroup{gGroup, "Ionosphere:"};
static args::ValueFlag<std::string> gStecMethod{
    gIonosphereGroup,      "method", "STEC method to use for the polynomial", {"l2s-stec-method"},
    args::Options::Single,
};
static args::Flag gStecTransform{
    gIonosphereGroup,
    "stec-transform",
    "Do not transform the STEC from LPP to SPARTN",
    {"l2s-no-stec-transform"},
};
static args::Flag gStecInvalidToZero{
    gIonosphereGroup,
    "stec-invalid-to-zero",
    "Set invalid STEC values to zero",
    {"l2s-stec-invalid-to-zero"},
};
static args::Flag gFlipStecResiduals{
    gIonosphereGroup,
    "sf-stec-residuals",
    "Flip the sign of the STEC residuals",
    {"l2s-sf-stec-residuals"},
};

static args::Group gCoefficientsGroup{gIonosphereGroup, "Coefficients:"};
static args::Flag  gSignFlipC00{
    gCoefficientsGroup,
    "sf-c00",
    "Flip the sign of the C00 coefficient",
     {"l2s-sf-c00"},
};
static args::Flag gSignFlipC01{
    gCoefficientsGroup,
    "sf-c01",
    "Flip the sign of the C01 coefficient",
    {"l2s-sf-c01"},
};
static args::Flag gSignFlipC10{
    gCoefficientsGroup,
    "sf-c10",
    "Flip the sign of the C10 coefficient",
    {"l2s-sf-c10"},
};
static args::Flag gSignFlipC11{
    gCoefficientsGroup,
    "sf-c11",
    "Flip the sign of the C11 coefficient",
    {"l2s-sf-c11"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions globals{parser, gGroup};
    gStecMethod.HelpChoices({"default", "discard", "residual"});
    gStecMethod.HelpDefault("default");
}

void parse(Config* config) {
    auto& lpp2spartn            = config->lpp2spartn;
    lpp2spartn.enabled          = false;
    lpp2spartn.generate_gps     = true;
    lpp2spartn.generate_glonass = true;
    lpp2spartn.generate_galileo = true;
    lpp2spartn.generate_beidou  = true;

    lpp2spartn.sf024_override = -1;
    lpp2spartn.sf024_default  = -1;
    lpp2spartn.sf042_override = -1;
    lpp2spartn.sf042_default  = -1;
    lpp2spartn.sf055_override = -1;
    lpp2spartn.sf055_default  = -1;

    lpp2spartn.generate_gad           = true;
    lpp2spartn.generate_ocb           = true;
    lpp2spartn.generate_hpac          = true;
    lpp2spartn.ublox_clock_correction = true;
    lpp2spartn.flip_grid_bitmask      = false;
    lpp2spartn.flip_orbit_correction  = false;
    lpp2spartn.average_zenith_delay   = true;
    lpp2spartn.increasing_siou        = false;
    lpp2spartn.filter_by_ocb          = false;
    lpp2spartn.filter_by_residuals    = false;

    lpp2spartn.hydrostatic_in_zenith = false;

    lpp2spartn.ignore_l2l                  = false;
    lpp2spartn.code_bias_translate         = true;
    lpp2spartn.code_bias_correction_shift  = true;
    lpp2spartn.phase_bias_translate        = true;
    lpp2spartn.phase_bias_correction_shift = true;

    lpp2spartn.stec_method              = generator::spartn::StecMethod::Default;
    lpp2spartn.stec_transform           = true;
    lpp2spartn.stec_invalid_to_zero     = false;
    lpp2spartn.sign_flip_c00            = false;
    lpp2spartn.sign_flip_c01            = false;
    lpp2spartn.sign_flip_c10            = false;
    lpp2spartn.sign_flip_c11            = false;
    lpp2spartn.sign_flip_stec_residuals = false;

    lpp2spartn.do_not_use_satellite = true;

    if (gEnable) lpp2spartn.enabled = true;
    if (gNoGPS) lpp2spartn.generate_gps = false;
    if (gNoGLONASS) lpp2spartn.generate_glonass = false;
    if (gNoGalileo) lpp2spartn.generate_galileo = false;
    if (gNoBeiDou) lpp2spartn.generate_beidou = false;

    if (gUraOverride) lpp2spartn.sf024_override = gUraOverride.Get();
    if (gUraDefault) lpp2spartn.sf024_default = gUraDefault.Get();
    if (gSf042Override) lpp2spartn.sf042_override = gSf042Override.Get();
    if (gSf042Default) lpp2spartn.sf042_default = gSf042Default.Get();
    if (gSf055Override) lpp2spartn.sf055_override = gSf055Override.Get();
    if (gSf055Default) lpp2spartn.sf055_default = gSf055Default.Get();

    if (gNoGenerateGAD) lpp2spartn.generate_gad = false;
    if (gNoGenerateOCB) lpp2spartn.generate_ocb = false;
    if (gNoGenerateHPAC) lpp2spartn.generate_hpac = false;

    if (gFlipBitmask) lpp2spartn.flip_grid_bitmask = true;
    if (gFlipClockCorrection) lpp2spartn.ublox_clock_correction = false;
    if (gFlipOrbitCorrection) lpp2spartn.flip_orbit_correction = true;

    if (gNoAverageZenithDelay) lpp2spartn.average_zenith_delay = false;
    if (gIncreasingSiou) lpp2spartn.increasing_siou = true;
    if (gFilterByResiduals) lpp2spartn.filter_by_residuals = true;
    if (gFilterByOcb) lpp2spartn.filter_by_ocb = true;

    if (gIgnoreL2L) lpp2spartn.ignore_l2l = true;
    if (gCodeBiasNoTranslate) lpp2spartn.code_bias_translate = false;
    if (gCodeBiasNoCorrectionShift) lpp2spartn.code_bias_correction_shift = false;
    if (gPhaseBiasNoTranslate) lpp2spartn.phase_bias_translate = false;
    if (gPhaseBiasNoCorrectionShift) lpp2spartn.phase_bias_correction_shift = false;

    if (gHydrostaticInZenith) lpp2spartn.hydrostatic_in_zenith = true;

    if (gDisableDoNotUseSatellite) lpp2spartn.do_not_use_satellite = false;

    if (gStecMethod) {
        auto method = gStecMethod.Get();
        if (method == "default") {
            lpp2spartn.stec_method = generator::spartn::StecMethod::Default;
        } else if (method == "discard") {
            lpp2spartn.stec_method = generator::spartn::StecMethod::DiscardC01C10C11;
        } else if (method == "residual") {
            lpp2spartn.stec_method = generator::spartn::StecMethod::MoveToResiduals;
        } else {
            throw args::ValidationError(
                "--l2s-stec-method: must be one of 'default', 'discard', or 'residual', got `" +
                method + "`");
        }
    }

    if (gStecTransform) lpp2spartn.stec_transform = false;
    if (gStecInvalidToZero) lpp2spartn.stec_invalid_to_zero = true;
    if (gFlipStecResiduals) lpp2spartn.sign_flip_stec_residuals = true;

    if (gSignFlipC00) lpp2spartn.sign_flip_c00 = true;
    if (gSignFlipC01) lpp2spartn.sign_flip_c01 = true;
    if (gSignFlipC10) lpp2spartn.sign_flip_c10 = true;
    if (gSignFlipC11) lpp2spartn.sign_flip_c11 = true;

    if (gUraOverride && (lpp2spartn.sf024_override < 0 || lpp2spartn.sf024_override > 7)) {
        throw args::ValidationError("URA override must be between 0 and 7, got `" +
                                    std::to_string(lpp2spartn.sf024_override) + "`");
    } else if (gUraDefault && (lpp2spartn.sf024_default < 0 || lpp2spartn.sf024_default > 7)) {
        throw args::ValidationError("URA default must be between 0 and 7, got `" +
                                    std::to_string(lpp2spartn.sf024_default) + "`");
    } else if (gSf042Override && (lpp2spartn.sf042_override < 0 || lpp2spartn.sf042_override > 7)) {
        throw args::ValidationError("SF042 override must be between 0 and 7, got `" +
                                    std::to_string(lpp2spartn.sf042_override) + "`");
    } else if (gSf042Default && (lpp2spartn.sf042_default < 0 || lpp2spartn.sf042_default > 7)) {
        throw args::ValidationError("SF042 default must be between 0 and 7, got `" +
                                    std::to_string(lpp2spartn.sf042_default) + "`");
    } else if (gSf055Override &&
               (lpp2spartn.sf055_override < 0 || lpp2spartn.sf055_override > 15)) {
        throw args::ValidationError("SF055 override must be between 0 and 15, got `" +
                                    std::to_string(lpp2spartn.sf055_override) + "`");
    } else if (gSf055Default && (lpp2spartn.sf055_default < 0 || lpp2spartn.sf055_default > 15)) {
        throw args::ValidationError("SF055 default must be between 0 and 15, got `" +
                                    std::to_string(lpp2spartn.sf055_default) + "`");
    }
}

void dump(Lpp2SpartnConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:    %s", config.generate_gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.generate_glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.generate_galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.generate_beidou ? "enabled" : "disabled");

    DEBUGF("SF024: override=%d, default=%d", config.sf024_override, config.sf024_default);
    DEBUGF("SF042: override=%d, default=%d", config.sf042_override, config.sf042_default);
    DEBUGF("SF055: override=%d, default=%d", config.sf055_override, config.sf055_default);

    DEBUGF("GAD:  %s", config.generate_gad ? "enabled" : "disabled");
    DEBUGF("OCB:  %s", config.generate_ocb ? "enabled" : "disabled");
    DEBUGF("HPAC: %s", config.generate_hpac ? "enabled" : "disabled");

    DEBUGF("grid bitmask: %s", config.flip_grid_bitmask ? "flipped" : "normal");
    DEBUGF("clock correction: %s", config.ublox_clock_correction ? "normal" : "flipped");
    DEBUGF("orbit correction: %s", config.flip_orbit_correction ? "flipped" : "normal");
    DEBUGF("average zenith delay: %s", config.average_zenith_delay ? "true" : "false");
    DEBUGF("increasing SIoU: %s", config.increasing_siou ? "true" : "false");
    DEBUGF("filter by residuals: %s", config.filter_by_residuals ? "true" : "false");
    DEBUGF("filter by OCB: %s", config.filter_by_ocb ? "true" : "false");

    DEBUGF("ignore L2L: %s", config.ignore_l2l ? "true" : "false");
    DEBUGF("code bias translate: %s", config.code_bias_translate ? "true" : "false");
    DEBUGF("code bias correction shift: %s", config.code_bias_correction_shift ? "true" : "false");
    DEBUGF("phase bias translate: %s", config.phase_bias_translate ? "true" : "false");
    DEBUGF("phase bias correction shift: %s",
           config.phase_bias_correction_shift ? "true" : "false");

    DEBUGF("hydrostatic in zenith: %s", config.hydrostatic_in_zenith ? "true" : "false");

    DEBUGF("STEC method: %s", [&]() {
        switch (config.stec_method) {
        case generator::spartn::StecMethod::Default: return "default";
        case generator::spartn::StecMethod::DiscardC01C10C11: return "discard";
        case generator::spartn::StecMethod::MoveToResiduals: return "residual";
        }

        UNREACHABLE();
    }());
    DEBUGF("STEC transform: %s", config.stec_transform ? "true" : "false");
    DEBUGF("STEC invalid to zero: %s", config.stec_invalid_to_zero ? "true" : "false");
    DEBUGF("STEC residuals: %s", config.sign_flip_stec_residuals ? "flipped" : "normal");

    DEBUGF("C00: %s", config.sign_flip_c00 ? "flipped" : "normal");
    DEBUGF("C01: %s", config.sign_flip_c01 ? "flipped" : "normal");
    DEBUGF("C10: %s", config.sign_flip_c10 ? "flipped" : "normal");
    DEBUGF("C11: %s", config.sign_flip_c11 ? "flipped" : "normal");
}

}  // namespace lpp2spartn
