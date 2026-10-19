#include "options.hpp"
#include <cxx11_compat.hpp>
#include <version.hpp>

#include <client-io/config.hpp>
#include <client-io/parse.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

#include <cstdlib>
#include <iostream>

static args::Group gCaster{
    "Caster:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> gAddress{
    gCaster, "address", "Listen address (default: 0.0.0.0)", {"address"}, args::Options::Single};
static args::ValueFlag<int> gPort{
    gCaster, "port", "Listen port (default: 2101)", {"port"}, args::Options::Single};
static args::ValueFlag<std::string> gUsername{gCaster,
                                              "username",
                                              "Username required to access the mountpoint",
                                              {"username"},
                                              args::Options::Single};
static args::ValueFlag<std::string> gPassword{gCaster,
                                              "password",
                                              "Password required to access the mountpoint",
                                              {"password"},
                                              args::Options::Single};

static args::Group gMountpoint{
    "Mountpoint:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> gMountpointName{gMountpoint,
                                                    "mountpoint",
                                                    "Mountpoint name (default: STREAM)",
                                                    {"mountpoint"},
                                                    args::Options::Single};
static args::ValueFlag<std::string> gIdentifier{gMountpoint,
                                                "identifier",
                                                "Sourcetable identifier field",
                                                {"identifier"},
                                                args::Options::Single};
static args::ValueFlag<std::string> gFormat{gMountpoint,
                                            "format",
                                            "Sourcetable format field (default: \"RTCM 3.3\")",
                                            {"format"},
                                            args::Options::Single};
static args::ValueFlag<std::string> gFormatDetails{gMountpoint,
                                                   "format-details",
                                                   "Sourcetable format-details field",
                                                   {"format-details"},
                                                   args::Options::Single};
static args::ValueFlag<std::string> gCarrier{gMountpoint,
                                             "carrier",
                                             "Sourcetable carrier field (default: 2)",
                                             {"carrier"},
                                             args::Options::Single};
static args::ValueFlag<std::string> gNavSystem{gMountpoint,
                                               "nav-system",
                                               "Sourcetable nav-system field",
                                               {"nav-system"},
                                               args::Options::Single};
static args::ValueFlag<std::string> gNetwork{
    gMountpoint, "network", "Sourcetable network field", {"network"}, args::Options::Single};
static args::ValueFlag<std::string> gCountry{gMountpoint,
                                             "country",
                                             "Sourcetable country field (default: SWE)",
                                             {"country"},
                                             args::Options::Single};
static args::ValueFlag<double>      gLatitude{
    gMountpoint, "latitude", "Sourcetable latitude field", {"latitude"}, args::Options::Single};
static args::ValueFlag<double> gLongitude{
    gMountpoint, "longitude", "Sourcetable longitude field", {"longitude"}, args::Options::Single};
static args::Flag gNmea{gMountpoint, "nmea", "Mountpoint requires NMEA GGA from client", {"nmea"}};
static args::Flag gSolution{
    gMountpoint, "solution", "Sourcetable solution field is a network RTK solution", {"solution"}};
static args::ValueFlag<std::string> gGenerator{
    gMountpoint, "generator", "Sourcetable generator field", {"generator"}, args::Options::Single};
static args::ValueFlag<std::string> gBitrate{gMountpoint,
                                             "bitrate",
                                             "Sourcetable bitrate field (default: 0)",
                                             {"bitrate"},
                                             args::Options::Single};
static args::ValueFlag<std::string> gMisc{
    gMountpoint, "misc", "Sourcetable misc field", {"misc"}, args::Options::Single};

static CasterOptions parse_caster() {
    CasterOptions opts;
    if (gAddress) opts.address = gAddress.Get();
    if (gPort) opts.port = static_cast<uint16_t>(gPort.Get());
    if (gUsername) opts.username = gUsername.Get();
    if (gPassword) opts.password = gPassword.Get();

    if ((gUsername && !gPassword) || (gPassword && !gUsername)) {
        std::cerr << "Error: --username and --password must be specified together\n";
        exit(1);
    }

    auto& mp = opts.mountpoint;
    if (gMountpointName) mp.mountpoint = gMountpointName.Get();
    if (gIdentifier) mp.identifier = gIdentifier.Get();
    if (gFormat) mp.format = gFormat.Get();
    if (gFormatDetails) mp.format_details = gFormatDetails.Get();
    if (gCarrier) mp.carrier = gCarrier.Get();
    if (gNavSystem) mp.nav_system = gNavSystem.Get();
    if (gNetwork) mp.network = gNetwork.Get();
    if (gCountry) mp.country = gCountry.Get();
    if (gLatitude) mp.latitude = gLatitude.Get();
    if (gLongitude) mp.longitude = gLongitude.Get();
    mp.nmea     = static_cast<bool>(gNmea);
    mp.solution = static_cast<bool>(gSolution);
    if (gGenerator) mp.generator = gGenerator.Get();
    if (gBitrate) mp.bitrate = gBitrate.Get();
    if (gMisc) mp.misc = gMisc.Get();

    return opts;
}

CasterConfig parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser{"example-ntrip-caster (" CLIENT_VERSION
                                ") - a basic NTRIP 1.0 caster with a single mountpoint"};
    args::HelpFlag       help{parser, "help", "Display this help menu", {'?', "help"}};

    args::GlobalOptions caster_globals{parser, gCaster};
    args::GlobalOptions mountpoint_globals{parser, gMountpoint};
    input::setup(parser);

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

    CasterConfig config;
    config.caster = parse_caster();
    input::parse(config.inputs);

    if (config.inputs.inputs.empty()) {
        std::cerr << "Error: at least one --input must be specified\n";
        exit(1);
    }

    return config;
}
