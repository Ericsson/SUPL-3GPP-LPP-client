#include "reconnectable_socket.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#ifdef INTERFACE_SOCKET_DEBUG
#include <cstdio>
#define SOCKET_DEBUG(...) printf(__VA_ARGS__)
#else
#define SOCKET_DEBUG(...)
#endif

namespace interface {

ReconnectableSocket::ReconnectableSocket() IF_NOEXCEPT : mShouldReconnect(false) {}

ReconnectableSocket::ReconnectableSocket(int file_descriptor, NetworkAddress address,
                                         bool should_reconnect) IF_NOEXCEPT
    : mSocket(file_descriptor),
      mShouldReconnect(should_reconnect),
      mAddress(address) {
    mSocket.set_non_blocking(true);
}

void ReconnectableSocket::close() IF_NOEXCEPT {
    mSocket.close();
}

bool ReconnectableSocket::can_read() IF_NOEXCEPT {
    try_reconnect();
    return mSocket.can_read();
}

bool ReconnectableSocket::can_write() IF_NOEXCEPT {
    try_reconnect();
    return mSocket.can_write();
}

bool ReconnectableSocket::wait_for_read() IF_NOEXCEPT {
    try_reconnect();
    return mSocket.wait_for_read();
}

bool ReconnectableSocket::wait_for_write() IF_NOEXCEPT {
    try_reconnect();
    return mSocket.wait_for_write();
}

size_t ReconnectableSocket::read(void* data, size_t length) IF_NOEXCEPT {
    try_reconnect();
    return mSocket.read(data, length);
}

size_t ReconnectableSocket::write(const void* data, size_t length) IF_NOEXCEPT {
    try_reconnect();
    return mSocket.write(data, length);
}

bool ReconnectableSocket::is_open() const IF_NOEXCEPT {
    return mSocket.is_open();
}

void ReconnectableSocket::try_reconnect() IF_NOEXCEPT {
    if (mSocket.is_open()) {
        return;
    }

    if (!mShouldReconnect) {
        return;
    }

    SOCKET_DEBUG("[rcsock/%6d] new socket...\n", -1);
    auto fd = ::socket(mAddress.family(), mAddress.type(), mAddress.protocol());
    if (fd < 0) {
        SOCKET_DEBUG("[rcsock/%6d]   failed=%d\n", fd, errno);
        return;
    }

    SOCKET_DEBUG("[rcsock/%6d] connecting...\n", fd);
    auto ret = ::connect(fd, mAddress.ptr(), mAddress.length());
    if (ret < 0) {
        SOCKET_DEBUG("[rcsock/%6d]   failed=%d\n", fd, errno);
        ::close(fd);
        return;
    }

    SOCKET_DEBUG("[socket/%6d] connected\n", fd);

    mSocket = Socket(fd);
    mSocket.set_non_blocking(true);
}

void ReconnectableSocket::print_info() IF_NOEXCEPT {
    printf("  [socket]\n");
    switch (mAddress.family()) {
    case AF_INET: printf("    family:     AF_INET\n"); break;
    case AF_INET6: printf("    family:     AF_INET6\n"); break;
    case AF_UNIX: printf("    family:     AF_UNIX\n"); break;
    default: printf("    family:     AF_??? (%d)\n", mAddress.family()); break;
    }
    switch (mAddress.type()) {
    case SOCK_STREAM: printf("    type:       SOCK_STREAM\n"); break;
    case SOCK_DGRAM: printf("    type:       SOCK_DGRAM\n"); break;
    default: printf("    type:       SOCK_??? (%d)\n", mAddress.type()); break;
    }
    switch (mAddress.protocol()) {
    case IPPROTO_TCP: printf("    protocol:   IPPROTO_TCP\n"); break;
    case IPPROTO_UDP: printf("    protocol:   IPPROTO_UDP\n"); break;
    case IPPROTO_IP: printf("    protocol:   IPPROTO_IP\n"); break;
    default: printf("    protocol:   IPPROTO_??? (%d)\n", mAddress.protocol()); break;
    }
    printf("    address:    %s\n", mAddress.to_string().c_str());
    if (mSocket.is_open()) {
        printf("    fd:         %d\n", mSocket.socket());
    } else {
        printf("    fd:         closed\n");
    }
}

ReconnectableSocket ReconnectableSocket::connect(NetworkAddress address,
                                                 bool           should_reconnect) IF_NOEXCEPT {
    SOCKET_DEBUG("[rcsock/%6d] new socket...\n", -1);
    auto fd = ::socket(address.family(), address.type(), address.protocol());
    if (fd < 0) {
        SOCKET_DEBUG("[rcsock/%6d]   failed=%d\n", fd, errno);
        return ReconnectableSocket();
    }

    SOCKET_DEBUG("[rcsock/%6d] connecting...\n", fd);
    auto ret = ::connect(fd, address.ptr(), address.length());
    if (ret < 0) {
        SOCKET_DEBUG("[rcsock/%6d]   failed=%d\n", fd, errno);
        ::close(fd);
        return ReconnectableSocket();
    }

    return ReconnectableSocket(fd, address, should_reconnect);
}

}  // namespace interface
