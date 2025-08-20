#include "options.hpp"

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

static args::Group arguments{"Arguments:"};

//
// NTRIP
//

static args::Group ntrip{
    arguments,
    "NTRIP:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> ntrip_hostname{
    ntrip, "hostname", "Hostname", {"host"}, args::Options::Single};
static args::ValueFlag<int> ntrip_port{ntrip, "port", "Port", {"port"}, args::Options::Single};
static args::ValueFlag<std::string> ntrip_mountpoint{
    ntrip, "mountpoint", "Mountpoint", {"mountpoint"}, args::Options::Single};
static args::ValueFlag<std::string> ntrip_username{
    ntrip, "username", "Username", {"username"}, args::Options::Single};
static args::ValueFlag<std::string> ntrip_password{
    ntrip, "password", "Password", {"password"}, args::Options::Single};

static args::ValueFlag<std::string> nmea_string{
    ntrip, "nmea_string", "NMEA String", {"nmea"}, args::Options::Single};
static args::Flag hexdump_flag{ntrip, "hexdump", "Hexdump", {"hexdump"}, args::Options::Single};

static HostOptions parse_host_options() {
    if (!ntrip_hostname) {
        throw args::RequiredError("ntrip_hostname");
    }

    std::unique_ptr<std::string> mountpoint;
    if (ntrip_mountpoint) {
        mountpoint = std::unique_ptr<std::string>(new std::string(ntrip_mountpoint.Get()));
    }

    uint16_t port = 2101;
    if (ntrip_port) {
        if (ntrip_port.Get() < 0) {
            throw args::ValidationError("ntrip_port must be positive");
        }

        port = static_cast<uint16_t>(ntrip_port.Get());
    }

    std::string username;
    if (ntrip_username) {
        username = ntrip_username.Get();
    }

    std::string password;
    if (ntrip_password) {
        password = ntrip_password.Get();
    }

    std::string nmea;
    if (nmea_string) {
        nmea = nmea_string.Get();
    }

    bool hexdump = false;
    if (hexdump_flag) {
        hexdump = true;
    }

    HostOptions hostOptions;
    hostOptions.hostname   = ntrip_hostname.Get();
    hostOptions.port       = port;
    hostOptions.mountpoint = std::move(mountpoint);
    hostOptions.username   = username;
    hostOptions.password   = password;
    hostOptions.nmea       = nmea;
    hostOptions.hexdump    = hexdump;
    return hostOptions;
}

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
static args::ValueFlag<std::string> serial_baud_rate{
    serial_output, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
static args::ValueFlag<int> serial_data_bits{
    serial_output, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
static args::ValueFlag<int> serial_stop_bits{
    serial_output, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> serial_parity_bits{
    serial_output, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

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

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("serial_baud_rate must be an integer, got `" + str + "'");
    }

    if (baud_rate == 50) return io::BaudRate::BR50;
    if (baud_rate == 75) return io::BaudRate::BR75;
    if (baud_rate == 110) return io::BaudRate::BR110;
    if (baud_rate == 134) return io::BaudRate::BR134;
    if (baud_rate == 150) return io::BaudRate::BR150;
    if (baud_rate == 200) return io::BaudRate::BR200;
    if (baud_rate == 300) return io::BaudRate::BR300;
    if (baud_rate == 600) return io::BaudRate::BR600;
    if (baud_rate == 1200) return io::BaudRate::BR1200;
    if (baud_rate == 1800) return io::BaudRate::BR1800;
    if (baud_rate == 2400) return io::BaudRate::BR2400;
    if (baud_rate == 4800) return io::BaudRate::BR4800;
    if (baud_rate == 9600) return io::BaudRate::BR9600;
    if (baud_rate == 19200) return io::BaudRate::BR19200;
    if (baud_rate == 38400) return io::BaudRate::BR38400;
    if (baud_rate == 57600) return io::BaudRate::BR57600;
    if (baud_rate == 115200) return io::BaudRate::BR115200;
    if (baud_rate == 230400) return io::BaudRate::BR230400;
    if (baud_rate == 460800) return io::BaudRate::BR460800;
    if (baud_rate == 500000) return io::BaudRate::BR500000;
    if (baud_rate == 576000) return io::BaudRate::BR576000;
    if (baud_rate == 921600) return io::BaudRate::BR921600;
    if (baud_rate == 1000000) return io::BaudRate::BR1000000;
    if (baud_rate == 1152000) return io::BaudRate::BR1152000;
    if (baud_rate == 1500000) return io::BaudRate::BR1500000;
    if (baud_rate == 2000000) return io::BaudRate::BR2000000;
    if (baud_rate == 2500000) return io::BaudRate::BR2500000;
    if (baud_rate == 3000000) return io::BaudRate::BR3000000;
    if (baud_rate == 3500000) return io::BaudRate::BR3500000;
    if (baud_rate == 4000000) return io::BaudRate::BR4000000;
    throw args::ParseError("serial_baud_rate must be a valid baud rate, got `" + str + "'");
}

static OutputOptions parse_output_options() {
    OutputOptions output_options{};

    if (file_path) {
        auto iout = new io::FileOutput(file_path.Get(), true, false, true);
        output_options.outputs.emplace_back(iout);
    }

    if (serial_device || serial_baud_rate) {
        if (!serial_device) {
            throw args::RequiredError("serial_device");
        }

        auto baud_rate = io::BaudRate::BR115200;
        if (serial_baud_rate) {
            baud_rate = parse_baudrate(serial_baud_rate.Get());
        }

        auto data_bits = io::DataBits::EIGHT;
        if (serial_data_bits) {
            switch (serial_data_bits.Get()) {
            case 5: data_bits = io::DataBits::FIVE; break;
            case 6: data_bits = io::DataBits::SIX; break;
            case 7: data_bits = io::DataBits::SEVEN; break;
            case 8: data_bits = io::DataBits::EIGHT; break;
            default: throw args::ValidationError("Invalid data bits");
            }
        }

        auto stop_bits = io::StopBits::ONE;
        if (serial_stop_bits) {
            switch (serial_stop_bits.Get()) {
            case 1: stop_bits = io::StopBits::ONE; break;
            case 2: stop_bits = io::StopBits::TWO; break;
            default: throw args::ValidationError("Invalid stop bits");
            }
        }

        auto parity_bit = io::ParityBit::NONE;
        if (serial_parity_bits) {
            if (serial_parity_bits.Get() == "none") {
                parity_bit = io::ParityBit::NONE;
            } else if (serial_parity_bits.Get() == "odd") {
                parity_bit = io::ParityBit::ODD;
            } else if (serial_parity_bits.Get() == "even") {
                parity_bit = io::ParityBit::EVEN;
            } else {
                throw args::ValidationError("Invalid parity bits");
            }
        }

        auto iout =
            new io::SerialOutput(serial_device.Get(), baud_rate, data_bits, stop_bits, parity_bit);
        output_options.outputs.emplace_back(iout);
    }

    if (tcp_ip_address || tcp_port) {
        if (!tcp_ip_address) {
            throw args::RequiredError("tcp_ip_address");
        }

        if (!tcp_port) {
            throw args::RequiredError("tcp_port");
        }

        auto iout = new io::TcpClientOutput(tcp_ip_address.Get(),
                                            static_cast<uint16_t>(tcp_port.Get()), true);
        output_options.outputs.emplace_back(iout);
    }

    if (udp_ip_address || udp_port) {
        if (!udp_ip_address) {
            throw args::RequiredError("udp_ip_address");
        }

        if (!udp_port) {
            throw args::RequiredError("udp_port");
        }

        auto iout =
            new io::UdpClientOutput(udp_ip_address.Get(), static_cast<uint16_t>(udp_port.Get()));
        output_options.outputs.emplace_back(iout);
    }

    if (stdout_output_flag) {
        auto iout = new io::StdoutOutput();
        output_options.outputs.emplace_back(iout);
    }

    return output_options;
}

Options parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("NTRIP Example (" CLIENT_VERSION
                                ") - This sample code illustrates the process of utilizing the "
                                "NTRIP client to establish a communication link with a caster and "
                                "transmit the data to a serial port, file, or stdout.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    ntrip_port.HelpDefault("2101");

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

    // Globals
    args::GlobalOptions ntrip_globals{parser, ntrip};
    args::GlobalOptions output_globals{parser, output};

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "NTRIP Example (" CLIENT_VERSION ")" << std::endl;
            exit(0);
        }

        Options options{};
        options.host   = parse_host_options();
        options.output = parse_output_options();
        return options;
    } catch (args::ValidationError const& e) {
        std::cerr << e.what() << std::endl;
        parser.Help(std::cerr);
        exit(1);
    } catch (args::Help const&) {
        std::cout << parser;
        exit(0);
    } catch (args::ParseError const& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        exit(1);
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
}
