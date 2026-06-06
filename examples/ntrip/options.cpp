#include "options.hpp"
#include <version.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

static args::Group gNtrip{
    "NTRIP:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> gNtripHostname{
    gNtrip, "hostname", "Hostname", {"host"}, args::Options::Single};
static args::ValueFlag<int> gNtripPort{
    gNtrip, "port", "Port (default: 2101)", {"port"}, args::Options::Single};
static args::ValueFlag<std::string> gNtripMountpoint{
    gNtrip, "mountpoint", "Mountpoint", {"mountpoint"}, args::Options::Single};
static args::ValueFlag<std::string> gNtripUsername{
    gNtrip, "username", "Username", {"username"}, args::Options::Single};
static args::ValueFlag<std::string> gNtripPassword{
    gNtrip, "password", "Password", {"password"}, args::Options::Single};
static args::ValueFlag<std::string> gNmeaString{
    gNtrip, "nmea", "NMEA string to send", {"nmea"}, args::Options::Single};
static args::Flag gHexdump{gNtrip, "hexdump", "Hexdump received data", {"hexdump"}};

static HostOptions parse_host() {
    if (!gNtripHostname) throw args::RequiredError("--host");

    HostOptions h;
    h.hostname = gNtripHostname.Get();
    h.port     = gNtripPort ? static_cast<uint16_t>(gNtripPort.Get()) : 2101;
    if (gNtripMountpoint) h.mountpoint = std::make_unique<std::string>(gNtripMountpoint.Get());
    if (gNtripUsername) h.username = gNtripUsername.Get();
    if (gNtripPassword) h.password = gNtripPassword.Get();
    if (gNmeaString) h.nmea = gNmeaString.Get();
    h.hexdump = static_cast<bool>(gHexdump);
    return h;
}

Options parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser{"example-ntrip (" CLIENT_VERSION ")"};
    args::HelpFlag       help{parser, "help", "Display this help menu", {'?', "help"}};

    args::GlobalOptions ntrip_globals{parser, gNtrip};
    output::setup(parser);

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help const&) {
        std::cout << parser;
        exit(0);
    } catch (args::ParseError const& e) {
        std::cerr << e.what() << "\n" << parser;
        exit(1);
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << "\n";
        exit(1);
    }

    Options opts;
    opts.host = parse_host();
    output::parse(opts.outputs);
    return opts;
}
