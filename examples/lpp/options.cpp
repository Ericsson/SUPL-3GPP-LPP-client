#include "options.hpp"
#include <interface/interface.hpp>
#include <receiver/ublox/receiver.hpp>

#include <math.h>

using namespace interface;
using namespace receiver::ublox;

static args::Group arguments{"Arguments:"};

//
// Location Server
//
static args::Group location_server{
    "Location Server:",
    args::Group::Validators::All,
    args::Options::Global,
};

static args::ValueFlag<std::string> location_server_host{
    location_server, "host", "Host", {'h', "host"}, args::Options::Single};
static args::ValueFlag<uint16_t> location_server_port{
    location_server, "port", "Port", {'p', "port"}, args::Options::Single};
static args::Flag location_server_ssl{
    location_server, "ssl", "TLS", {'s', "ssl"}, args::Options::Single};
static args::Flag location_server_slp_host_cell{location_server,
                                                "slp-host-cell",
                                                "Use Cell ID as SLP Host",
                                                {"slp-host-cell"},
                                                args::Options::Single};
static args::Flag location_server_slp_host_imsi{location_server,
                                                "slp-host-imsi",
                                                "Use IMSI as SLP Host",
                                                {"slp-host-imsi"},
                                                args::Options::Single};

//
// Identity
//
static args::Group identity{
    "Identity:",
    args::Group::Validators::All,
    args::Options::Global,
};

static args::ValueFlag<unsigned long long> msisdn{
    identity, "msisdn", "MSISDN", {"msisdn"}, args::Options::Single};
static args::ValueFlag<unsigned long long> imsi{
    identity, "imsi", "IMSI", {"imsi"}, args::Options::Single};
static args::ValueFlag<std::string> ipv4{identity, "ipv4", "IPv4", {"ipv4"}, args::Options::Single};
static args::Flag                   use_supl_identity_fix{identity,
                                        "supl-identity-fix",
                                        "Use SUPL Identity Fix",
                                                          {"supl-identity-fix"},
                                        args::Options::Single};
static args::Flag                   wait_for_identity{
    identity,
    "wait-for-identity",
    "Wait for the identity to be provided via the control interface",
                      {"wait-for-identity"},
    args::Options::Single};

//
// Cell Information
//
static args::Group cell_information{
    "Cell Information:",
    args::Group::Validators::All,
    args::Options::Global,
};

static args::ValueFlag<int>                mcc{cell_information,
                                "mcc",
                                "Mobile Country Code",
                                               {'c', "mcc"},
                                args::Options::Single | args::Options::Required};
static args::ValueFlag<int>                mnc{cell_information,
                                "mnc",
                                "Mobile Network Code",
                                               {'n', "mnc"},
                                args::Options::Single | args::Options::Required};
static args::ValueFlag<int>                tac{cell_information,
                                "tac",
                                "Tracking Area Code",
                                               {'t', "lac", "tac"},
                                args::Options::Single | args::Options::Required};
static args::ValueFlag<unsigned long long> ci{cell_information,
                                              "ci",
                                              "Cell Identity",
                                              {'i', "ci"},
                                              args::Options::Single | args::Options::Required};
static args::Flag is_nr{cell_information, "nr", "The cell specified is a 5G NR cell", {"nr"}};

//
// u-blox
//

static args::Group ublox_receiver_group{
    "u-blox Receiver:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> ublox_receiver_port{
    ublox_receiver_group,
    "port",
    "The port used on the u-blox receiver, used by configuration.",
    {"ublox-port"},
    args::Options::Single};

static args::Group ublox_serial_group{
    ublox_receiver_group,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ublox_serial_device{
    ublox_serial_group, "device", "Device", {"ublox-serial"}, args::Options::Single};
static args::ValueFlag<int> ublox_serial_baud_rate{
    ublox_serial_group, "baud_rate", "Baud Rate", {"ublox-serial-baud"}, args::Options::Single};
static args::ValueFlag<int> ublox_serial_data_bits{
    ublox_serial_group, "data_bits", "Data Bits", {"ublox-serial-data"}, args::Options::Single};
static args::ValueFlag<int> ublox_serial_stop_bits{
    ublox_serial_group, "stop_bits", "Stop Bits", {"ublox-serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> ublox_serial_parity_bits{ublox_serial_group,
                                                             "parity_bits",
                                                             "Parity Bits",
                                                             {"ublox-serial-parity"},
                                                             args::Options::Single};

static args::Group ublox_i2c_group{
    ublox_receiver_group,
    "I2C:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ublox_i2c_device{
    ublox_i2c_group, "device", "Device", {"ublox-i2c"}, args::Options::Single};
static args::ValueFlag<uint8_t> ublox_i2c_address{
    ublox_i2c_group, "address", "Address", {"ublox-i2c-address"}, args::Options::Single};

static args::Group ublox_tcp_device{
    ublox_receiver_group,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ublox_tcp_ip_address{
    ublox_tcp_device, "ip_address", "Host or IP Address", {"ublox-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> ublox_tcp_port{
    ublox_tcp_device, "port", "Port", {"ublox-tcp-port"}, args::Options::Single};

static args::Group ublox_udp_device{
    ublox_receiver_group,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ublox_udp_ip_address{
    ublox_udp_device, "ip_address", "Host or IP Address", {"ublox-udp"}, args::Options::Single};
static args::ValueFlag<uint16_t> ublox_udp_port{
    ublox_udp_device, "port", "Port", {"ublox-udp-port"}, args::Options::Single};

//
// NMEA
//

static args::Group nmea_receiver_group{
    "NMEA Receiver:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group nmea_serial_group{
    nmea_receiver_group,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> nmea_serial_device{
    nmea_receiver_group, "device", "Device", {"nmea-serial"}, args::Options::Single};
static args::ValueFlag<int> nmea_serial_baud_rate{
    nmea_receiver_group, "baud_rate", "Baud Rate", {"nmea-serial-baud"}, args::Options::Single};
static args::ValueFlag<int> nmea_serial_data_bits{
    nmea_receiver_group, "data_bits", "Data Bits", {"nmea-serial-data"}, args::Options::Single};
static args::ValueFlag<int> nmea_serial_stop_bits{
    nmea_receiver_group, "stop_bits", "Stop Bits", {"nmea-serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> nmea_serial_parity_bits{nmea_receiver_group,
                                                            "parity_bits",
                                                            "Parity Bits",
                                                            {"nmea-serial-parity"},
                                                            args::Options::Single};
// export nmea to unix socket
static args::ValueFlag<std::string> nmea_export_un{nmea_receiver_group,
                                                   "unix socket",
                                                   "Export NMEA to unix socket",
                                                   {"nmea-export-un"},
                                                   args::Options::Single};
static args::ValueFlag<std::string> nmea_export_tcp{
    nmea_receiver_group, "ip", "Export NMEA to TCP", {"nmea-export-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> nmea_export_tcp_port{nmea_receiver_group,
                                                      "port",
                                                      "Export NMEA to TCP Port",
                                                      {"nmea-export-tcp-port"},
                                                      args::Options::Single};

//
// Output
//

static args::Group other_receiver_group{
    "Other Receiver Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Flag print_messages{other_receiver_group,
                                 "print-receiver-messages",
                                 "Print Receiver Messages",
                                 {"print-receiver-messages", "prm"},
                                 args::Options::Single};

//
// Output
//

static args::Group output{
    "Output:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group file_output{
    output,
    "File:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> file_path{
    file_output, "file_path", "Path", {"file"}, args::Options::Single};

static args::Group serial_output{
    output,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> serial_device{
    serial_output, "device", "Device", {"serial"}, args::Options::Single};
static args::ValueFlag<int> serial_baud_rate{
    serial_output, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
static args::ValueFlag<int> serial_data_bits{
    serial_output, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
static args::ValueFlag<int> serial_stop_bits{
    serial_output, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> serial_parity_bits{
    serial_output, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

static args::Group i2c_output{
    output,
    "I2C:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> i2c_device{
    i2c_output, "device", "Device", {"i2c"}, args::Options::Single};
static args::ValueFlag<uint8_t> i2c_address{
    i2c_output, "address", "Address", {"i2c-address"}, args::Options::Single};

static args::Group tcp_output{
    output,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> tcp_ip_address{
    tcp_output, "ip_address", "Host or IP Address", {"tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> tcp_port{
    tcp_output, "port", "Port", {"tcp-port"}, args::Options::Single};

static args::Group udp_output{
    output,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> udp_ip_address{
    udp_output, "ip_address", "Host or IP Address", {"udp"}, args::Options::Single};
static args::ValueFlag<uint16_t> udp_port{
    udp_output, "port", "Port", {"udp-port"}, args::Options::Single};

static args::Group stdout_output{
    output,
    "Stdout:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::Flag stdout_output_flag{
    stdout_output, "stdout", "Stdout", {"stdout"}, args::Options::Single};

//
// Location Information
//

static args::Group location_infomation{
    "Location Infomation:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Flag li_enable{
    location_infomation,
    "location-info",
    "Enable sending fake location information. Configure with '--fake-*' options.",
    {"fake-location-info", "fli"},
    args::Options::Single,
};
static args::Flag li_force{
    location_infomation,
    "force-location-info",
    "Force Location Information (always send even if not requested)",
    {"force-location-info"},
    args::Options::Single,
};
static args::Flag li_unlocked{
    location_infomation,
    "unlocked",
    "Send location reports without locking the update rate. By default, the update rate is locked "
    "to 1 second.",
    {"location-report-unlocked"},
    args::Options::Single,
};
static args::ValueFlag<double> li_latitude{location_infomation,
                                           "latitude",
                                           "Fake Latitude",
                                           {"fake-latitude", "flat"},
                                           args::Options::Single};
static args::ValueFlag<double> li_longitude{location_infomation,
                                            "longitude",
                                            "Fake Longitude",
                                            {"fake-longitude", "flon"},
                                            args::Options::Single};
static args::ValueFlag<double> li_altitude{location_infomation,
                                           "altitude",
                                           "Fake Altitude",
                                           {"fake-altitude", "falt"},
                                           args::Options::Single};
static args::Flag              li_conf95to39{location_infomation,
                                "confidence-95to39",
                                "Convert 95p confidence to 39p confidence",
                                             {"confidence-95to39"},
                                args::Options::Single};
static args::ValueFlag<double> li_override_horizontal_confidence{
    location_infomation,
    "override-horizontal-confidence",
    "Override horizontal confidence [0.0-1.0]",
    {"override-horizontal-confidence"},
    args::Options::Single,
};

//
// Control Options
//

static args::Group control_options{
    "Control Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group control_interface{
    control_options,
    "Interface:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

static args::Group ctrl_serial_output{
    control_interface,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ctrl_serial_device{
    ctrl_serial_output, "device", "Device", {"ctrl-serial"}, args::Options::Single};
static args::ValueFlag<int> ctrl_serial_baud_rate{
    ctrl_serial_output, "baud_rate", "Baud Rate", {"ctrl-serial-baud"}, args::Options::Single};
static args::ValueFlag<int> ctrl_serial_data_bits{
    ctrl_serial_output, "data_bits", "Data Bits", {"ctrl-serial-data"}, args::Options::Single};
static args::ValueFlag<int> ctrl_serial_stop_bits{
    ctrl_serial_output, "stop_bits", "Stop Bits", {"ctrl-serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> ctrl_serial_parity_bits{ctrl_serial_output,
                                                            "parity_bits",
                                                            "Parity Bits",
                                                            {"ctrl-serial-parity"},
                                                            args::Options::Single};

static args::Group ctrl_tcp_output{
    control_interface,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ctrl_tcp_ip_address{
    ctrl_tcp_output, "ip_address", "Host or IP Address", {"ctrl-tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> ctrl_tcp_port{
    ctrl_tcp_output, "port", "Port", {"ctrl-tcp-port"}, args::Options::Single};

static args::Group ctrl_udp_output{
    control_interface,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> ctrl_udp_ip_address{
    ctrl_udp_output, "ip_address", "Host or IP Address", {"ctrl-udp"}, args::Options::Single};
static args::ValueFlag<uint16_t> ctrl_udp_port{
    ctrl_udp_output, "port", "Port", {"ctrl-udp-port"}, args::Options::Single};

static args::Group ctrl_stdin_output{
    control_interface,
    "Stdin:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::Flag ctrl_stdin_output_flag{
    ctrl_stdin_output, "stdin", "Stdin", {"ctrl-stdin"}, args::Options::Single};

static args::Group ctrl_un_output{
    control_interface,
    "Unix Socket:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};

static args::ValueFlag<std::string> ctrl_un_output_path{
    ctrl_un_output, "path", "Path", {"ctrl-un"}, args::Options::Single};

//
// Options
//

static LocationServerOptions parse_location_server_options(Options& options) {
    LocationServerOptions location_server_options{};
    location_server_options.host = location_server_host.Get();
    location_server_options.port = 5431;
    location_server_options.ssl  = false;

    if (location_server_host) {
        location_server_options.host = location_server_host.Get();
    } else if (location_server_slp_host_imsi) {
        if (!options.identity_options.imsi) {
            throw args::RequiredError("`imsi` is required to use `slp-host-imsi`");
        } else if(options.identity_options.wait_for_identity) {
            throw args::ValidationError("`slp-host-imsi` cannot be used with `wait-for-identity`");
        }

        auto imsi   = *options.identity_options.imsi;
        auto digits = std::to_string(imsi).size();
        if (digits < 6) {
            throw args::ValidationError("`imsi` must be at least 6 digits long");
        }

        auto mcc = (imsi / (unsigned long long)std::pow(10, digits - 3)) % 1000;
        auto mnc = (imsi / (unsigned long long)std::pow(10, digits - 6)) % 1000;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "h-slp.%03llu.%03llu.pub.3gppnetwork.org", mnc, mcc);
        auto h_slp                   = std::string{buffer};
        location_server_options.host = h_slp;
    } else if (location_server_slp_host_cell) {
        auto mcc = options.cell_options.mcc;
        auto mnc = options.cell_options.mnc;
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "h-slp.%03d.%03d.pub.3gppnetwork.org", mnc, mcc);
        auto h_slp                   = std::string{buffer};
        location_server_options.host = h_slp;
    } else {
        throw args::RequiredError("`host` or `slp-host-cell` or `slp-host-imsi` is required");
    }

    if (location_server_port) {
        location_server_options.port = location_server_port.Get();
    }

    if (location_server_ssl) {
        location_server_options.ssl = location_server_ssl.Get();
    }

    return location_server_options;
}

static IdentityOptions parse_identity_options() {
    IdentityOptions identity_options{};
    identity_options.use_supl_identity_fix = false;
    identity_options.wait_for_identity     = false;

    if (msisdn) {
        identity_options.msisdn =
            std::unique_ptr<unsigned long long>{new unsigned long long{msisdn.Get()}};
    }

    if (imsi) {
        identity_options.imsi =
            std::unique_ptr<unsigned long long>{new unsigned long long{imsi.Get()}};
    }

    if (ipv4) {
        identity_options.ipv4 = std::unique_ptr<std::string>{new std::string{ipv4.Get()}};
    }

    if (wait_for_identity) {
        identity_options.wait_for_identity = true;
    }

    if (!identity_options.wait_for_identity && !identity_options.msisdn && !identity_options.imsi &&
        !identity_options.ipv4) {
        identity_options.imsi =
            std::unique_ptr<unsigned long long>{new unsigned long long{2460813579lu}};
    }

    if (use_supl_identity_fix) {
        identity_options.use_supl_identity_fix = true;
    }

    return identity_options;
}

static CellOptions parse_cell_options() {
    CellOptions cell_options{};
    cell_options.mcc   = mcc.Get();
    cell_options.mnc   = mnc.Get();
    cell_options.tac   = tac.Get();
    cell_options.cid   = ci.Get();
    cell_options.is_nr = is_nr ? is_nr.Get() : false;
    return cell_options;
}

static OutputOptions parse_output_options() {
    OutputOptions output_options{};

    if (file_path) {
        auto interface = interface::Interface::file(file_path.Get(), true);
        output_options.interfaces.emplace_back(interface);
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

        auto data_bits = DataBits::EIGHT;
        if (serial_data_bits) {
            switch (serial_data_bits.Get()) {
            case 5: data_bits = DataBits::FIVE; break;
            case 6: data_bits = DataBits::SIX; break;
            case 7: data_bits = DataBits::SEVEN; break;
            case 8: data_bits = DataBits::EIGHT; break;
            default: throw args::ValidationError("Invalid data bits");
            }
        }

        auto stop_bits = StopBits::ONE;
        if (serial_stop_bits) {
            switch (serial_stop_bits.Get()) {
            case 1: stop_bits = StopBits::ONE; break;
            case 2: stop_bits = StopBits::TWO; break;
            default: throw args::ValidationError("Invalid stop bits");
            }
        }

        auto parity_bit = ParityBit::NONE;
        if (serial_parity_bits) {
            if (serial_parity_bits.Get() == "none") {
                parity_bit = ParityBit::NONE;
            } else if (serial_parity_bits.Get() == "odd") {
                parity_bit = ParityBit::ODD;
            } else if (serial_parity_bits.Get() == "even") {
                parity_bit = ParityBit::EVEN;
            } else {
                throw args::ValidationError("Invalid parity bits");
            }
        }

        auto interface = interface::Interface::serial(serial_device.Get(), baud_rate, data_bits,
                                                      stop_bits, parity_bit);
        output_options.interfaces.emplace_back(interface);
    }

    if (i2c_device || i2c_address) {
        if (!i2c_device) {
            throw args::RequiredError("i2c_device");
        }

        if (!i2c_address) {
            throw args::RequiredError("i2c_address");
        }

        auto interface = interface::Interface::i2c(i2c_device.Get(), i2c_address.Get());
        output_options.interfaces.emplace_back(interface);
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
        output_options.interfaces.emplace_back(interface);
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
        output_options.interfaces.emplace_back(interface);
    }

    if (stdout_output_flag) {
        auto interface = interface::Interface::stdout();
        output_options.interfaces.emplace_back(interface);
    }

    for (auto& interface : output_options.interfaces) {
        interface->open();
    }

    return output_options;
}

//
// u-blox
//

static std::unique_ptr<Interface> ublox_parse_serial() {
    assert(ublox_serial_device);

    uint32_t baud_rate = 115200;
    if (ublox_serial_baud_rate) {
        if (ublox_serial_baud_rate.Get() < 0) {
            throw args::ValidationError("ublox-serial-baud-rate must be positive");
        }

        baud_rate = static_cast<uint32_t>(ublox_serial_baud_rate.Get());
    }

    auto data_bits = DataBits::EIGHT;
    if (ublox_serial_data_bits) {
        switch (ublox_serial_data_bits.Get()) {
        case 5: data_bits = DataBits::FIVE; break;
        case 6: data_bits = DataBits::SIX; break;
        case 7: data_bits = DataBits::SEVEN; break;
        case 8: data_bits = DataBits::EIGHT; break;
        default: throw args::ValidationError("Invalid data bits");
        }
    }

    auto stop_bits = StopBits::ONE;
    if (ublox_serial_stop_bits) {
        switch (ublox_serial_stop_bits.Get()) {
        case 1: stop_bits = StopBits::ONE; break;
        case 2: stop_bits = StopBits::TWO; break;
        default: throw args::ValidationError("Invalid stop bits");
        }
    }

    auto parity_bit = ParityBit::NONE;
    if (ublox_serial_parity_bits) {
        if (ublox_serial_parity_bits.Get() == "none") {
            parity_bit = ParityBit::NONE;
        } else if (ublox_serial_parity_bits.Get() == "odd") {
            parity_bit = ParityBit::ODD;
        } else if (ublox_serial_parity_bits.Get() == "even") {
            parity_bit = ParityBit::EVEN;
        } else {
            throw args::ValidationError("Invalid parity bits");
        }
    }

    return std::unique_ptr<Interface>(
        Interface::serial(ublox_serial_device.Get(), baud_rate, data_bits, stop_bits, parity_bit));
}

static std::unique_ptr<Interface> ublox_parse_i2c() {
    assert(ublox_i2c_device);

    uint8_t address = 66;
    if (ublox_i2c_address) {
        address = ublox_i2c_address.Get();
    }

    return std::unique_ptr<Interface>(Interface::i2c(ublox_i2c_device.Get(), address));
}

static std::unique_ptr<Interface> ublox_parse_tcp() {
    assert(ublox_tcp_ip_address);

    if (!ublox_tcp_port) {
        throw args::RequiredError("ublox-tcp-port");
    }

    return std::unique_ptr<Interface>(
        Interface::tcp(ublox_tcp_ip_address.Get(), ublox_tcp_port.Get(), true /* reconnect */));
}

static std::unique_ptr<Interface> ublox_parse_udp() {
    assert(ublox_udp_ip_address);

    if (!ublox_udp_port) {
        throw args::RequiredError("ublox-udp-port");
    }

    return std::unique_ptr<Interface>(
        Interface::udp(ublox_udp_ip_address.Get(), ublox_udp_port.Get(), true /* reconnect */));
}

static std::unique_ptr<Interface> ublox_parse_interface() {
    if (ublox_serial_device) {
        return ublox_parse_serial();
    } else if (ublox_i2c_device) {
        return ublox_parse_i2c();
    } else if (ublox_tcp_ip_address) {
        return ublox_parse_tcp();
    } else if (ublox_udp_ip_address) {
        return ublox_parse_udp();
    } else {
        throw args::RequiredError("No device/interface specified for u-blox receiver");
    }
}

static Port ublox_parse_port() {
    if (ublox_receiver_port) {
        if (ublox_receiver_port.Get() == "uart1") {
            return Port::UART1;
        } else if (ublox_receiver_port.Get() == "uart2") {
            return Port::UART2;
        } else if (ublox_receiver_port.Get() == "i2c") {
            return Port::I2C;
        } else if (ublox_receiver_port.Get() == "usb") {
            return Port::USB;
        } else {
            throw args::ValidationError("Invalid port");
        }
    } else {
        if (ublox_serial_device) {
            return Port::UART1;
        } else if (ublox_i2c_device) {
            return Port::I2C;
        } else {
            throw args::RequiredError("u-blox port must be specified for this device/interface");
        }
    }
}

static bool print_receiver_options_parse() {
    if (print_messages) {
        return true;
    } else {
        return false;
    }
}

static UbloxOptions ublox_parse_options() {
    if (ublox_serial_device || ublox_i2c_device || ublox_tcp_ip_address || ublox_udp_ip_address) {
        auto port      = ublox_parse_port();
        auto interface = ublox_parse_interface();
        auto prm       = print_receiver_options_parse();
        return UbloxOptions{port, std::move(interface), prm};
    } else {
        return UbloxOptions{};
    }
}

static NmeaOptions nmea_parse_options() {
    if (nmea_serial_device) {
        uint32_t baud_rate = 115200;
        if (nmea_serial_baud_rate) {
            if (nmea_serial_baud_rate.Get() < 0) {
                throw args::ValidationError("nmea-serial-baud-rate must be positive");
            }

            baud_rate = static_cast<uint32_t>(nmea_serial_baud_rate.Get());
        }

        auto data_bits = DataBits::EIGHT;
        if (nmea_serial_data_bits) {
            switch (nmea_serial_data_bits.Get()) {
            case 5: data_bits = DataBits::FIVE; break;
            case 6: data_bits = DataBits::SIX; break;
            case 7: data_bits = DataBits::SEVEN; break;
            case 8: data_bits = DataBits::EIGHT; break;
            default: throw args::ValidationError("Invalid data bits");
            }
        }

        auto stop_bits = StopBits::ONE;
        if (nmea_serial_stop_bits) {
            switch (nmea_serial_stop_bits.Get()) {
            case 1: stop_bits = StopBits::ONE; break;
            case 2: stop_bits = StopBits::TWO; break;
            default: throw args::ValidationError("Invalid stop bits");
            }
        }

        auto parity_bit = ParityBit::NONE;
        if (nmea_serial_parity_bits) {
            if (nmea_serial_parity_bits.Get() == "none") {
                parity_bit = ParityBit::NONE;
            } else if (nmea_serial_parity_bits.Get() == "odd") {
                parity_bit = ParityBit::ODD;
            } else if (nmea_serial_parity_bits.Get() == "even") {
                parity_bit = ParityBit::EVEN;
            } else {
                throw args::ValidationError("Invalid parity bits");
            }
        }

        std::vector<std::unique_ptr<interface::Interface>> nmea_export_interfaces;
        if (nmea_export_un) {
            auto interface = interface::Interface::unix_socket_stream(nmea_export_un.Get(), true);
            nmea_export_interfaces.emplace_back(interface);
        }

        if (nmea_export_tcp) {
            if (!nmea_export_tcp_port) {
                throw args::RequiredError("nmea-export-tcp-port");
            }

            auto interface =
                interface::Interface::tcp(nmea_export_tcp.Get(), nmea_export_tcp_port.Get(), true);
            nmea_export_interfaces.emplace_back(interface);
        }

        auto interface = interface::Interface::serial(nmea_serial_device.Get(), baud_rate,
                                                      data_bits, stop_bits, parity_bit);
        auto prm       = print_receiver_options_parse();
        return NmeaOptions{std::unique_ptr<Interface>(interface), prm,
                           std::move(nmea_export_interfaces)};
    } else {
        return NmeaOptions{};
    }
}

static LocationInformationOptions parse_location_information_options() {
    LocationInformationOptions location_information{};
    location_information.latitude                       = 69.0599730655754;
    location_information.longitude                      = 20.54864403253676;
    location_information.altitude                       = 0;
    location_information.force                          = false;
    location_information.unlock_update_rate             = false;
    location_information.convert_confidence_95_to_39    = false;
    location_information.override_horizontal_confidence = -1.0;

    if (li_force) {
        location_information.force = true;
    }

    if (li_unlocked) {
        location_information.unlock_update_rate = true;
    }

    if (li_enable) {
        location_information.enabled = true;
        if (li_latitude) {
            location_information.latitude = li_latitude.Get();
        }

        if (li_longitude) {
            location_information.longitude = li_longitude.Get();
        }

        if (li_altitude) {
            location_information.altitude = li_altitude.Get();
        }
    }

    if (li_conf95to39) {
        location_information.convert_confidence_95_to_39 = true;
    }

    if (li_override_horizontal_confidence) {
        location_information.override_horizontal_confidence =
            li_override_horizontal_confidence.Get();
        if (location_information.override_horizontal_confidence < 0 ||
            location_information.override_horizontal_confidence > 1) {
            throw args::ValidationError(
                "Override horizontal confidence must be between 0.0 and 1.0");
        }
    }

    return location_information;
}

static std::unique_ptr<Interface> control_parse_serial() {
    assert(ctrl_serial_device);

    uint32_t baud_rate = 115200;
    if (ctrl_serial_baud_rate) {
        if (ctrl_serial_baud_rate.Get() < 0) {
            throw args::ValidationError("ctrl-serial-baud-rate must be positive");
        }

        baud_rate = static_cast<uint32_t>(ctrl_serial_baud_rate.Get());
    }

    auto data_bits = DataBits::EIGHT;
    if (ctrl_serial_data_bits) {
        switch (ctrl_serial_data_bits.Get()) {
        case 5: data_bits = DataBits::FIVE; break;
        case 6: data_bits = DataBits::SIX; break;
        case 7: data_bits = DataBits::SEVEN; break;
        case 8: data_bits = DataBits::EIGHT; break;
        default: throw args::ValidationError("Invalid data bits");
        }
    }

    auto stop_bits = StopBits::ONE;
    if (ctrl_serial_stop_bits) {
        switch (ctrl_serial_stop_bits.Get()) {
        case 1: stop_bits = StopBits::ONE; break;
        case 2: stop_bits = StopBits::TWO; break;
        default: throw args::ValidationError("Invalid stop bits");
        }
    }

    auto parity_bit = ParityBit::NONE;
    if (ctrl_serial_parity_bits) {
        if (ctrl_serial_parity_bits.Get() == "none") {
            parity_bit = ParityBit::NONE;
        } else if (ctrl_serial_parity_bits.Get() == "odd") {
            parity_bit = ParityBit::ODD;
        } else if (ctrl_serial_parity_bits.Get() == "even") {
            parity_bit = ParityBit::EVEN;
        } else {
            throw args::ValidationError("Invalid parity bits");
        }
    }

    return std::unique_ptr<Interface>(
        Interface::serial(ctrl_serial_device.Get(), baud_rate, data_bits, stop_bits, parity_bit));
}

static std::unique_ptr<Interface> control_parse_tcp() {
    assert(ctrl_tcp_ip_address);

    if (!ctrl_tcp_port) {
        throw args::RequiredError("ctrl-tcp-port");
    }

    return std::unique_ptr<Interface>(
        Interface::tcp(ctrl_tcp_ip_address.Get(), ctrl_tcp_port.Get(), true /* reconnect */));
}

static std::unique_ptr<Interface> control_parse_udp() {
    assert(ctrl_udp_ip_address);

    if (!ctrl_udp_port) {
        throw args::RequiredError("ctrl-udp-port");
    }

    return std::unique_ptr<Interface>(
        Interface::udp(ctrl_udp_ip_address.Get(), ctrl_udp_port.Get(), true /* reconnect */));
}

static std::unique_ptr<Interface> control_parse_stdin() {
    return std::unique_ptr<Interface>(Interface::stdin());
}

static std::unique_ptr<Interface> control_parse_unix_socket() {
    assert(ctrl_un_output_path);

    return std::unique_ptr<Interface>(
        Interface::unix_socket_stream(ctrl_un_output_path.Get(), true));
}

static std::unique_ptr<Interface> control_parse_interface() {
    if (ctrl_serial_device) {
        return control_parse_serial();
    } else if (ctrl_tcp_ip_address) {
        return control_parse_tcp();
    } else if (ctrl_udp_ip_address) {
        return control_parse_udp();
    } else if (ctrl_stdin_output_flag) {
        return control_parse_stdin();
    } else if (ctrl_un_output_path) {
        return control_parse_unix_socket();
    } else {
        throw args::RequiredError("No device/interface specified for control interface");
    }
}

static ControlOptions parse_control_options() {
    if (ctrl_serial_device || ctrl_tcp_ip_address || ctrl_udp_ip_address ||
        ctrl_stdin_output_flag || ctrl_un_output_path) {
        auto interface = control_parse_interface();
        return ControlOptions{std::move(interface)};
    } else {
        return ControlOptions{};
    }
}

//
// Option Parser
//

OptionParser::OptionParser() {}

void OptionParser::add_command(Command* command) {
    add_command(std::unique_ptr<Command>(command));
}

void OptionParser::add_command(std::unique_ptr<Command> command) {
    mCommands.push_back(std::move(command));
}

int OptionParser::parse_and_execute(int argc, char** argv) {
    args::ArgumentParser parser(
        "3GPP LPP Example (" CLIENT_VERSION
        ") - This sample code is a simple client that asks for assistance data from a location "
        "server. It can handle OSR, SSR, and AGNSS requests. The assistance data can converted to "
        "RTCM or SPARTN before being sent to a GNSS receiver or other interface. The client also "
        "supports to 3GPP LPP Provide Location Information, which can be used to send the device's "
        "location to the location server.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    // Commands
    args::Group commands{
        parser,
        "Commands:",
        args::Group::Validators::DontCare,
        args::Options::Global,
    };

    std::vector<std::unique_ptr<args::Command>> args_commands;
    for (auto& command : mCommands) {
        auto command_ptr = command.get();
        args_commands.emplace_back(new args::Command(
            commands, command->name(), command->description(),
            [command_ptr](args::Subparser& subparser) {
                command_ptr->parse(subparser);
                subparser.Parse();

                Options options{};
                options.cell_options                 = parse_cell_options();
                options.identity_options             = parse_identity_options();
                options.location_server_options      = parse_location_server_options(options);
                options.output_options               = parse_output_options();
                options.ublox_options                = ublox_parse_options();
                options.nmea_options                 = nmea_parse_options();
                options.location_information_options = parse_location_information_options();
                options.control_options              = parse_control_options();
                command_ptr->execute(std::move(options));
            }));
    }

    // Defaults
    ublox_i2c_address.HelpDefault("66");
    ublox_serial_baud_rate.HelpDefault("115200");
    ublox_serial_data_bits.HelpDefault("8");
    ublox_serial_data_bits.HelpChoices({"5", "6", "7", "8"});
    ublox_serial_stop_bits.HelpDefault("1");
    ublox_serial_stop_bits.HelpChoices({"1", "2"});
    ublox_serial_parity_bits.HelpDefault("none");
    ublox_serial_parity_bits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    ublox_receiver_port.HelpDefault("(by interface)");
    ublox_receiver_port.HelpChoices({
        "uart1",
        "uart2",
        "i2c",
        "usb",
    });

    nmea_serial_baud_rate.HelpDefault("115200");
    nmea_serial_data_bits.HelpDefault("8");
    nmea_serial_data_bits.HelpChoices({"5", "6", "7", "8"});
    nmea_serial_stop_bits.HelpDefault("1");
    nmea_serial_stop_bits.HelpChoices({"1", "2"});
    nmea_serial_parity_bits.HelpDefault("none");
    nmea_serial_parity_bits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    location_server_port.HelpDefault("5431");
    location_server_ssl.HelpDefault("false");
    imsi.HelpDefault("2460813579");

    i2c_address.HelpDefault("66");
    serial_baud_rate.HelpDefault("115200");
    serial_data_bits.HelpDefault("8");
    serial_data_bits.HelpChoices({"5", "6", "7", "8"});
    serial_stop_bits.HelpDefault("1");
    serial_stop_bits.HelpChoices({"1", "2"});
    serial_parity_bits.HelpDefault("none");
    serial_parity_bits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    li_latitude.HelpDefault("69.0599730655754");
    li_longitude.HelpDefault("20.54864403253676");
    li_altitude.HelpDefault("0");

    ctrl_serial_baud_rate.HelpDefault("115200");
    ctrl_serial_data_bits.HelpDefault("8");
    ctrl_serial_data_bits.HelpChoices({"5", "6", "7", "8"});
    ctrl_serial_stop_bits.HelpDefault("1");
    ctrl_serial_stop_bits.HelpChoices({"1", "2"});
    ctrl_serial_parity_bits.HelpDefault("none");
    ctrl_serial_parity_bits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    // Globals
    args::GlobalOptions location_server_globals{parser, location_server};
    args::GlobalOptions identity_globals{parser, identity};
    args::GlobalOptions cell_information_globals{parser, cell_information};
    args::GlobalOptions ublox_receiver_globals{parser, ublox_receiver_group};
    args::GlobalOptions nmea_receiver_globals{parser, nmea_receiver_group};
    args::GlobalOptions other_receiver_globals{parser, other_receiver_group};
    args::GlobalOptions output_globals{parser, output};
    args::GlobalOptions location_information_globals{parser, location_infomation};
    args::GlobalOptions control_options_globals{parser, control_options};

    // Parse
    try {
        parser.ParseCLI(argc, argv);
        if (version) {
            std::cout << "3GPP LPP Example (" << CLIENT_VERSION << ")" << std::endl;
            return 0;
        }

        return 0;
    } catch (args::ValidationError const& e) {
        std::cerr << e.what() << std::endl;
        parser.Help(std::cerr);
        return 1;
    } catch (args::Help const&) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError const& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
