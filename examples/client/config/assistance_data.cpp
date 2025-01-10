
namespace ad {

static args::Group gGroup{"Assistance Data:"};
static args::Flag  gDisable{
    gGroup,
    "disable",
    "Don't request assistance data",
     {"ad-disable"},
};

#if 0
// TODO(ewasjon): Add support for requesting assisted GNSS data first
static args::Flag gAssistedGnss{
    gGroup,
    "assisted-gnss",
    "Request assisted GNSS data first",
    {"ad-assisted-gnss"},
};
#endif

static args::ValueFlag<std::string> gType{
    gGroup, "type", "Type of assistance data to request", {"ad-type"}, args::Options::Single,
};

static args::Group          gCellInformation{gGroup, "Cell Information:"};
static args::ValueFlag<int> gMcc{
    gCellInformation, "mcc", "Mobile Country Code", {'c', "mcc"}, args::Options::Single,
};
static args::ValueFlag<int> gMnc{
    gCellInformation, "mnc", "Mobile Network Code", {'n', "mnc"}, args::Options::Single,
};
static args::ValueFlag<int> gTac{
    gCellInformation, "tac", "Tracking Area Code", {'t', "lac", "tac"}, args::Options::Single,
};
static args::ValueFlag<unsigned long long> gCi{
    gCellInformation, "ci", "Cell Identity", {'i', "ci"}, args::Options::Single,
};
static args::Flag gIsNr{
    gCellInformation,
    "nr",
    "The cell specified is a 5G NR cell",
    {"nr-cell"},
};
static args::Flag gWaitForCell{
    gCellInformation,
    "wait-for-cell",
    "Wait for the cell information to be provided via the control interface",
    {"wait-for-cell"},
};

static args::Group gGnss{gGroup, "GNSS:"};
static args::Flag  gGps{
    gGnss,
    "gps",
    "Do not request GPS assistance data",
     {"ad-no-gps"},
};
static args::Flag gGlonass{
    gGnss,
    "glo",
    "Do not request GLONASS assistance data",
    {"ad-no-glo"},
};
static args::Flag gGalileo{
    gGnss,
    "gal",
    "Do not request Galileo assistance data",
    {"ad-no-gal"},
};
static args::Flag gBds{
    gGnss,
    "bds",
    "Do not request BDS assistance data",
    {"ad-no-bds"},
};

static args::Group           gOsrs{gGroup, "OSR:"};
static args::ValueFlag<long> gOsrObservations{
    gOsrs, "rate", "OSR Observations", {"ad-osr-observations"}, args::Options::Single,
};
static args::ValueFlag<long> gOsrResiduals{
    gOsrs, "rate", "OSR Residuals", {"ad-osr-residuals"}, args::Options::Single,
};
static args::ValueFlag<long> gOsrBiasInformation{
    gOsrs, "rate", "OSR Bias Information", {"ad-osr-bias-information"}, args::Options::Single,
};

static args::Group           gSsrs{gGroup, "SSR:"};
static args::ValueFlag<long> gSsrClock{
    gSsrs, "rate", "SSR Clock", {"ad-ssr-clock"}, args::Options::Single,
};
static args::ValueFlag<long> gSsrOrbit{
    gSsrs, "rate", "SSR Orbit", {"ad-ssr-orbit"}, args::Options::Single,
};
static args::ValueFlag<long> gSsrUra{
    gSsrs, "rate", "SSR URA", {"ad-ssr-ura"}, args::Options::Single,
};
static args::ValueFlag<long> gSsrCodeBias{
    gSsrs, "rate", "SSR Code Bias", {"ad-ssr-code-bias"}, args::Options::Single,
};
static args::ValueFlag<long> gSsrPhaseBias{
    gSsrs, "rate", "SSR Phase Bias", {"ad-ssr-phase-bias"}, args::Options::Single,
};
static args::ValueFlag<long> gSsrStec{
    gSsrs, "rate", "SSR STEC", {"ad-ssr-stec"}, args::Options::Single,
};
static args::ValueFlag<long> gSsrGridded{
    gSsrs, "rate", "SSR Gridded", {"ad-ssr-gridded"}, args::Options::Single,
};

static void setup() {
    gType.HelpChoices({"osr", "ssr"});

    gOsrObservations.HelpDefault("1");
    gOsrResiduals.HelpDefault("1");
    gOsrBiasInformation.HelpDefault("1");

    gSsrClock.HelpDefault("5");
    gSsrOrbit.HelpDefault("5");
    gSsrUra.HelpDefault("5");
    gSsrCodeBias.HelpDefault("5");
    gSsrPhaseBias.HelpDefault("5");
    gSsrStec.HelpDefault("30");
    gSsrGridded.HelpDefault("30");
}

static void parse(Config* config) {
    auto& ad         = config->assistance_data;
    ad.enabled       = true;
    ad.wait_for_cell = false;

    ad.gps     = true;
    ad.glonass = true;
    ad.galileo = true;
    ad.beidou  = true;

    ad.rtk_observations           = 1;
    ad.rtk_residuals              = 1;
    ad.rtk_bias_information       = 1;
    ad.rtk_reference_station_info = 1;

    ad.ssr_clock             = 5;
    ad.ssr_orbit             = 5;
    ad.ssr_code_bias         = 5;
    ad.ssr_phase_bias        = 5;
    ad.ssr_stec              = 30;
    ad.ssr_gridded           = 30;
    ad.ssr_ura               = 5;
    ad.ssr_correction_points = 1;

    if (gDisable) {
        ad.enabled = false;
        return;
    } else if (!config->location_server.enabled) {
        ad.enabled = false;
        VERBOSEF("assistance data can only be requested when connecting to the location server\n");
        return;
    }

    if (gWaitForCell) {
        ad.wait_for_cell = true;
    } else {
        if (!gMcc || !gMnc || !gTac || !gCi) {
            throw args::RequiredError(
                "cell information is required, use `--mcc`, `--mnc`, `--tac`, `--ci`");
        }

        if (gIsNr) {
            ad.cell = supl::Cell::nr(gMcc.Get(), gMnc.Get(), gTac.Get(), gCi.Get());
        } else {
            ad.cell = supl::Cell::lte(gMcc.Get(), gMnc.Get(), gTac.Get(), gCi.Get());
        }
    }

    if (gGps) ad.gps = false;
    if (gGlonass) ad.glonass = false;
    if (gGalileo) ad.galileo = false;
    if (gBds) ad.beidou = false;

    if (gType) {
        if (gType.Get() == "OSR" || gType.Get() == "osr") {
            ad.type = lpp::RequestAssistanceData::Type::OSR;
        } else if (gType.Get() == "SSR" || gType.Get() == "ssr") {
            ad.type = lpp::RequestAssistanceData::Type::SSR;
        } else {
            throw args::ParseError("invalid assistance data type: " + gType.Get());
        }
    } else {
        throw args::RequiredError("assistance data type is required, use `--ad-type`");
    }

    if (gOsrObservations) ad.rtk_observations = gOsrObservations.Get();
    if (gOsrResiduals) ad.rtk_residuals = gOsrResiduals.Get();
    if (gOsrBiasInformation) ad.rtk_bias_information = gOsrBiasInformation.Get();

    if (gSsrClock) ad.ssr_clock = gSsrClock.Get();
    if (gSsrOrbit) ad.ssr_orbit = gSsrOrbit.Get();
    if (gSsrCodeBias) ad.ssr_code_bias = gSsrCodeBias.Get();
    if (gSsrPhaseBias) ad.ssr_phase_bias = gSsrPhaseBias.Get();
    if (gSsrStec) ad.ssr_stec = gSsrStec.Get();
    if (gSsrGridded) ad.ssr_gridded = gSsrGridded.Get();
    if (gSsrUra) ad.ssr_ura = gSsrUra.Get();

    if (ad.rtk_observations <= 0)
        throw args::ParseError("invalid OSR observations: " + std::to_string(ad.rtk_observations));
    if (ad.rtk_residuals <= 0)
        throw args::ParseError("invalid OSR residuals: " + std::to_string(ad.rtk_residuals));
    if (ad.rtk_bias_information <= 0)
        throw args::ParseError("invalid OSR bias information: " +
                               std::to_string(ad.rtk_bias_information));

    if (ad.ssr_clock <= 0)
        throw args::ParseError("invalid SSR clock: " + std::to_string(ad.ssr_clock));
    if (ad.ssr_orbit <= 0)
        throw args::ParseError("invalid SSR orbit: " + std::to_string(ad.ssr_orbit));
    if (ad.ssr_code_bias <= 0)
        throw args::ParseError("invalid SSR code bias: " + std::to_string(ad.ssr_code_bias));
    if (ad.ssr_phase_bias <= 0)
        throw args::ParseError("invalid SSR phase bias: " + std::to_string(ad.ssr_phase_bias));
    if (ad.ssr_stec <= 0)
        throw args::ParseError("invalid SSR stec: " + std::to_string(ad.ssr_stec));
    if (ad.ssr_gridded <= 0)
        throw args::ParseError("invalid SSR gridded: " + std::to_string(ad.ssr_gridded));
    if (ad.ssr_ura <= 0) throw args::ParseError("invalid SSR ura: " + std::to_string(ad.ssr_ura));
}

static void dump(AssistanceDataConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("wait-for-cell: %s", config.wait_for_cell ? "true" : "false");

    if (!config.wait_for_cell) {
        if (config.cell.type == supl::Cell::Type::GSM) {
            auto const& cell = config.cell.data.gsm;
            DEBUGF("cell: GSM %" PRIi64 "-%" PRIi64 "-%" PRIi64 "-%" PRIu64, cell.mcc, cell.mnc,
                   cell.lac, cell.ci);
        } else if (config.cell.type == supl::Cell::Type::LTE) {
            auto const& cell = config.cell.data.lte;
            DEBUGF("cell: LTE %" PRIi64 "-%" PRIi64 "-%" PRIi64 "-%" PRIu64, cell.mcc, cell.mnc,
                   cell.tac, cell.ci);
        } else if (config.cell.type == supl::Cell::Type::NR) {
            auto const& cell = config.cell.data.nr;
            DEBUGF("cell: NR %" PRIi64 "-%" PRIi64 "-%" PRIi64 "-%" PRIu64, cell.mcc, cell.mnc,
                   cell.tac, cell.ci);
        } else {
            DEBUGF("cell: unknown");
        }
    }

    DEBUGF("gps: %s", config.gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou: %s", config.beidou ? "enabled" : "disabled");

    DEBUGF("type: %s", config.type == lpp::RequestAssistanceData::Type::OSR ? "OSR" : "SSR");

    DEBUGF("rtk_observations: %ld", config.rtk_observations);
    DEBUGF("rtk_residuals: %ld", config.rtk_residuals);
    DEBUGF("rtk_bias_information: %ld", config.rtk_bias_information);

    DEBUGF("ssr_clock: %ld", config.ssr_clock);
    DEBUGF("ssr_orbit: %ld", config.ssr_orbit);
    DEBUGF("ssr_code_bias: %ld", config.ssr_code_bias);
    DEBUGF("ssr_phase_bias: %ld", config.ssr_phase_bias);
    DEBUGF("ssr_stec: %ld", config.ssr_stec);
    DEBUGF("ssr_gridded: %ld", config.ssr_gridded);
    DEBUGF("ssr_ura: %ld", config.ssr_ura);
}

}  // namespace ad
