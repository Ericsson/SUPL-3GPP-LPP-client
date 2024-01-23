#include <args.hpp>
#include <interface/interface.hpp>
#include <receiver/nmea/receiver.hpp>

using namespace interface;
using namespace receiver::nmea;

args::Group arguments{"Arguments:"};

//
// Interface
//

args::Group interface_group{
    "Interface:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::Group serial_group{
    interface_group,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> serial_device{
    serial_group, "device", "Device", {"serial"}, args::Options::Single};
args::ValueFlag<int> serial_baud_rate{
    serial_group, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};
args::ValueFlag<int> serial_data_bits{
    serial_group, "data_bits", "Data Bits", {"serial-data"}, args::Options::Single};
args::ValueFlag<int> serial_stop_bits{
    serial_group, "stop_bits", "Stop Bits", {"serial-stop"}, args::Options::Single};
args::ValueFlag<std::string> serial_parity_bits{
    serial_group, "parity_bits", "Parity Bits", {"serial-parity"}, args::Options::Single};

args::Group i2c_group{
    interface_group,
    "I2C:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> i2c_device{
    i2c_group, "device", "Device", {"i2c"}, args::Options::Single};
args::ValueFlag<uint8_t> i2c_address{
    i2c_group, "address", "Address", {"i2c-address"}, args::Options::Single};

args::Group stdin_group{
    interface_group,
    "Stdin:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::Flag stdin_device{stdin_group, "", "Enable stdin", {"stdin"}, args::Options::Single};

//
//
//

static std::unique_ptr<Interface> parse_serial() {
    assert(serial_device);

    auto baud_rate = 115200;
    if (serial_baud_rate) {
        baud_rate = serial_baud_rate.Get();
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

    switch (data_bits) {
    case DataBits::FIVE: printf("  data bits:  5\n"); break;
    case DataBits::SIX: printf("  data bits:  6\n"); break;
    case DataBits::SEVEN: printf("  data bits:  7\n"); break;
    case DataBits::EIGHT: printf("  data bits:  8\n"); break;
    }

    switch (stop_bits) {
    case StopBits::ONE: printf("  stop bits:  1\n"); break;
    case StopBits::TWO: printf("  stop bits:  2\n"); break;
    }

    switch (parity_bit) {
    case ParityBit::NONE: printf("  parity bit: none\n"); break;
    case ParityBit::ODD: printf("  parity bit: odd\n"); break;
    case ParityBit::EVEN: printf("  parity bit: even\n"); break;
    }

    return std::unique_ptr<Interface>(
        Interface::serial(serial_device.Get(), baud_rate, data_bits, stop_bits, parity_bit));
}

static std::unique_ptr<Interface> parse_i2c() {
    assert(i2c_device);

    auto address = 66;
    if (i2c_address) {
        address = i2c_address.Get();
    }

    return std::unique_ptr<Interface>(Interface::i2c(i2c_device.Get(), address));
}

static std::unique_ptr<Interface> parse_interface() {
    if (serial_device) {
        return parse_serial();
    } else if (i2c_device) {
        return parse_i2c();
    } else if (stdin_device) {
        return std::unique_ptr<Interface>(Interface::stdin());
    } else {
        throw args::RequiredError("No interface specified: --serial, --i2c, or --stdin");
    }
}

static std::unique_ptr<NmeaReceiver> create_receiver() {
    auto interface = parse_interface();
    interface->open();
    if (!interface->is_open()) {
        throw std::runtime_error("Could not open interface");
    }

    auto receiver = std::unique_ptr<NmeaReceiver>(new NmeaReceiver(std::move(interface)));
    return receiver;
}

std::unique_ptr<NmeaReceiver> parse_configuration(int argc, char** argv) {
    args::ArgumentParser parser(
        "NMEA Example (" CLIENT_VERSION
        ") - This is an example program that demonstrates how to use the NMEA receiver "
        "library. The program takes an interface and port associated with the "
        "receiver as arguments and print all received messages.");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

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

    args::GlobalOptions interface_globals{parser, interface_group};

    try {
        parser.ParseCLI(argc, argv);

        if (version) {
            std::cout << "NMEA Example (" << CLIENT_VERSION << ")" << std::endl;
            exit(0);
        }

        return create_receiver();
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
