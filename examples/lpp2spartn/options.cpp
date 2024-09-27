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

using namespace interface;

static args::Group arguments{"Arguments:"};

//
// Format
//

static args::Group format{
    "Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

static args::ValueFlag<std::string> format_value{
    format, "format", "Format", {"format"}, args::Options::Single};

static args::Flag iode_shift{
    format, "iode-shift", "IODE Shift", {"iode-shift"}, args::Options::Single};

static Format parse_format_options() {
    if (!format_value) {
        throw args::RequiredError("format");
    }

    if (format_value.Get() == "xer") {
        return Format::XER;
    }
#ifdef INCLUDE_GENERATOR_SPARTN
    else if (format_value.Get() == "spartn") {
        return Format::SPARTN_NEW;
    }
#endif
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
    else if (format_value.Get() == "spartn-old") {
        return Format::SPARTN_OLD;
    }
#endif
    else {
        throw args::ValidationError("Invalid format");
    }
}

static SpartnOptions parse_spartn_options() {
    SpartnOptions options{};
    options.iode_shift = false;

    if (iode_shift) {
        options.iode_shift = iode_shift.Get();
    }

    return options;
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
                                                      stop_bits, parity_bit, false);
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

Options parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("LPP2SPARTN Example (" CLIENT_VERSION
                                ") - This sample code takes UPER encoded 3GPP LPP messages from "
                                "STDIN and transforms them into SPARTN messages.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    format_value.HelpChoices({
        "xer",
#ifdef INCLUDE_GENERATOR_SPARTN
        "spartn",
#endif
#ifdef INCLUDE_GENERATOR_SPARTN_OLD
        "spartn-old",
#endif
    });

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

    // Globals
    args::GlobalOptions format_globals{parser, format};
    args::GlobalOptions output_globals{parser, output};

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "LPP2SPARTN Example (" CLIENT_VERSION << ")" << std::endl;
            exit(0);
        }

        Options options;
        options.format = parse_format_options();
        options.output = parse_output_options();
        options.spartn = parse_spartn_options();
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
