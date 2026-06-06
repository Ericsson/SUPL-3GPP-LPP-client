#include "../config.hpp"

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace ntrip {

static args::Group gGroup{"NTRIP:", args::Group::Validators::AllChildGroups, args::Options::Global};

static args::ValueFlag<std::string> gHost{gGroup, "host", "NTRIP caster hostname", {"ntrip-host"}};
static args::ValueFlag<int>         gPort{
    gGroup, "port", "NTRIP caster port (default: 2101)", {"ntrip-port"}, 2101};
static args::ValueFlag<std::string> gMountpoint{
    gGroup, "mountpoint", "NTRIP mountpoint", {"ntrip-mountpoint"}};
static args::ValueFlag<std::string> gUser{gGroup, "user", "NTRIP username", {"ntrip-user"}};
static args::ValueFlag<std::string> gPassword{
    gGroup, "password", "NTRIP password", {"ntrip-password"}};
static args::Flag gTls{gGroup, "tls", "Use TLS", {"ntrip-tls"}};

static args::ValueFlag<std::string> gPosition{
    gGroup,
    "mode",
    "Position mode: none, fixed, internal (default: none)",
    {"ntrip-position"},
    "none"};
static args::ValueFlag<double> gLat{gGroup, "deg", "Fixed latitude (WGS84)", {"ntrip-lat"}, 0.0};
static args::ValueFlag<double> gLon{gGroup, "deg", "Fixed longitude (WGS84)", {"ntrip-lon"}, 0.0};
static args::ValueFlag<double> gAlt{gGroup, "m", "Fixed altitude (default: 0)", {"ntrip-alt"}, 0.0};

static args::ValueFlag<double> gRoundDeg{gGroup,
                                         "deg",
                                         "Round position to nearest N degrees (privacy)",
                                         {"ntrip-position-round-deg"},
                                         0.0};
static args::ValueFlag<double> gOffsetM{gGroup,
                                        "m",
                                        "Add random position offset up to N metres (privacy)",
                                        {"ntrip-position-offset-m"},
                                        0.0};
static args::ValueFlag<int>    gPosInterval{gGroup,
                                         "s",
                                         "Position re-send interval seconds (default: 10)",
                                            {"ntrip-position-interval"},
                                         10};
static args::ValueFlag<int>    gReconnect{
    gGroup, "s", "Reconnect interval seconds (default: 5)", {"ntrip-reconnect-interval"}, 5};
static args::ValueFlag<int> gTimeout{
    gGroup, "s", "Read timeout seconds (default: 30)", {"ntrip-timeout"}, 30};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

void parse(Config* config) {
    if (!gHost) return;

    auto& c   = config->ntrip;
    c.enabled = true;
    c.host    = gHost.Get();
    c.port    = static_cast<uint16_t>(gPort.Get());
    if (gMountpoint) c.mountpoint = gMountpoint.Get();
    if (gUser) c.username = gUser.Get();
    if (gPassword) c.password = gPassword.Get();
    c.tls = static_cast<bool>(gTls);

    auto mode = gPosition.Get();
    if (mode == "fixed")
        c.position_mode = NtripConfig::PositionMode::Fixed;
    else if (mode == "internal")
        c.position_mode = NtripConfig::PositionMode::Internal;
    else
        c.position_mode = NtripConfig::PositionMode::None;

    c.latitude  = gLat.Get();
    c.longitude = gLon.Get();
    c.altitude  = gAlt.Get();

    c.position_round_deg   = gRoundDeg.Get();
    c.position_offset_m    = gOffsetM.Get();
    c.position_interval_s  = gPosInterval.Get();
    c.reconnect_interval_s = gReconnect.Get();
    c.timeout_s            = gTimeout.Get();
}

}  // namespace ntrip
