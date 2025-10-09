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

static args::Flag gReliable{
    gGroup, "reliable", "Reliable", {"dt-reliable"}, args::Options::Single,
};

static args::Flag gDisableSsrData{
    gGroup, "disable-ssr-data", "Disable SSR Data", {"dt-disable-ssr-data"}, args::Options::Single,
};

static args::Flag gPossibLog{
    gGroup, "possib-log", "Enable posSIB logging", {"dt-possib-log"}, args::Options::Single,
};

static args::Flag gPossibWrap{
    gGroup, "possib-wrap", "Include wrapped posSIB data", {"dt-possib-wrap"}, args::Options::Single,
};

void setup() {
    gPort.HelpDefault("1883");
}

void parse(Config* config) {
    auto& dt            = config->data_tracing;
    dt.enabled          = false;
    dt.port             = 1883;
    dt.reliable         = gReliable;
    dt.disable_ssr_data = gDisableSsrData;
    dt.possib_log       = gPossibLog;
    dt.possib_wrap      = gPossibWrap;

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

void dump(DataTracingConfig const& config) {
    DEBUGF("status: %s", config.enabled ? "enabled" : "disabled");
    if (!config.enabled) return;

    DEBUGF("device: \"%s\"", config.device.c_str());
    DEBUGF("server: \"%s\"", config.server.c_str());
    DEBUGF("port: %d", config.port);
    DEBUGF("username: \"%s\"", config.username.c_str());
    DEBUGF("password: \"%s\"", config.password.c_str());
    DEBUGF("possib-log:  %s", config.possib_log ? "true" : "false");
    DEBUGF("possib-wrap: %s", config.possib_wrap ? "true" : "false");
}

}  // namespace data_tracing
