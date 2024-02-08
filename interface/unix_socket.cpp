#include "unix_socket.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace interface {

UnixSocketInterface::UnixSocketInterface(std::string path, bool reconnect) IF_NOEXCEPT
    : mPath(std::move(path)),
      mReconnect(reconnect) {}

UnixSocketInterface::~UnixSocketInterface() IF_NOEXCEPT {
    close();
}

void UnixSocketInterface::open() {
    if (mSocket.is_open()) {
        return;
    }

    auto address = NetworkAddress::unix_socket_stream(mPath);
    mSocket      = ReconnectableSocket::connect(address, mReconnect);
    if (!mSocket.is_open()) {
        throw std::runtime_error("Failed to connect to unix socket: '" + mPath + "'");
    }
}

void UnixSocketInterface::close() {
    mSocket.close();
}

size_t UnixSocketInterface::read(void* data, const size_t size) {
    return mSocket.read(data, size);
}

size_t UnixSocketInterface::write(const void* data, const size_t size) {
    return mSocket.write(data, size);
}

bool UnixSocketInterface::can_read() IF_NOEXCEPT {
    return mSocket.can_read();
}

bool UnixSocketInterface::can_write() IF_NOEXCEPT {
    return mSocket.can_write();
}

void UnixSocketInterface::wait_for_read() IF_NOEXCEPT {
    mSocket.wait_for_read();
}

void UnixSocketInterface::wait_for_write() IF_NOEXCEPT {
    mSocket.wait_for_write();
}

bool UnixSocketInterface::is_open() IF_NOEXCEPT {
    return mSocket.is_open();
}

void UnixSocketInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       unix-socket (stream)\n");
    printf("  path:       %s\n", mPath.c_str());
    printf("  reconnect:  %s\n", mReconnect ? "true" : "false");
    mSocket.print_info();
}

//
//
//

Interface* Interface::unix_socket_stream(std::string path, bool reconnect) {
    return new UnixSocketInterface(std::move(path), reconnect);
}

}  // namespace interface
