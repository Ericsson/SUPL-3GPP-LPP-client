#include <args.hpp>
#include <interface/interface.hpp>
#include <receiver/ublox/decoder.hpp>
#include <receiver/ublox/parser.hpp>
#include <receiver/ublox/message.hpp>

args::Group arguments{"Arguments:"};

//
// Receiver
//

args::Group receiver_group{
    "Receiver:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::Group serial_group{
    receiver_group,
    "Serial:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> serial_device{
    serial_group, "device", "Device", {"serial"}, args::Options::Single};
args::ValueFlag<int> serial_baud_rate{
    serial_group, "baud_rate", "Baud Rate", {"serial-baud"}, args::Options::Single};

args::Group i2c_group{
    receiver_group,
    "I2C:",
    args::Group::Validators::AllOrNone,
    args::Options::Global,
};
args::ValueFlag<std::string> i2c_device{
    i2c_group, "device", "Device", {"i2c"}, args::Options::Single};
args::ValueFlag<uint8_t> i2c_address{
    i2c_group, "address", "Address", {"i2c-address"}, args::Options::Single};

//
//
//

static void ublox_loop(interface::Interface& interface) {
    auto parser = receiver::ublox::Parser();

    while (true) {
        uint8_t buffer[1024];
        auto    length = interface.read(buffer, sizeof(buffer));
        if (length <= 0) {
            throw std::runtime_error("Failed to read from interface");
        }

        if (!parser.append(buffer, length)) {
            throw std::runtime_error("Failed to append to parser");
        }

        auto message = parser.try_parse();
        if (message) {
            message->print();
        }
    }
}

static void example() {
    if (serial_device) {
        auto baud_rate = 115200;
        if (serial_baud_rate) {
            baud_rate = serial_baud_rate.Get();
        }

        printf("[serial]\n");
        printf("  device:    %s\n", serial_device.Get().c_str());
        printf("  baud rate: %d\n", baud_rate);

        auto interface = interface::Interface::serial(
            serial_device.Get(), baud_rate, interface::StopBits::One, interface::ParityBits::None);
        ublox_loop(*interface);
        delete interface;
    } else if (i2c_device) {
        auto address = 66;
        if (i2c_address) {
            address = i2c_address.Get();
        }

        printf("[i2c]\n");
        printf("  device:  %s\n", i2c_device.Get().c_str());
        printf("  address: %d (0x%02X)\n", address, address);

        auto interface = interface::Interface::i2c(i2c_device.Get(), address);
        ublox_loop(*interface);
        delete interface;
    } else {
        throw args::RequiredError("No receiver specified: --serial or --i2c");
    }
}

int main(int argc, char** argv) {
    args::ArgumentParser parser("u-Blox Example", "TODO");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    i2c_address.HelpDefault("66");
    serial_baud_rate.HelpDefault("115200");

    args::GlobalOptions receiver_globals{parser, receiver_group};

    try {
        parser.ParseCLI(argc, argv);
        example();
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

    return EXIT_SUCCESS;
}
