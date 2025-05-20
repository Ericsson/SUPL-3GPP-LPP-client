
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
static args::ValueFlag<std::string> gInterface{
    gGroup,           "interface",           "Interface to use for location server connection",
    {"ls-interface"}, args::Options::Single,
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
static args::Flag gShutdownOnDisconnect{
    gGroup,
    "shutdown-on-disconnect",
    "Shutdown the client if the location server connection is lost",
    {"ls-shutdown-on-disconnect"},
};
static args::Flag gHackBadTransactionInitiator{
    gGroup,
    "hack-bad-transaction-initiator",
    "Hack to allow the transaction initiator in ProvideAssistanceData to be LocationServer",
    {"ls-hack-bad-transaction-initiator"},
};

static void setup() {}

static void parse(Config* config) {
    auto& ls                          = config->location_server;
    ls.enabled                        = true;
    ls.shutdown_on_disconnect         = false;
    ls.interface                      = nullptr;
    ls.hack_bad_transaction_initiator = gHackBadTransactionInitiator.Get();

    if (gDisable) {
        ls.enabled = false;
        return;
    }

    if (gInterface) {
        ls.interface = std::unique_ptr<std::string>(new std::string(gInterface.Get()));
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

    if (gShutdownOnDisconnect) ls.shutdown_on_disconnect = true;
}

static void dump(LocationServerConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("host: \"%s\"", config.host.c_str());
    DEBUGF("port: %d", config.port);
    if (config.interface) {
        DEBUGF("interface: \"%s\"", config.interface.get()->c_str());
    }
    DEBUGF("slp-host-cell: %s", config.slp_host_cell ? "true" : "false");
    DEBUGF("slp-host-imsi: %s", config.slp_host_imsi ? "true" : "false");
    DEBUGF("shutdown-on-disconnect:         %s", config.shutdown_on_disconnect ? "true" : "false");
    DEBUGF("hack-bad-transaction-initiator: %s",
           config.hack_bad_transaction_initiator ? "true" : "false");
}

}  // namespace ls
