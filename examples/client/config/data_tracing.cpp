
namespace data_tracing {

static args::Group                  gGroup{"Data Tracing:"};
static args::ValueFlag<std::string> gDevice{
    gGroup, "device", "Device", {"dt-device"}, args::Options::Single,
};

static args::ValueFlag<std::string> gServer{
    gGroup, "server", "Server", {"dt-server"}, args::Options::Single,
};

static args::ValueFlag<int> gPort{
    gGroup, "port", "Port", {"dt-port"}, args::Options::Single,
};

static args::ValueFlag<std::string> gUsername{
    gGroup, "username", "Username", {"dt-username"}, args::Options::Single,
};

static args::ValueFlag<std::string> gPassword{
    gGroup, "password", "Password", {"dt-password"}, args::Options::Single,
};

static void setup() {
    gPort.HelpDefault("1883");
}

static void parse(Config* config) {
    auto& dt   = config->data_tracing;
    dt.enabled = false;
    dt.port    = 1883;

    if (gDevice || gServer || gPort || gUsername || gPassword) {
        if (!gDevice || !gServer || !gUsername || !gPassword) {
            throw args::RequiredError("--dt-device, --dt-server, --dt-username, and --dt-password "
                                      "must be specified together");
        }

        dt.enabled  = true;
        dt.device   = gDevice.Get();
        dt.server   = gServer.Get();
        dt.username = gUsername.Get();
        dt.password = gPassword.Get();

        if (gPort) {
            dt.port = static_cast<uint16_t>(gPort.Get());
        }
    }
}

static void dump(DataTracingConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("device: \"%s\"", config.device.c_str());
    DEBUGF("server: \"%s\"", config.server.c_str());
    DEBUGF("port: %d", config.port);
    DEBUGF("username: \"%s\"", config.username.c_str());
    DEBUGF("password: \"%s\"", config.password.c_str());
}

}  // namespace data_tracing
