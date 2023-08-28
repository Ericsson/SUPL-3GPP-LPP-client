#include "options.hpp"
#include <receiver/ublox/receiver.hpp>
#include <receiver/ublox/message.hpp>
#include <unistd.h>

using namespace receiver::ublox;

static void ublox_loop(UbloxReceiver& receiver) {
    printf("[ublox]\n");
    printf("  software version: %s\n", receiver.software_version().c_str());
    printf("  hardware version: %s\n", receiver.hardware_version().c_str());
    printf("  extensions:\n");
    for (auto& extension : receiver.extensions()) {
        printf("    %s\n", extension.c_str());
    }
    printf("  spartn support: %s\n", receiver.spartn_support() ? "yes" : "no");
    printf("-----------------------------------------------------\n");

    for (;;) {
        // Wait for the next ubx message.
        auto message = receiver.wait_for_message();
        if (message) {
            message->print();
        }

        usleep(10 * 1000);
    }
}

int main(int argc, char** argv) {
    auto receiver = parse_configuration(argc, argv);
    ublox_loop(*receiver.get());
    return 0;
}
