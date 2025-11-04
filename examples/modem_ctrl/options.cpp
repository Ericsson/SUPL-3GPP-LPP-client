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

static args::Group logging_group{"Logging:"};
static args::Flag  log_trace{logging_group, "trace", "Set log level to trace", {"trace"}};
static args::Flag  log_verbose{logging_group, "verbose", "Set log level to verbose", {"verbose"}};
static args::Flag  log_debug{logging_group, "debug", "Set log level to debug", {"debug"}};
static args::Flag  log_info{logging_group, "info", "Set log level to info", {"info"}};
static args::Flag  log_warning{logging_group, "warning", "Set log level to warning", {"warning"}};
static args::Flag  log_error{logging_group, "error", "Set log level to error", {"error"}};
static args::Flag  log_no_color{
    logging_group, "no-color", "Disable colored output", {"log-no-color"}};
static args::Flag log_flush{logging_group, "flush", "Flush log after each line", {"log-flush"}};

static args::Group control_group{
    "Control:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<uint16_t> port{
    control_group, "port", "Control TCP port", {"port"}, args::Options::Single};

static args::ValueFlag<int> update_interval{control_group,
                                            "update_interval",
                                            "How often the modem should be queried",
                                            {"update"},
                                            args::Options::Single};

static args::Group modem_group{
    "Modem:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::Group serial_group{
    modem_group,
    "Serial:",
    args::Group::Validators::DontCare,
    args::Options::Global,
};
static args::ValueFlag<std::string> serial_device{
    serial_group, "device", "Device", {"serial"}, args::Options::Single};
static args::ValueFlag<std::string> serial_baud_rate{
    serial_group, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
static args::ValueFlag<int> serial_data_bits{
    serial_group, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
static args::ValueFlag<int> serial_stop_bits{
    serial_group, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> serial_parity_bits{
    serial_group, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

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

static void parse_serial(Config& config) {
    if (!serial_device) {
        throw args::ValidationError("serial-device is required");
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
        default: throw args::ValidationError("invalid serial-data-bits");
        }
    }

    auto stop_bits = io::StopBits::ONE;
    if (serial_stop_bits) {
        switch (serial_stop_bits.Get()) {
        case 1: stop_bits = io::StopBits::ONE; break;
        case 2: stop_bits = io::StopBits::TWO; break;
        default: throw args::ValidationError("invalid serial-stop-bits");
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
            throw args::ValidationError("invalid serial-parity-bits");
        }
    }

    auto device  = serial_device.Get();
    config.input = std::unique_ptr<io::Input>(
        new io::SerialInput(device, baud_rate, data_bits, stop_bits, parity_bit));
    config.output = std::unique_ptr<io::Output>(
        new io::SerialOutput(device, baud_rate, data_bits, stop_bits, parity_bit));
}

Config parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    port.HelpDefault("13226");
    update_interval.HelpDefault("5");

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

    parser.Add(logging_group);
    parser.Add(control_group);
    parser.Add(modem_group);

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "Modem Control (" << CLIENT_VERSION << ")" << std::endl;
            exit(0);
        }

        uint16_t port_value = 13226;
        if (port) {
            port_value = port.Get();
        }

        auto update_interval_value = 5;
        if (update_interval) {
            update_interval_value = update_interval.Get();
            if (update_interval_value <= 0) {
                throw args::ValidationError("update-interval must be positive");
            }
        }

        Config config{};
        config.port            = port_value;
        config.update_interval = update_interval_value;

        config.logging.log_level = loglet::Level::Info;
        config.logging.color     = !log_no_color;
        config.logging.flush     = log_flush;

        if (log_trace) {
            config.logging.log_level = loglet::Level::Trace;
        } else if (log_verbose) {
            config.logging.log_level = loglet::Level::Verbose;
        } else if (log_debug) {
            config.logging.log_level = loglet::Level::Debug;
        } else if (log_info) {
            config.logging.log_level = loglet::Level::Info;
        } else if (log_warning) {
            config.logging.log_level = loglet::Level::Warning;
        } else if (log_error) {
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
