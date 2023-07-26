#include "udp.h"

#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

UdpTarget::UdpTarget(std::string ip_address, const int port)
    : mIpAddress(std::move(ip_address)), mPort(port) {
    mSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(mIpAddress.c_str());
    server.sin_family      = AF_INET;
    server.sin_port        = htons(mPort);

    if (connect(mSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        throw std::runtime_error("Failed to connect to server");
    }
}

UdpTarget::~UdpTarget() {
    if (mSocket >= 0) {
        close(mSocket);
    }
}

void UdpTarget::transmit(const void* data, const size_t size) {
    if (mSocket < 0) {
        throw std::runtime_error("Socket not open");
    }

    // To support reconnecting / starting the client before the server, we
    // don't check the return value of write. If the server is not running,
    // the write will fail and we'll just drop the data.
    write(mSocket, data, size);
}
