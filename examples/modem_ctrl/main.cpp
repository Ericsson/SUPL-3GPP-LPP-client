#include "options.hpp"

#include <inttypes.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <interface/interface.hpp>
#include <modem/modem.hpp>

static int start_server(int port) {
    auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    auto socket_addr = reinterpret_cast<struct sockaddr*>(&addr);
    auto socket_size = static_cast<socklen_t>(sizeof(addr));
    if (::bind(socket, socket_addr, socket_size) < 0) {
        perror("bind");
        exit(1);
    }

    int opt = 1;
    if (::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    if (::listen(socket, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("listening on port %d\n", port);
    return socket;
}

[[noreturn]] static void loop(Config& config) {
    printf("[modem-ctrl]\n");

    auto interface = std::move(config.interface);
    interface->open();
    interface->print_info();

    modem::Device device{std::move(interface)};
    device.disable_echo();

    auto modes = device.list_creg();
    if (modes.mode_2) {
        device.set_creg(2);
    } else if (modes.mode_1) {
        device.set_creg(1);
    } else {
        printf("No supported CREG modes\n");
    }

    device.set_cops_format(2);

    auto socket       = start_server(config.port);
    auto send_command = [&](int client, char const* command) {
        printf("SEND: %s\n", command);
        char buffer[1024];
        auto length = snprintf(buffer, sizeof(buffer), "%s\r\n", command);
        auto result = ::send(client, buffer, static_cast<size_t>(length), MSG_NOSIGNAL);
        if (result < 0) {
            return false;
        }

        return true;
    };

    for (;;) {
        printf("-----------------------------------------------------\n");
        auto client = ::accept(socket, nullptr, nullptr);
        if (client < 0) {
            printf("accept failed\n");
            continue;
        }

        device.get_cgmi();
        auto cimi = device.get_cimi();
        char send_buffer[1024];
        snprintf(send_buffer, sizeof(send_buffer), "/IDENTITY,IMSI,%" PRIu64, cimi.imsi);
        if (!send_command(client, send_buffer)) {
            ::close(client);
            continue;
        }

        auto connected = true;
        while (connected) {
            auto cops = device.get_cops();
            auto reg  = device.get_creg();
            if (cops.format == 2) {
                printf("OPERATOR: %d:%d %s\n", cops.mcc, cops.mnc,
                       (cops.act >= 0 && cops.act < 7 ?
                            "GSM" :
                            (cops.act >= 7 && cops.act < 11 ? "LTE" : "UNKNOWN")));
            }
            if (reg.mode != 0) {
                printf("CELL: %d:%d %s\n", reg.lac, reg.ci,
                       (reg.act >= 0 && reg.act < 7 ?
                            "GSM" :
                            (reg.act >= 7 && reg.act < 11 ? "LTE" : "UNKNOWN")));
            }
            if (reg.mode != 0 && cops.format == 2) {
                if (reg.is_gsm()) {
                    snprintf(send_buffer, sizeof(send_buffer), "/CID,G,%d,%d,%d,%d", cops.mcc,
                             cops.mnc, reg.lac, reg.ci);
                } else {
                    snprintf(send_buffer, sizeof(send_buffer), "/CID,L,%d,%d,%d,%d", cops.mcc,
                             cops.mnc, reg.lac, reg.ci);
                }
                if (!send_command(client, send_buffer)) {
                    ::close(client);
                    continue;
                }
            }

            sleep(config.update_interval);
        }

        ::close(client);
    }
}

int main(int argc, char** argv) {
    auto config = parse_configuration(argc, argv);
    loop(config);
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    return 0;
#endif
}
