#include <args.hpp>
#include <lpp/lpp.h>

#include "agnss_example.h"
#include "example.h"
#include "osr_example.h"
#include "ssr_example.h"

#define VERSION "v3.2.0 (public)"

args::Group arguments{"Arguments:"};

//
// Location Server
//
args::Group location_server{
    "Location Server:",
    args::Group::Validators::All,
    args::Options::Global,
};

args::ValueFlag<std::string> host{
    location_server, "host", "Host", {'h', "host"}, args::Options::Single | args::Options::Required,
};
args::ValueFlag<int> port{
    location_server, "port", "Port", {'p', "port"}, args::Options::Single | args::Options::Required,
};
args::Flag ssl{
    location_server, "ssl", "TLS", {'s', "ssl"}, args::Options::Single,
};

//
// Identity
//
args::Group identity{
    "Identity:",
    args::Group::Validators::All,
    args::Options::Global,
};

args::ValueFlag<unsigned long> msisdn{
    identity, "msisdn", "MSISDN", {"msisdn"}, args::Options::Single,
};

args::ValueFlag<unsigned long> imsi{
    identity, "imsi", "IMSI", {"imsi"}, args::Options::Single,
};

args::ValueFlag<std::string> ipv4{
    identity, "ipv4", "IPv4", {"ipv4"}, args::Options::Single,
};

//
// Cell Information
//
args::Group cell_information{
    "Cell Information:",
    args::Group::Validators::All,
    args::Options::Global,
};

args::ValueFlag<int> mcc{
    cell_information,
    "mcc",
    "Mobile Country Code",
    {'c', "mcc"},
    args::Options::Single | args::Options::Required,
};

args::ValueFlag<int> mnc{
    cell_information,
    "mnc",
    "Mobile Network Code",
    {'n', "mnc"},
    args::Options::Single | args::Options::Required,
};

args::ValueFlag<int> tac{
    cell_information,
    "tac",
    "Tracking Area Code",
    {'t', "lac", "tac"},
    args::Options::Single | args::Options::Required,
};

args::ValueFlag<int> ci{
    cell_information,
    "ci",
    "Cell Identity",
    {'i', "ci"},
    args::Options::Single | args::Options::Required,
};

//
// Modem
//

args::Group modem{
    "Modem:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::ValueFlag<std::string> modem_device{
    modem, "device", "Device", {"modem"}, args::Options::Single,
};

args::ValueFlag<int> modem_baud_rate{
    modem, "baud_rate", "Baud Rate", {"modem-baud"}, args::Options::Single,
};

//
// Output
//

args::Group output{
    "Output:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::Group file_output{
    output,
    "File:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

args::ValueFlag<std::string> file_path{
    file_output, "file_path", "Path", {"file"}, args::Options::Single,
};

args::Group serial_output{
    output,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

args::ValueFlag<std::string> serial_device{
    serial_output, "device", "Device", {"serial"}, args::Options::Single,
};

args::ValueFlag<int> serial_baud_rate{
    serial_output, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single,
};

args::Group i2c_output{
    output,
    "I2C:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

args::ValueFlag<std::string> i2c_device{
    i2c_output, "device", "Device", {"i2c"}, args::Options::Single,
};

args::ValueFlag<uint8_t> i2c_address{
    i2c_output, "address", "Address", {"i2c-address"}, args::Options::Single,
};

args::Group tcp_output{
    output,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

args::ValueFlag<std::string> tcp_ip_address{
    tcp_output, "ip_address", "IP Address", {"tcp"}, args::Options::Single,
};

args::ValueFlag<int> tcp_port{
    tcp_output, "port", "Port", {"tcp-port"}, args::Options::Single,
};

args::Group udp_output{
    output,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

args::ValueFlag<std::string> udp_ip_address{
    udp_output, "ip_address", "IP Address", {"udp"}, args::Options::Single,
};

args::ValueFlag<int> udp_port{
    udp_output, "port", "Port", {"udp-port"}, args::Options::Single,
};

args::Group stdout_output{
    output,
    "Stdout:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

args::Flag stdout_output_flag{
    stdout_output, "stdout", "Stdout", {"stdout"}, args::Options::Single,
};

//
//
//

LocationServerOptions parse_location_server_options() {
    LocationServerOptions location_server{
        .host = host.Get(),
        .port = port.Get(),
        .ssl  = false,
    };

    if (ssl) {
        location_server.ssl = ssl.Get();
    }

    return location_server;
}

IdentityOptions parse_identity_options() {
    IdentityOptions identity{};

    if (msisdn) {
        identity.msisdn = std::unique_ptr<unsigned long>{new unsigned long{msisdn.Get()}};
    }

    if (imsi) {
        identity.imsi = std::unique_ptr<unsigned long>{new unsigned long{imsi.Get()}};
    }

    if (ipv4) {
        identity.ipv4 = std::unique_ptr<std::string>{new std::string{ipv4.Get()}};
    }

    if (!identity.msisdn && !identity.imsi && !identity.ipv4) {
        identity.imsi = std::unique_ptr<unsigned long>{new unsigned long{2460813579lu}};
    }

    return identity;
}

CellOptions parse_cell_options() {
    CellOptions cell_information{
        .mcc = mcc.Get(),
        .mnc = mnc.Get(),
        .tac = tac.Get(),
        .cid = ci.Get(),
    };

    return cell_information;
}

ModemOptions parse_modem_options() {
    ModemOptions modem{};

    if (modem_device || modem_baud_rate) {
        if (!modem_device) {
            throw args::RequiredError("modem_device");
        }

        if (!modem_baud_rate) {
            throw args::RequiredError("modem_device");
        }

        modem.device = std::unique_ptr<ModemDevice>{new ModemDevice{
            .device    = modem_device.Get(),
            .baud_rate = modem_baud_rate.Get(),
        }};
    }

    return modem;
}

OutputOptions parse_output_options() {
    OutputOptions output{};

    if (file_path) {
        auto interface = interface::Interface::file(file_path.Get(), true);
        output.interfaces.emplace_back(interface);
    }

    if (serial_device || serial_baud_rate) {
        if (!serial_device) {
            throw args::RequiredError("serial_device");
        }

        uint32_t baud_rate = 115200;
        if (serial_baud_rate) {
            if (serial_baud_rate.Get() < 0) {
                throw args::ValidationError("serial_baud_rate must be positive");
            }

            baud_rate = static_cast<uint32_t>(serial_baud_rate.Get());
        }

        auto interface =
            interface::Interface::serial(serial_device.Get(), baud_rate, interface::DataBits::EIGHT,
                                         interface::StopBits::ONE, interface::ParityBit::NONE);
        output.interfaces.emplace_back(interface);
    }

    if (i2c_device || i2c_address) {
        if (!i2c_device) {
            throw args::RequiredError("i2c_device");
        }

        if (!i2c_address) {
            throw args::RequiredError("i2c_address");
        }

        auto interface = interface::Interface::i2c(i2c_device.Get(), i2c_address.Get());
        output.interfaces.emplace_back(interface);
    }

    if (tcp_ip_address || tcp_port) {
        if (!tcp_ip_address) {
            throw args::RequiredError("tcp_ip_address");
        }

        if (!tcp_port) {
            throw args::RequiredError("tcp_port");
        }

        auto interface =
            interface::Interface::tcp(tcp_ip_address.Get(), tcp_port.Get(), true /* reconnect */);
        output.interfaces.emplace_back(interface);
    }

    if (udp_ip_address || udp_port) {
        if (!udp_ip_address) {
            throw args::RequiredError("udp_ip_address");
        }

        if (!udp_port) {
            throw args::RequiredError("udp_port");
        }

        auto interface =
            interface::Interface::udp(udp_ip_address.Get(), udp_port.Get(), true /* reconnect */);
        output.interfaces.emplace_back(interface);
    }

    if (stdout_output_flag) {
        auto interface = interface::Interface::stdout();
        output.interfaces.emplace_back(interface);
    }

    for (auto& interface : output.interfaces) {
        interface->open();
    }

    return output;
}

void osr_callback(args::Subparser& parser) {
    args::ValueFlag<std::string> format_arg{
        parser, "format", "Format", {'f', "format"}, args::Options::Single,
    };

    format_arg.HelpDefault("rtcm");
    format_arg.HelpChoices({"rtcm", "xer"});

    args::ValueFlag<std::string> msm_type_arg{
        parser, "msm_type", "RTCM MSM type", {'y', "msm_type"}, args::Options::Single,
    };
    msm_type_arg.HelpDefault("any");
    msm_type_arg.HelpChoices({"any", "4", "5", "6", "7"});

    parser.Parse();

    auto location_server = parse_location_server_options();
    auto identity        = parse_identity_options();
    auto cell            = parse_cell_options();
    auto modem           = parse_modem_options();
    auto output          = parse_output_options();

    auto format   = osr_example::Format::RTCM;
    auto msm_type = osr_example::MsmType::ANY;

    if (format_arg) {
        if (format_arg.Get() == "rtcm") {
            format = osr_example::Format::RTCM;
        } else if (format_arg.Get() == "xer") {
            format = osr_example::Format::XER;
        } else {
            throw args::ValidationError("Invalid format");
        }
    }

    if (msm_type_arg) {
        if (msm_type_arg.Get() == "any") {
            msm_type = osr_example::MsmType::ANY;
        } else if (msm_type_arg.Get() == "4") {
            msm_type = osr_example::MsmType::MSM4;
        } else if (msm_type_arg.Get() == "5") {
            msm_type = osr_example::MsmType::MSM5;
        } else if (msm_type_arg.Get() == "6") {
            msm_type = osr_example::MsmType::MSM6;
        } else if (msm_type_arg.Get() == "7") {
            msm_type = osr_example::MsmType::MSM7;
        } else {
            throw args::ValidationError("Invalid MSM type");
        }
    }

    osr_example::execute(location_server, identity, cell, modem, output, format, msm_type);
}

void ssr_callback(args::Subparser& parser) {
    args::ValueFlag<std::string> format_arg{
        parser, "format", "Format", {'f', "format"}, args::Options::Single,
    };

    format_arg.HelpDefault("xer");
    format_arg.HelpChoices({"xer"});

    args::ValueFlag<int> ura_override_arg{
        parser,
        "ura",
        "A hacky fix to set a nominal value for the URA if the LPP message does not include it"
        ", value will be clamped between 0-7.",
        {"ura-override"},
        args::Options::Single,
    };

    ura_override_arg.HelpDefault("0");

    args::Flag ublox_clock_correction_arg{parser,
                                          "ublox-clock-correction",
                                          "Change the sign of the clock correction",
                                          {"ublox-clock-correction"}};

    args::Flag force_continuity_arg{parser,
                                    "force-continuity",
                                    "Force SF022 (IODE Continuity) to be 320 secs",
                                    {"force-continuity"}};

    parser.Parse();

    auto location_server = parse_location_server_options();
    auto identity        = parse_identity_options();
    auto cell            = parse_cell_options();
    auto modem           = parse_modem_options();
    auto output          = parse_output_options();

    auto format = ssr_example::Format::XER;
    if (format_arg) {
        if (format_arg.Get() == "xer") {
            format = ssr_example::Format::XER;
        } else {
            throw args::ValidationError("Invalid format");
        }
    }

    auto ura_override = 0;
    if (ura_override_arg) {
        ura_override = ura_override_arg.Get();
    }

    auto ublox_clock_correction = false;
    if (ublox_clock_correction_arg) {
        ublox_clock_correction = ublox_clock_correction_arg.Get();
    }

    auto force_continuity = false;
    if (force_continuity_arg) {
        force_continuity = force_continuity_arg.Get();
    }

    ssr_example::execute(location_server, identity, cell, modem, output, format, ura_override,
                         ublox_clock_correction, force_continuity);
}

void agnss_callback(args::Subparser& parser) {
    args::ValueFlag<std::string> format_arg{
        parser, "format", "Format", {'f', "format"}, args::Options::Single,
    };

    format_arg.HelpDefault("xer");
    format_arg.HelpChoices({"xer"});

    parser.Parse();

    auto location_server = parse_location_server_options();
    auto identity        = parse_identity_options();
    auto cell            = parse_cell_options();
    auto modem           = parse_modem_options();
    auto output          = parse_output_options();

    auto format = agnss_example::Format::XER;
    if (format_arg) {
        if (format_arg.Get() == "xer") {
            format = agnss_example::Format::XER;
        } else {
            throw args::ValidationError("Invalid format");
        }
    }

    agnss_example::execute(location_server, identity, cell, modem, output, format);
}

int main(int argc, char** argv) {
    // Initialize OpenSSL
    network_initialize();

    args::ArgumentParser parser(
        "SUPL-3GPP-LPP-client " VERSION "",
        "SUPL-3GPP-LPP-client is an example client for connecting requesting assistance data from "
        "a location server using 3GPP LPP (Release 16.4) over SUPL. The client includes a handful "
        "of libraries that can be used to build a more feature rich and fully functioning client, "
        "that for example communications with a GNSS receiver to estimate a precise position. "
        "The source code is available at https://github.com/Ericsson/SUPL-3GPP-LPP-client");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    port.HelpDefault("5431");
    ssl.HelpDefault("false");

    serial_baud_rate.HelpDefault("115200");

    //
    // Commands
    //
    args::Group commands{
        parser,
        "Commands:",
        args::Group::Validators::DontCare,
        args::Options::Global,
    };

    args::Command command_osr(
        commands, "osr",
        "Request Observation Space Representation (OSR) data from the location server.",
        &osr_callback);
    args::Command command_ssr(
        commands, "ssr", "Request State-space Representation (SSR) data from the location server.",
        &ssr_callback);
    args::Command command_agnss(
        commands, "agnss", "Request Assisted GNSS data from the location server.", &agnss_callback);

    args::GlobalOptions location_server_globals{parser, location_server};
    args::GlobalOptions identity_globals{parser, identity};
    args::GlobalOptions cell_information_globals{parser, cell_information};
    args::GlobalOptions modem_globals{parser, modem};
    args::GlobalOptions output_globals{parser, output};

    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        parser.Help(std::cerr);
        exit(1);
    } catch (const args::Help&) {
        std::cout << parser;
        exit(0);
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    }

    return EXIT_SUCCESS;
}
