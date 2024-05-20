#include "options.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#include <args.hpp>
#pragma GCC diagnostic pop

static args::Group control_group{
    "Control:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<int> port{
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
static args::ValueFlag<int> serial_baud_rate{
    serial_group, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
static args::ValueFlag<int> serial_data_bits{
    serial_group, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
static args::ValueFlag<int> serial_stop_bits{
    serial_group, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
static args::ValueFlag<std::string> serial_parity_bits{
    serial_group, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

static std::unique_ptr<interface::Interface> parse_serial() {
    if (!serial_device) {
        throw args::ValidationError("serial-device is required");
    }

    uint32_t baud_rate = 115200;
    if (serial_baud_rate) {
        if (serial_baud_rate.Get() < 0) {
            throw args::ValidationError("serial-baud-rate must be positive");
        }

        baud_rate = static_cast<uint32_t>(serial_baud_rate.Get());
    }

    auto data_bits = interface::DataBits::EIGHT;
    if (serial_data_bits) {
        switch (serial_data_bits.Get()) {
        case 5: data_bits = interface::DataBits::FIVE; break;
        case 6: data_bits = interface::DataBits::SIX; break;
        case 7: data_bits = interface::DataBits::SEVEN; break;
        case 8: data_bits = interface::DataBits::EIGHT; break;
        default: throw args::ValidationError("invalid serial-data-bits");
        }
    }

    auto stop_bits = interface::StopBits::ONE;
    if (serial_stop_bits) {
        switch (serial_stop_bits.Get()) {
        case 1: stop_bits = interface::StopBits::ONE; break;
        case 2: stop_bits = interface::StopBits::TWO; break;
        default: throw args::ValidationError("invalid serial-stop-bits");
        }
    }

    auto parity_bit = interface::ParityBit::NONE;
    if (serial_parity_bits) {
        if (serial_parity_bits.Get() == "none") {
            parity_bit = interface::ParityBit::NONE;
        } else if (serial_parity_bits.Get() == "odd") {
            parity_bit = interface::ParityBit::ODD;
        } else if (serial_parity_bits.Get() == "even") {
            parity_bit = interface::ParityBit::EVEN;
        } else {
            throw args::ValidationError("invalid serial-parity-bits");
        }
    }

    return std::unique_ptr<interface::Interface>(interface::Interface::serial(
        serial_device.Get(), baud_rate, data_bits, stop_bits, parity_bit));
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

    parser.Add(control_group);
    parser.Add(modem_group);

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "Modem Control (" << CLIENT_VERSION << ")" << std::endl;
            exit(0);
        }

        auto port_value = 13226;
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
        config.interface       = parse_serial();
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
