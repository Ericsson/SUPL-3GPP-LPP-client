#include "options.hpp"
#include <version.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hpp>
EXTERNAL_WARNINGS_POP

static args::Group gLoggingGroup{"Logging:"};
static args::Flag  gLogTrace{gLoggingGroup, "trace", "Set log level to trace", {"trace"}};
static args::Flag  gLogVerbose{gLoggingGroup, "verbose", "Set log level to verbose", {"verbose"}};
static args::Flag  gLogDebug{gLoggingGroup, "debug", "Set log level to debug", {"debug"}};
static args::Flag  gLogInfo{gLoggingGroup, "info", "Set log level to info", {"info"}};
static args::Flag  gLogWarning{gLoggingGroup, "warning", "Set log level to warning", {"warning"}};
static args::Flag  gLogError{gLoggingGroup, "error", "Set log level to error", {"error"}};
static args::Flag  gLogNoColor{
    gLoggingGroup, "no-color", "Disable colored output", {"log-no-color"}};
static args::Flag gLogFlush{gLoggingGroup, "flush", "Flush log after each line", {"log-flush"}};

static args::Group gControlGroup{
    "Control:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<uint16_t> gPort{
    gControlGroup, "port", "Control TCP port", {"port"}, args::Options::Single};

static args::ValueFlag<int> gUpdateInterval{gControlGroup,
                                            "update_interval",
                                            "How often the modem should be queried",
                                            {"update"},
                                            args::Options::Single};

static args::Group gModemGroup{
    "Modem:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group gSerialGroup{
    gModemGroup,
    "Serial:",
    args::Group::Validators::DontCare,
    args::Options::Global,
};
static args::ValueFlag<std::string> gSerialDevice{
    gSerialGroup, "device", "Device", {"serial"}, args::Options::Single};
static args::ValueFlag<std::string> gSerialBaudRate{
    gSerialGroup, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
static args::ValueFlag<int> gSerialDataBits{
    gSerialGroup, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
static args::ValueFlag<int> gSerialStopBits{
    gSerialGroup, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> gSerialParityBits{
    gSerialGroup, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

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

static void parse_serial(Config& config) {
    if (!gSerialDevice) {
        throw args::ValidationError("serial-device is required");
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
        default: throw args::ValidationError("invalid serial-data-bits");
        }
    }

    auto stop_bits = io::StopBits::ONE;
    if (gSerialStopBits) {
        switch (gSerialStopBits.Get()) {
        case 1: stop_bits = io::StopBits::ONE; break;
        case 2: stop_bits = io::StopBits::TWO; break;
        default: throw args::ValidationError("invalid serial-stop-bits");
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
            throw args::ValidationError("invalid serial-parity-bits");
        }
    }

    auto device  = gSerialDevice.Get();
    config.input = std::unique_ptr<io::Input>(
        new io::SerialInput(device, baud_rate, data_bits, stop_bits, parity_bit));
    config.output = std::unique_ptr<io::Output>(
        new io::SerialOutput(device, baud_rate, data_bits, stop_bits, parity_bit));
}

Config parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    gPort.HelpDefault("13226");
    gUpdateInterval.HelpDefault("5");

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

    parser.Add(gLoggingGroup);
    parser.Add(gControlGroup);
    parser.Add(gModemGroup);

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "Modem Control " << CLIENT_VERSION << std::endl;
            std::cout << "  Commit: " << GIT_COMMIT_HASH << (GIT_DIRTY ? "-dirty" : "") << " ("
                      << GIT_BRANCH << ")" << std::endl;
            std::cout << "  Built: " << BUILD_DATE << " [" << BUILD_TYPE << "]" << std::endl;
            std::cout << "  Compiler: " << BUILD_COMPILER << std::endl;
            std::cout << "  Platform: " << BUILD_SYSTEM << " (" << BUILD_ARCH << ")" << std::endl;
            exit(0);
        }

        uint16_t port_value = 13226;
        if (gPort) {
            port_value = gPort.Get();
        }

        auto update_interval_value = 5;
        if (gUpdateInterval) {
            update_interval_value = gUpdateInterval.Get();
            if (update_interval_value <= 0) {
                throw args::ValidationError("update-interval must be positive");
            }
        }

        Config config{};
        config.port            = port_value;
        config.update_interval = update_interval_value;

        config.logging.log_level = loglet::Level::Info;
        config.logging.color     = !gLogNoColor;
        config.logging.flush     = gLogFlush;

        if (gLogTrace) {
            config.logging.log_level = loglet::Level::Trace;
        } else if (gLogVerbose) {
            config.logging.log_level = loglet::Level::Verbose;
        } else if (gLogDebug) {
            config.logging.log_level = loglet::Level::Debug;
        } else if (gLogInfo) {
            config.logging.log_level = loglet::Level::Info;
        } else if (gLogWarning) {
            config.logging.log_level = loglet::Level::Warning;
        } else if (gLogError) {
            config.logging.log_level = loglet::Level::Error;
        }

        parse_serial(config);
        return config;
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
