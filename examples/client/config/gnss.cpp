
namespace gnss {

static args::Group gGroup{"GNSS:"};
static args::Flag  gGps{
    gGroup,
    "gps",
    "Disable GPS usage",
     {"no-gps"},
};
static args::Flag gGlonass{
    gGroup,
    "glonass",
    "Disable GLONASS usage",
    {"no-glonass", "no-glo"},
};
static args::Flag gGalileo{
    gGroup,
    "galileo",
    "Disable Galileo usage",
    {"no-galileo", "no-gal"},
};
static args::Flag gBds{
    gGroup,
    "bds",
    "Disable BeiDou usage",
    {"no-beidou", "no-bds"},
};

static void setup() {}

static void parse(Config* config) {
    auto& gnss = config->gnss;

    if (gGps) gnss.gps = false;
    if (gGlonass) gnss.glonass = false;
    if (gGalileo) gnss.galileo = false;
    if (gBds) gnss.beidou = false;
}

static void dump(GnssConfig const& config) {
    DEBUGF("gps:     %s", config.gps ? "enabled" : "disabled");
    DEBUGF("glonass: %s", config.glonass ? "enabled" : "disabled");
    DEBUGF("galileo: %s", config.galileo ? "enabled" : "disabled");
    DEBUGF("beidou:  %s", config.beidou ? "enabled" : "disabled");
}

}  // namespace gnss
