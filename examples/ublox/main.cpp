#include <args.hpp>
#include <interface/interface.hpp>
#include <receiver/ublox/decoder.hpp>
#include <receiver/ublox/message.hpp>
#include <receiver/ublox/parser.hpp>
#include <receiver/ublox/receiver.hpp>
#include <receiver/ublox/ubx_mon_ver.hpp>
#include <receiver/ublox/ubx_nav_pvt.hpp>
#include <receiver/ublox/ubx_cfg_valget.hpp>
#include <unistd.h>

args::Group arguments{"Arguments:"};

//
// Configuration
//

args::Group configuration_group{
    "Configuration:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::Flag disable_config{configuration_group,
                          "disable",
                          "Disable configuration",
                          {"disable-config"},
                          args::Options::Single};

args::ValueFlagList<std::string> enable_message(configuration_group, "message", "Enable message",
                                                {"enable-message"}, {}, args::Options::None);

args::ValueFlagList<std::string> disable_message(configuration_group, "message", "Disable message",
                                                 {"disable-message"}, {}, args::Options::None);

//

//
// Receiver
//

args::Group receiver_group{
    "Receiver:",
    args::Group::Validators::AllChildGroups,
    args::Options::Global,
};

args::Flag port_uart1{receiver_group, "uart1", "UART1", {"port-uart1"}, args::Options::Single};
args::Flag port_uart2{receiver_group, "uart2", "UART2", {"port-uart2"}, args::Options::Single};
args::Flag port_i2c{receiver_group, "i2c", "I2C", {"port-i2c"}, args::Options::Single};
args::Flag port_usb{receiver_group, "usb", "USB", {"port-usb"}, args::Options::Single};

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

//
//
//

static void ublox_loop(receiver::ublox::UbloxReceiver& receiver) {
    printf("[ublox]\n");
    printf("  software version: %s\n", receiver.software_version().c_str());
    printf("  hardware version: %s\n", receiver.hardware_version().c_str());
    printf("  extensions:\n");
    for(auto& extension : receiver.extensions()) {
        printf("    %s\n", extension.c_str());
    }
    printf("  spartn support: %s\n", receiver.spartn_support() ? "yes" : "no");

    for (;;) {
        auto message = receiver.wait_for_message();

        usleep(10 * 1000);
    }
}

static void example() {
    if (serial_device) {
        auto baud_rate = 115200;
        if (serial_baud_rate) {
            baud_rate = serial_baud_rate.Get();
        }

        receiver::ublox::Port port;
        if (port_uart1) {
            port = receiver::ublox::Port::UART1;
        } else if (port_uart2) {
            port = receiver::ublox::Port::UART2;
        } else if (port_i2c) {
            port = receiver::ublox::Port::I2C;
        } else if (port_usb) {
            port = receiver::ublox::Port::USB;
        } else {
            throw args::RequiredError(
                "--serial requires a port to be specified. E.g. --port-uart1");
        }

        printf("[serial]\n");
        printf("  device:    %s\n", serial_device.Get().c_str());
        printf("  baud rate: %d\n", baud_rate);
        switch (port) {
        case receiver::ublox::Port::UART1: printf("  port:      UART1\n"); break;
        case receiver::ublox::Port::UART2: printf("  port:      UART2\n"); break;
        case receiver::ublox::Port::I2C: printf("  port:      I2C\n"); break;
        case receiver::ublox::Port::USB: printf("  port:      USB\n"); break;
        default: printf("  port:      Unknown\n"); break;
        }

        auto interface = interface::Interface::serial(
            serial_device.Get(), baud_rate, interface::StopBits::One, interface::ParityBits::None);
        interface->open();

        receiver::ublox::UbloxReceiver receiver(receiver::ublox::Port::UART1, *interface);
        receiver.enable_message(receiver::ublox::MessageId::UbxNavPvt);
        receiver.configure();

        ublox_loop(receiver);
    } else if (i2c_device) {
        auto address = 66;
        if (i2c_address) {
            address = i2c_address.Get();
        }

        auto port = receiver::ublox::Port::I2C;
        if (port_uart1) {
            port = receiver::ublox::Port::UART1;
        } else if (port_uart2) {
            port = receiver::ublox::Port::UART2;
        } else if (port_usb) {
            port = receiver::ublox::Port::USB;
        }

        printf("[i2c]\n");
        printf("  device:  %s\n", i2c_device.Get().c_str());
        printf("  address: %d (0x%02X)\n", address, address);
        switch (port) {
        case receiver::ublox::Port::UART1: printf("  port:      UART1\n"); break;
        case receiver::ublox::Port::UART2: printf("  port:      UART2\n"); break;
        case receiver::ublox::Port::I2C: printf("  port:      I2C\n"); break;
        case receiver::ublox::Port::USB: printf("  port:      USB\n"); break;
        default: printf("  port:      Unknown\n"); break;
        }

        auto interface = interface::Interface::i2c(i2c_device.Get(), address);
        interface->open();

        receiver::ublox::UbloxReceiver receiver(receiver::ublox::Port::UART1, *interface);
        receiver.enable_message(receiver::ublox::MessageId::UbxNavPvt);
        receiver.configure();

        ublox_loop(receiver);
    } else {
        throw args::RequiredError("No interface specified: --serial or --i2c");
    }
}

int main(int argc, char** argv) {
    args::ArgumentParser parser("u-Blox Example", "TODO");

    args::HelpFlag help{parser, "help", "Display this help menu", {'?', "help"}};
    args::Flag     version{parser, "version", "Display version information", {'v', "version"}};

    i2c_address.HelpDefault("66");
    serial_baud_rate.HelpDefault("115200");

    enable_message.HelpChoices({
        "ubx-nav-pvt",
        "ubx-rxm-rawx",
        "ubx-rxm-sfrbx",
        "ubx-rxm-rtcm",
    });

    disable_message.HelpChoices({
        "ubx-nav-pvt",
        "ubx-rxm-rawx",
        "ubx-rxm-sfrbx",
        "ubx-rxm-rtcm",
    });

    args::GlobalOptions configruation_globals{parser, configuration_group};
    args::GlobalOptions receiver_globals{parser, receiver_group};
    args::GlobalOptions interface_globals{parser, interface_group};

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
