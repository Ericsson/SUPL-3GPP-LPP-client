
namespace ls {

static args::Group                  gGroup{"Location Server:"};
static args::ValueFlag<std::string> gHost{
    gGroup,
    "host",
    "Location server hostname or IP address",
    {'h', "ls-host"},
    args::Options::Single,
};
static args::ValueFlag<uint16_t> gPort{
    gGroup, "port", "Location server port", {'p', "ls-port"}, args::Options::Single,
};
static args::Flag gDisable{
    gGroup,
    "disable",
    "Run without connecting to the location server",
    {"ls-disable"},
};
static args::Flag gSlpHostCell{
    gGroup,
    "slp-host-cell",
    "Use host cell to resolve location server IP address",
    {"slp-host-cell"},
};
static args::Flag gSlpHostImsi{
    gGroup,
    "slp-host-imsi",
    "Use IMSI to resolve location server IP address",
    {"slp-host-imsi"},
};

static void setup() {}

static void parse(Config* config) {
    auto& ls   = config->location_server;
    ls.enabled = true;

    if (gDisable) {
        ls.enabled = false;
        return;
    }

    if (gSlpHostCell) {
        ls.slp_host_cell = true;

        if (gHost) throw args::RequiredError("`--ls-host` cannot be used with `--slp-host-cell`");
        if (gPort) throw args::RequiredError("`--ls-port` cannot be used with `--slp-host-cell`");
    } else if (gSlpHostImsi) {
        ls.slp_host_imsi = true;

        if (gHost) throw args::RequiredError("`--ls-host` cannot be used with `--slp-host-imsi`");
        if (gPort) throw args::RequiredError("`--ls-port` cannot be used with `--slp-host-imsi`");
    } else {
        if (!gHost)
            throw args::RequiredError(
                "`--ls-host` is required, if you want to run without connecting "
                "to the location server use `--ls-disable`");
        if (!gPort) throw args::RequiredError("`--ls-port` is required with `--ls-host`");

        ls.host = gHost.Get();
        ls.port = gPort.Get();
    }
}

static void dump(LocationServerConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("host: \"%s\"", config.host.c_str());
    DEBUGF("port: %d", config.port);
    DEBUGF("slp-host-cell: %s", config.slp_host_cell ? "true" : "false");
    DEBUGF("slp-host-imsi: %s", config.slp_host_imsi ? "true" : "false");
}

}  // namespace ls
