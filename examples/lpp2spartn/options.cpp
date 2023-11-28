#include "options.hpp"
#include <args.hpp>

using namespace interface;

args::Group arguments{"Arguments:"};

//
// Format
//

args::Group format{
    "Options:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::ValueFlag<std::string> format_value{
    format, "format", "Format", {"format"}, args::Options::Single};

args::Flag iode_shift{format, "iode-shift", "IODE Shift", {"iode-shift"}, args::Options::Single};

Format parse_format_options() {
    if (!format_value) {
        throw args::RequiredError("format");
    }

    if (format_value.Get() == "spartn") {
        return Format::SPARTN_NEW;
    } else if (format_value.Get() == "spartn-old") {
        return Format::SPARTN_OLD;
    } else {
        throw args::ValidationError("Invalid format");
    }
}

SpartnOptions parse_spartn_options() {
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
    file_output, "file_path", "Path", {"file"}, args::Options::Single};

args::Group serial_output{
    output,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> serial_device{
    serial_output, "device", "Device", {"serial"}, args::Options::Single};
args::ValueFlag<int> serial_baud_rate{
    serial_output, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
args::ValueFlag<int> serial_data_bits{
    serial_output, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
args::ValueFlag<int> serial_stop_bits{
    serial_output, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
args::ValueFlag<std::string> serial_parity_bits{
    serial_output, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

args::Group i2c_output{
    output,
    "I2C:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> i2c_device{
    i2c_output, "device", "Device", {"i2c"}, args::Options::Single};
args::ValueFlag<uint8_t> i2c_address{
    i2c_output, "address", "Address", {"i2c-address"}, args::Options::Single};

args::Group tcp_output{
    output,
    "TCP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> tcp_ip_address{
    tcp_output, "ip_address", "Host or IP Address", {"tcp"}, args::Options::Single};
args::ValueFlag<int> tcp_port{tcp_output, "port", "Port", {"tcp-port"}, args::Options::Single};

args::Group udp_output{
    output,
    "UDP:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> udp_ip_address{
    udp_output, "ip_address", "Host or IP Address", {"udp"}, args::Options::Single};
args::ValueFlag<int> udp_port{udp_output, "port", "Port", {"udp-port"}, args::Options::Single};

args::Group stdout_output{
    output,
    "Stdout:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::Flag stdout_output_flag{stdout_output, "stdout", "Stdout", {"stdout"}, args::Options::Single};

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

Options parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser("Example - LPP to SPARTN Converter", "");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    format_value.HelpChoices({
        "spartn",
        "spartn-old",
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
        return Options{
            .format = parse_format_options(),
            .output = parse_output_options(),
            .spartn = parse_spartn_options(),
        };
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
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }
}
