#include "options.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

[[noreturn]] static void loop(Config& config) {
    printf("[ctrl-switch]\n");

    // Open TCP server
    auto socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        perror("socket");
        exit(1);
    }

    // Bind to port
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(13226);
    addr.sin_addr.s_addr = INADDR_ANY;

    auto socket_addr = reinterpret_cast<struct sockaddr*>(&addr);
    auto socket_size = static_cast<socklen_t>(sizeof(addr));
    if (::bind(socket, socket_addr, socket_size) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for connections
    if (::listen(socket, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("listening on port 13226\n");

    // Accept connections
    for (;;) {
        printf("-----------------------------------------------------\n");
        auto client = ::accept(socket, nullptr, nullptr);
        if (client < 0) {
            printf("accept failed\n");
            exit(1);
        }

        printf("client connected:\n");

        auto connected = true;
        while (connected) {
            for (auto& command : config.commands) {
                printf("  %s\n", command.c_str());

                char buffer[1024];
                auto length = snprintf(buffer, sizeof(buffer), "%s\r\n", command.c_str());
                auto result = ::send(client, buffer, static_cast<size_t>(length), MSG_NOSIGNAL);
                if (result < 0) {
                    printf("write failed\n");
                    connected = false;
                    break;
                }

                usleep(1000 * 5000);
            }
        }

        // Close client
        printf("client disconnected\n");
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
