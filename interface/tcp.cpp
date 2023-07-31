#include "tcp.h"

#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>

TcpTarget::TcpTarget(std::string ip_address, const int port)
    : mIpAddress(std::move(ip_address)), mPort(port) {
    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
}

TcpTarget::~TcpTarget() {
    if (mSocket >= 0) {
        close(mSocket);
    }
}

void TcpTarget::connect() {
    mConnected = false;

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(mIpAddress.c_str());
    server.sin_family      = AF_INET;
    server.sin_port        = htons(mPort);

    if (::connect(mSocket, (struct sockaddr*)&server, sizeof(server)) == 0) {
        mConnected = true;
    }
}

void TcpTarget::transmit(const void* data, const size_t size) {
    if (mSocket < 0) {
        throw std::runtime_error("Socket not open");
    }

    if (!mConnected) {
        connect();
    }

    if (mConnected) {
        if (write(mSocket, data, size) != size) {
            mConnected = false;
        }
    }
}
