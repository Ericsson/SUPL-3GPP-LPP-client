#include "udp.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace interface {

UdpInterface::UdpInterface(std::string host, uint16_t port, bool reconnect) IF_NOEXCEPT
    : mHost(std::move(host)),
      mPort(port),
      mReconnect(reconnect) {}

UdpInterface::~UdpInterface() IF_NOEXCEPT {
    close();
}

void UdpInterface::open() {
    if (mSocket.is_open()) {
        return;
    }

    struct addrinfo hints {};
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;

    auto service = std::to_string(mPort);

    struct addrinfo* result;
    auto             error = getaddrinfo(mHost.c_str(), service.c_str(), &hints, &result);
    if (error != 0) {
        throw std::runtime_error("Failed to get address info");
    }

    for (auto* rp = result; rp != nullptr; rp = rp->ai_next) {
        auto address = NetworkAddress::from_addrinfo(rp);
        mSocket      = ReconnectableSocket::connect(address, mReconnect);
        if (mSocket.is_open()) {
            return;
        }
    }

    throw std::runtime_error("Failed to connect to host");
}

void UdpInterface::close() {
    mSocket.close();
}

size_t UdpInterface::read(void* data, const size_t size) {
    return mSocket.read(data, size);
}

size_t UdpInterface::write(const void* data, const size_t size) {
    return mSocket.write(data, size);
}

bool UdpInterface::can_read() IF_NOEXCEPT {
    return mSocket.can_read();
}

bool UdpInterface::can_write() IF_NOEXCEPT {
    return mSocket.can_write();
}

void UdpInterface::wait_for_read() IF_NOEXCEPT {
    mSocket.wait_for_read();
}

void UdpInterface::wait_for_write() IF_NOEXCEPT {
    mSocket.wait_for_write();
}

bool UdpInterface::is_open() IF_NOEXCEPT {
    return mSocket.is_open();
}

void UdpInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       udp\n");
    printf("  host:       %s\n", mHost.c_str());
    printf("  port:       %u\n", mPort);
    printf("  reconnect:  %s\n", mReconnect ? "true" : "false");
    mSocket.print_info();
}

//
//
//

Interface* Interface::udp(std::string host, uint16_t port, bool reconnect) {
    return new UdpInterface(std::move(host), port, reconnect);
}

}  // namespace interface
