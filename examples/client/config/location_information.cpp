
namespace li {

static args::Group gGroup{"Location Information:"};
static args::Flag  gUnsolicited{
    gGroup,
    "li-unsolicited",
    "Send unsolicited provide location information messages to the location server",
     {"li-unsolicited"},
};
static args::ValueFlag<int> gUpdateRate{
    gGroup,
    "ms",
    "Update rate is determined by the location server. Setting this value "
    "will override the location server's update rate",
    {"li-update-rate"},
    args::Options::Single,
};

static args::Flag gNmeaLocation{
    gGroup,
    "nmea-location",
    "Do not use NMEA messages for location information",
    {"li-no-nmea"},
};
static args::Flag gUbxLocation{
    gGroup,
    "ubx-location",
    "Do not use UBX messages for location information",
    {"li-no-ubx"},
};

static args::Flag gConvertConfidence95to68{
    gGroup,
    "confidence-95-to-68",
    "Scale the semi-major/semi-minor axes from 95% to 68% confidence",
    {"li-confidence-95-to-68"},
};
static args::Flag gOutputEllipse68{
    gGroup,
    "error-ellipse-68",
    "Output error ellipse with 68% confidence instead of 39%",
    {"li-error-ellipse-68"},
};
static args::ValueFlag<double> gOverrideHorizontalConfidence{
    gGroup,
    "0.0-1.0",
    "Override horizontal confidence for the error ellipse",
    {"li-override-horizontal-confidence"},
    args::Options::Single,
};

static args::Group gFakeLocationGroup{gGroup, "Fake Location:"};
static args::Flag  gFakeLocation{
    gFakeLocationGroup,
    "fake-location",
    "Enable fake location information",
     {"li-fake-location"},
};
static args::ValueFlag<double> gLatitude{
    gFakeLocationGroup, "degree", "Fake Latitude", {"li-fake-latitude"}, args::Options::Single,
};
static args::ValueFlag<double> gLongitude{
    gFakeLocationGroup, "degree", "Fake Longitude", {"li-fake-longitude"}, args::Options::Single,
};
static args::ValueFlag<double> gAltitude{
    gFakeLocationGroup, "meter", "Fake Altitude", {"li-fake-altitude"}, args::Options::Single,
};

static void setup() {
    gUpdateRate.HelpDefault("1000");
    gLatitude.HelpDefault("69.06");
    gLongitude.HelpDefault("20.55");
    gAltitude.HelpDefault("0.0");
}

static void parse(Config* config) {
    auto& li                          = config->location_information;
    li.unsolicited                    = false;
    li.update_rate_forced             = false;
    li.update_rate_ms                 = 1000;
    li.use_nmea_location              = true;
    li.use_ubx_location               = true;
    li.convert_confidence_95_to_68    = false;
    li.output_ellipse_68              = false;
    li.override_horizontal_confidence = -1.0;
    li.fake.enabled                   = false;
    li.fake.latitude                  = 69.06;
    li.fake.longitude                 = 20.55;
    li.fake.altitude                  = 0.0;

    if (gUnsolicited) {
        li.unsolicited = true;
    }

    if (gUpdateRate) {
        li.update_rate_forced = true;
        li.update_rate_ms     = gUpdateRate.Get();
    }

    if (gNmeaLocation) li.use_nmea_location = false;
    if (gUbxLocation) li.use_ubx_location = false;
    if (gConvertConfidence95to68) li.convert_confidence_95_to_68 = true;
    if (gOutputEllipse68) li.output_ellipse_68 = true;

    if (gOverrideHorizontalConfidence) {
        li.override_horizontal_confidence = gOverrideHorizontalConfidence.Get();
        if (li.override_horizontal_confidence < 0.0 || li.override_horizontal_confidence > 1.0) {
            throw args::ValidationError(
                "override horizontal confidence must be between 0.0 and 1.0");
        }
    }

    if (gFakeLocation) {
        li.fake.enabled = true;
        if (gLatitude) li.fake.latitude = gLatitude.Get();
        if (gLongitude) li.fake.longitude = gLongitude.Get();
        if (gAltitude) li.fake.altitude = gAltitude.Get();
    }
}

static void dump(LocationInformationConfig const& config) {
    DEBUGF("unsolicited: %s", config.unsolicited ? "true" : "false");
    DEBUGF("update_rate_forced: %s", config.update_rate_forced ? "true" : "false");
    DEBUGF("update_rate_ms: %d ms", config.update_rate_ms);
    DEBUGF("use_nmea_location: %s", config.use_nmea_location ? "true" : "false");
    DEBUGF("use_ubx_location: %s", config.use_ubx_location ? "true" : "false");
    DEBUGF("convert_confidence_95_to_68: %s", config.convert_confidence_95_to_68 ? "true" : "false");
    DEBUGF("output_ellipse_68: %s", config.output_ellipse_68 ? "true" : "false");
    DEBUGF("override_horizontal_confidence: %.1f", config.override_horizontal_confidence);
    DEBUGF("fake.enabled: %s", config.fake.enabled ? "true" : "false");
    DEBUGF("fake.latitude: %.8f", config.fake.latitude);
    DEBUGF("fake.longitude: %.8f", config.fake.longitude);
    DEBUGF("fake.altitude: %.3f", config.fake.altitude);
}

}  // namespace li
