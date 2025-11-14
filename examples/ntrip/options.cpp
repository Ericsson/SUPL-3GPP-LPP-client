#include "options.hpp"
#include <version.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hpp>
EXTERNAL_WARNINGS_POP

static args::Group gArguments{"Arguments:"};

//
// gNtrip
//

static args::Group gNtrip{
    gArguments,
    "gNtrip:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> gNtripHostname{
    gNtrip, "hostname", "Hostname", {"host"}, args::Options::Single};
static args::ValueFlag<int> gNtripPort{gNtrip, "port", "Port", {"port"}, args::Options::Single};
static args::ValueFlag<std::string> gNtripMountpoint{
    gNtrip, "mountpoint", "Mountpoint", {"mountpoint"}, args::Options::Single};
static args::ValueFlag<std::string> gNtripUsername{
    gNtrip, "username", "Username", {"username"}, args::Options::Single};
static args::ValueFlag<std::string> gNtripPassword{
    gNtrip, "password", "Password", {"password"}, args::Options::Single};

static args::ValueFlag<std::string> gNmeaString{
    gNtrip, "gNmeaString", "NMEA String", {"nmea"}, args::Options::Single};
static args::Flag gHexdumpFlag{gNtrip, "hexdump", "Hexdump", {"hexdump"}, args::Options::Single};

static HostOptions parse_host_options() {
    if (!gNtripHostname) {
        throw args::RequiredError("gNtripHostname");
    }

    std::unique_ptr<std::string> mountpoint;
    if (gNtripMountpoint) {
        mountpoint = std::unique_ptr<std::string>(new std::string(gNtripMountpoint.Get()));
    }

    uint16_t port = 2101;
    if (gNtripPort) {
        if (gNtripPort.Get() < 0) {
            throw args::ValidationError("gNtripPort must be positive");
        }

        port = static_cast<uint16_t>(gNtripPort.Get());
    }

    std::string username;
    if (gNtripUsername) {
        username = gNtripUsername.Get();
    }

    std::string password;
    if (gNtripPassword) {
        password = gNtripPassword.Get();
    }

    std::string nmea;
    if (gNmeaString) {
        nmea = gNmeaString.Get();
    }

    bool hexdump = false;
    if (gHexdumpFlag) {
        hexdump = true;
    }

    HostOptions host_options;
    host_options.hostname   = gNtripHostname.Get();
    host_options.port       = port;
    host_options.mountpoint = std::move(mountpoint);
    host_options.username   = username;
    host_options.password   = password;
    host_options.nmea       = nmea;
    host_options.hexdump    = hexdump;
    return host_options;
}

//
// Output
//

static args::Group gOutput{
    "Output:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group gFileOutput{
    gOutput,
    "File:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gFilePath{
    gFileOutput, "gFilePath", "Path", {"file"}, args::Options::Single};

static args::Group gSerialOutput{
    gOutput,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gSerialDevice{
    gSerialOutput, "device", "Device", {"serial"}, args::Options::Single};
static args::ValueFlag<std::string> gSerialBaudRate{
    gSerialOutput, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
static args::ValueFlag<int> gSerialDataBits{
    gSerialOutput, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
static args::ValueFlag<int> gSerialStopBits{
    gSerialOutput, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> gSerialParityBits{
    gSerialOutput, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

static args::Group gTcpOutput{
    gOutput,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gTcpIpAddress{
    gTcpOutput, "ip_address", "Host or IP Address", {"tcp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gTcpPort{
    gTcpOutput, "port", "Port", {"tcp-port"}, args::Options::Single};

static args::Group gUdpOutput{
    gOutput,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::ValueFlag<std::string> gUdpIpAddress{
    gUdpOutput, "ip_address", "Host or IP Address", {"udp"}, args::Options::Single};
static args::ValueFlag<uint16_t> gUdpPort{
    gUdpOutput, "port", "Port", {"udp-port"}, args::Options::Single};

static args::Group gStdoutOutput{
    gOutput,
    "Stdout:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
static args::Flag gStdoutOutputFlag{
    gStdoutOutput, "stdout", "Stdout", {"stdout"}, args::Options::Single};

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = 0;
    try {
        baud_rate = std::stol(str);
    } catch (...) {
        throw args::ParseError("gSerialBaudRate must be an integer, got `" + str + "'");
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
    throw args::ParseError("gSerialBaudRate must be a valid baud rate, got `" + str + "'");
}

static OutputOptions parse_output_options() {
    OutputOptions output_options{};

    if (gFilePath) {
        auto iout = new io::FileOutput(gFilePath.Get(), true, false, true);
        output_options.outputs.emplace_back(iout);
    }

    if (gSerialDevice || gSerialBaudRate) {
        if (!gSerialDevice) {
            throw args::RequiredError("gSerialDevice");
        }

        auto baud_rate = io::BaudRate::BR115200;
        if (gSerialBaudRate) {
            baud_rate = parse_baudrate(gSerialBaudRate.Get());
        }

        auto data_bits = io::DataBits::EIGHT;
        if (gSerialDataBits) {
            switch (gSerialDataBits.Get()) {
            case 5: data_bits = io::DataBits::FIVE; break;
            case 6: data_bits = io::DataBits::SIX; break;
            case 7: data_bits = io::DataBits::SEVEN; break;
            case 8: data_bits = io::DataBits::EIGHT; break;
            default: throw args::ValidationError("Invalid data bits");
            }
        }

        auto stop_bits = io::StopBits::ONE;
        if (gSerialStopBits) {
            switch (gSerialStopBits.Get()) {
            case 1: stop_bits = io::StopBits::ONE; break;
            case 2: stop_bits = io::StopBits::TWO; break;
            default: throw args::ValidationError("Invalid stop bits");
            }
        }

        auto parity_bit = io::ParityBit::NONE;
        if (gSerialParityBits) {
            if (gSerialParityBits.Get() == "none") {
                parity_bit = io::ParityBit::NONE;
            } else if (gSerialParityBits.Get() == "odd") {
                parity_bit = io::ParityBit::ODD;
            } else if (gSerialParityBits.Get() == "even") {
                parity_bit = io::ParityBit::EVEN;
            } else {
                throw args::ValidationError("Invalid parity bits");
            }
        }

        auto iout =
            new io::SerialOutput(gSerialDevice.Get(), baud_rate, data_bits, stop_bits, parity_bit);
        output_options.outputs.emplace_back(iout);
    }

    if (gTcpIpAddress || gTcpPort) {
        if (!gTcpIpAddress) {
            throw args::RequiredError("gTcpIpAddress");
        }

        if (!gTcpPort) {
            throw args::RequiredError("gTcpPort");
        }

        auto iout = new io::TcpClientOutput(gTcpIpAddress.Get(),
                                            static_cast<uint16_t>(gTcpPort.Get()), true);
        output_options.outputs.emplace_back(iout);
    }

    if (gUdpIpAddress || gUdpPort) {
        if (!gUdpIpAddress) {
            throw args::RequiredError("gUdpIpAddress");
        }

        if (!gUdpPort) {
            throw args::RequiredError("gUdpPort");
        }

        auto iout =
            new io::UdpClientOutput(gUdpIpAddress.Get(), static_cast<uint16_t>(gUdpPort.Get()));
        output_options.outputs.emplace_back(iout);
    }

    if (gStdoutOutputFlag) {
        auto iout = new io::StdoutOutput();
        output_options.outputs.emplace_back(iout);
    }

    return output_options;
}

Options parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("gNtrip Example (" CLIENT_VERSION
                                ") - This sample code illustrates the process of utilizing the "
                                "gNtrip client to establish a communication link with a caster and "
                                "transmit the data to a serial port, file, or stdout.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    gNtripPort.HelpDefault("2101");

    gSerialBaudRate.HelpDefault("115200");

    gSerialDataBits.HelpDefault("8");
    gSerialDataBits.HelpChoices({"5", "6", "7", "8"});

    gSerialStopBits.HelpDefault("1");
    gSerialStopBits.HelpChoices({"1", "2"});

    gSerialParityBits.HelpDefault("none");
    gSerialParityBits.HelpChoices({
        "none",
        "odd",
        "even",
    });

    // Globals
    args::GlobalOptions ntrip_globals{parser, gNtrip};
    args::GlobalOptions output_globals{parser, gOutput};

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "gNtrip Example " << CLIENT_VERSION << std::endl;
            std::cout << "  Commit: " << GIT_COMMIT_HASH << (GIT_DIRTY ? "-dirty" : "") << " ("
                      << GIT_BRANCH << ")" << std::endl;
            std::cout << "  Built: " << BUILD_DATE << " [" << BUILD_TYPE << "]" << std::endl;
            std::cout << "  Compiler: " << BUILD_COMPILER << std::endl;
            std::cout << "  Platform: " << BUILD_SYSTEM << " (" << BUILD_ARCH << ")" << std::endl;
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
