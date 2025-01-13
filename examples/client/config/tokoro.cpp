
namespace tokoro {

static args::Group gGroup{"Tokoro:"};

static args::Flag gEnable{
    gGroup,
    "enable",
    "Enable Tokoro generation",
    {"tokoro"},
};

static args::Flag gNoGPS{
    gGroup,
    "no-gps",
    "Skip generating GPS messages",
    {"tkr-no-gps"},
};

static args::Flag gNoGLONASS{
    gGroup,
    "no-glonass",
    "Skip generating GLONASS messages",
    {"tkr-no-glonass"},
};

static args::Flag gNoGalileo{
    gGroup,
    "no-galileo",
    "Skip generating Galileo messages",
    {"tkr-no-galileo"},
};

static args::Flag gNoBeiDou{
    gGroup,
    "no-beidou",
    "Skip generating BeiDou messages",
    {"tkr-no-beidou"},
};

static args::ValueFlag<std::string> gVrsModeArg{
    gGroup,
    "vrs-mode",
    "VRS mode",
    {"tkr-vrs-mode"},
};

static args::ValueFlag<std::string> gGenStrategyArg{
    gGroup,
    "strategy",
    "Generation strategy",
    {"tkr-gen"},
};

static args::ValueFlag<double> gDynamicDistThresholdArg{
    gGroup,
    "km",
    "Distance threshold for dynamic VRS mode (<= 0 means every time)",
    {"tkr-distance-threshold"},
};

static args::ValueFlag<double> gFixedItrfX{
    gGroup,
    "meter",
    "X coordinate for ITRF system in Fixed VRS mode",
    {"tkr-fixed-itrf-x"},
};

static args::ValueFlag<double> gFixedItrfY{
    gGroup,
    "meter",
    "Y coordinate for ITRF system in Fixed VRS mode",
    {"tkr-fixed-itrf-y"},
};

static args::ValueFlag<double> gFixedItrfZ{
    gGroup,
    "meter",
    "Z coordinate for ITRF system in Fixed VRS mode",
    {"tkr-fixed-itrf-z"},
};

static args::ValueFlag<double> gFixedRtcmX{
    gGroup,
    "meter",
    "X coordinate for RTCM system in Fixed VRS mode",
    {"tkr-fixed-rtcm-x"},
};

static args::ValueFlag<double> gFixedRtcmY{
    gGroup,
    "meter",
    "Y coordinate for RTCM system in Fixed VRS mode",
    {"tkr-fixed-rtcm-y"},
};

static args::ValueFlag<double> gFixedRtcmZ{
    gGroup,
    "meter",
    "Z coordinate for RTCM system in Fixed VRS mode",
    {"tkr-fixed-rtcm-z"},
};

static args::ValueFlag<double> gTimeStep{
    gGroup,
    "seconds",
    "Time step (how often to generate messages) used with\n`--tkr-gen time-step` or\n"
    "`--tkr-gen time-step-aligned`",
    {"tkr-time-step"},
};

static args::Flag gShapiroCorrection{
    gGroup,
    "tkr-no-shapiro-correction",
    "Disable shapiro correction",
    {"tkr-no-sc"},
};

static args::Flag gPhaseWindupCorrection{
    gGroup,
    "tkr-no-phase-windup-correction",
    "Disable phase windup correction",
    {"tkr-no-pwc"},
};

static args::Flag gEarthSolidTidesCorrection{
    gGroup,
    "tkr-no-earth-solid-tides-correction",
    "Disable earth solid tides correction",
    {"tkr-no-estc"},
};

static args::Flag gAntennaPhaseVariationCorrection{
    gGroup,
    "tkr-no-antenna-phase-variation-correction",
    "Disable antenna phase variation correction",
    {"tkr-no-apvc"},
};

static args::Flag gTroposphericHeightCorrection{
    gGroup,
    "tkr-no-tropospheric-height-correction",
    "Disable tropospheric height correction",
    {"tkr-no-thc"},
};

static args::Flag gIodConsistencyCheck{
    gGroup,
    "tkr-no-iod-consistency-check",
    "Disable Issue of Data consistency check for ephemeris",
    {"tkr-no-icc"},
};

static args::Flag gRtOC{
    gGroup,
    "rtoc",
    "Use reception time in orbit correction instead of transmission time",
    {"tkr-rtoc"},
};

static args::Flag gOcit{
    gGroup,
    "ocit",
    "Use orbit corrections in satellite position evaluation",
    {"tkr-ocit"},
};

static args::Flag gNegativePhaseWindup{
    gGroup,
    "negative-phase-windup",
    "Enable negative phase windup correction",
    {"tkr-npw"},
};

static void setup() {
    gVrsModeArg.HelpChoices({"fixed", "dynamic"});
    gVrsModeArg.HelpDefault("dynamic");

    gGenStrategyArg.HelpChoices({"assistance", "time-step", "time-step-aligned"});
    gGenStrategyArg.HelpDefault("assistance");

    gTimeStep.HelpDefault("1.0");
    gDynamicDistThresholdArg.HelpDefault("5.0");
}

static void parse(Config* config) {
    auto& tokoro            = config->tokoro;
    tokoro.enabled          = false;
    tokoro.generate_gps     = true;
    tokoro.generate_glonass = true;
    tokoro.generate_galileo = true;
    tokoro.generate_beidou  = true;

    tokoro.shapiro_correction                 = true;
    tokoro.phase_windup_correction            = true;
    tokoro.earth_solid_tides_correction       = true;
    tokoro.antenna_phase_variation_correction = true;
    tokoro.tropospheric_height_correction     = true;
    tokoro.iod_consistency_check              = true;
    tokoro.rtoc                               = false;
    tokoro.ocit                               = false;
    tokoro.negative_phase_windup              = false;

    tokoro.vrs_mode                   = TokoroConfig::VrsMode::Dynamic;
    tokoro.dynamic_distance_threshold = 5.0;
    tokoro.fixed_itrf_x               = 0.0;
    tokoro.fixed_itrf_y               = 0.0;
    tokoro.fixed_itrf_z               = 0.0;
    tokoro.fixed_rtcm_x               = 0.0;
    tokoro.fixed_rtcm_y               = 0.0;
    tokoro.fixed_rtcm_z               = 0.0;

    tokoro.generation_strategy = TokoroConfig::GenerationStrategy::AssistanceData;
    tokoro.time_step           = 1.0;

    if (gEnable) tokoro.enabled = true;
    if (gNoGPS) tokoro.generate_gps = false;
    if (gNoGLONASS) tokoro.generate_glonass = false;
    if (gNoGalileo) tokoro.generate_galileo = false;
    if (gNoBeiDou) tokoro.generate_beidou = false;

    if (gVrsModeArg) {
        auto v = gVrsModeArg.Get();
        if (v == "dynamic") {
            tokoro.vrs_mode = TokoroConfig::VrsMode::Dynamic;
        } else if (v == "fixed") {
            tokoro.vrs_mode = TokoroConfig::VrsMode::Fixed;
        } else {
            throw args::ValidationError("--tkr-vrs-mode must be 'fixed' or 'dynamic', got `" + v +
                                        "`");
        }
    }

    if (gGenStrategyArg) {
        auto s = gGenStrategyArg.Get();
        if (s == "time-step") {
            tokoro.generation_strategy = TokoroConfig::GenerationStrategy::TimeStep;
        } else if (s == "time-step-aligned") {
            tokoro.generation_strategy = TokoroConfig::GenerationStrategy::TimeStepAligned;
        } else if (s == "assistance") {
            tokoro.generation_strategy = TokoroConfig::GenerationStrategy::AssistanceData;
        } else {
            throw args::ValidationError("--tkr-gen must be 'assistance', 'time-step', or "
                                        "'time-step-aligned', got `" +
                                        s + "`");
        }
    }

    if (gDynamicDistThresholdArg) {
        tokoro.dynamic_distance_threshold = gDynamicDistThresholdArg.Get();
    }

    if (gTimeStep) {
        tokoro.time_step = gTimeStep.Get();
        if (tokoro.time_step < 1.0) {
            throw args::ValidationError("--tkr-time-step must be greater than or equal 1.0, got `" +
                                        std::to_string(tokoro.time_step) + "`");
        }
    }

    if (tokoro.vrs_mode == TokoroConfig::VrsMode::Fixed) {
        if (!gFixedItrfX || !gFixedItrfY || !gFixedItrfZ) {
            throw args::RequiredError(
                "--tkr-fixed-itrf-x, --tkr-fixed-itrf-y, and --tkr-fixed-itrf-z "
                "are required for fixed VRS mode");
        }

        tokoro.fixed_itrf_x = gFixedItrfX.Get();
        tokoro.fixed_itrf_y = gFixedItrfY.Get();
        tokoro.fixed_itrf_z = gFixedItrfZ.Get();

        tokoro.fixed_rtcm_x = gFixedRtcmX ? gFixedRtcmX.Get() : tokoro.fixed_itrf_x;
        tokoro.fixed_rtcm_y = gFixedRtcmY ? gFixedRtcmY.Get() : tokoro.fixed_itrf_y;
        tokoro.fixed_rtcm_z = gFixedRtcmZ ? gFixedRtcmZ.Get() : tokoro.fixed_itrf_z;
    }

    if (gShapiroCorrection) tokoro.shapiro_correction = false;
    if (gPhaseWindupCorrection) tokoro.phase_windup_correction = false;
    if (gEarthSolidTidesCorrection) tokoro.earth_solid_tides_correction = false;
    if (gAntennaPhaseVariationCorrection) tokoro.antenna_phase_variation_correction = false;
    if (gTroposphericHeightCorrection) tokoro.tropospheric_height_correction = false;
    if (gIodConsistencyCheck) tokoro.iod_consistency_check = false;
    if (gRtOC) tokoro.rtoc = true;
    if (gOcit) tokoro.ocit = true;
    if (gNegativePhaseWindup) tokoro.negative_phase_windup = true;
}

static void dump(TokoroConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("gps:     %s", config.generate_gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.generate_glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.generate_galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.generate_beidou ? "enabled" : "disabled");

    DEBUGF("VRS mode: %s", [&]() {
        switch (config.vrs_mode) {
        case TokoroConfig::VrsMode::Dynamic: return "dynamic";
        case TokoroConfig::VrsMode::Fixed: return "fixed";
        }
    }());

    DEBUGF("generation strategy: %s", [&]() {
        switch (config.generation_strategy) {
        case TokoroConfig::GenerationStrategy::AssistanceData: return "assistance data";
        case TokoroConfig::GenerationStrategy::TimeStep: return "time step";
        case TokoroConfig::GenerationStrategy::TimeStepAligned: return "time step aligned";
        }
    }());

    DEBUGF("dynamic distance threshold: %f km", config.dynamic_distance_threshold);

    if (config.vrs_mode == TokoroConfig::VrsMode::Fixed) {
        DEBUGF("fixed ITRF: (%f, %f, %f)", config.fixed_itrf_x, config.fixed_itrf_y,
              config.fixed_itrf_z);
        DEBUGF("fixed RTCM: (%f, %f, %f)", config.fixed_rtcm_x, config.fixed_rtcm_y,
              config.fixed_rtcm_z);
    } else {
        DEBUGF("time step: %f seconds", config.time_step);
    }

    DEBUGF("Shapiro:                 %s", config.shapiro_correction ? "true" : "false");
    DEBUGF("phase windup:            %s", config.phase_windup_correction ? "true" : "false");
    DEBUGF("earth solid tides:       %s", config.earth_solid_tides_correction ? "true" : "false");
    DEBUGF("antenna phase variation: %s",
          config.antenna_phase_variation_correction ? "true" : "false");

    DEBUGF("tropospheric height:     %s", config.tropospheric_height_correction ? "true" : "false");
    DEBUGF("iod consistency check:   %s", config.iod_consistency_check ? "true" : "false");
    DEBUGF("rtoc:                    %s", config.rtoc ? "true" : "false");
    DEBUGF("ocit:                    %s", config.ocit ? "true" : "false");
    DEBUGF("negative phase windup:   %s", config.negative_phase_windup ? "true" : "false");
}

}  // namespace tokoro
