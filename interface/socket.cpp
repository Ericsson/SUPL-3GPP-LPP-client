#include "socket.hpp"
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

Socket::Socket() IF_NOEXCEPT : mSocket(-1), mError(NONE) {
    SOCKET_DEBUG("[socket/%6d] created\n", mSocket);
}

Socket::Socket(int file_descriptor) IF_NOEXCEPT : mSocket(file_descriptor), mError(NONE) {
    SOCKET_DEBUG("[socket/%6d] created\n", mSocket);
}

Socket::~Socket() IF_NOEXCEPT {
    close();
}

Socket::Socket(Socket&& other) IF_NOEXCEPT {
    SOCKET_DEBUG("[socket/%6d] move created\n", other.mSocket);

    mSocket       = other.mSocket;
    mError        = other.mError;
    other.mSocket = -1;
    other.mError  = NONE;
}

Socket& Socket::operator=(Socket&& other) IF_NOEXCEPT {
    SOCKET_DEBUG("[socket/%6d] move assigned from [socket/%6d]\n", mSocket, other.mSocket);
    close();

    mSocket       = other.mSocket;
    mError        = other.mError;
    other.mSocket = -1;
    other.mError  = NONE;
    return *this;
}

void Socket::close() IF_NOEXCEPT {
    SOCKET_DEBUG("[socket/%6d] closed\n", mSocket);
    if (mSocket >= 0) {
        ::close(mSocket);
        mSocket = -1;
    }
}

bool Socket::can_read() IF_NOEXCEPT {
    struct timeval tv = {0, 0};
    return select(true, false, false, &tv);
}

bool Socket::can_write() IF_NOEXCEPT {
    struct timeval tv = {0, 0};
    return select(false, true, false, &tv);
}

bool Socket::wait_for_read() IF_NOEXCEPT {
    return select(true, false, false, nullptr);
}

bool Socket::wait_for_write() IF_NOEXCEPT {
    return select(false, true, false, nullptr);
}

bool Socket::select(bool read, bool write, bool except, timeval* tv) IF_NOEXCEPT {
    if (!is_open()) {
        SOCKET_DEBUG("[socket/%6d] select(%i,%i,%i,%f) = false (socket not open)\n", mSocket, read,
                     write, except, tv ? (tv->tv_sec + tv->tv_usec / 1000000.0) : 0.0);
        return false;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mSocket, &fds);

    auto read_fds   = read ? &fds : nullptr;
    auto write_fds  = write ? &fds : nullptr;
    auto except_fds = except ? &fds : nullptr;

    auto ret = ::select(mSocket + 1, read_fds, write_fds, except_fds, tv);
    SOCKET_DEBUG("[socket/%6d] select(%i,%i,%i,%f) = %d%s\n", mSocket, read_fds != nullptr,
                 write_fds != nullptr, except_fds != nullptr,
                 tv ? (tv->tv_sec + tv->tv_usec / 1000000.0) : 0.0, ret, tv ? " (timeout)" : "");
    if (ret < 0) {
        SOCKET_DEBUG("[socket/%6d]   failed=%d\n", mSocket, errno);
        set_error(SELECT);
        return false;
    }

    return ret > 0;
}

size_t Socket::read(void* data, size_t length) IF_NOEXCEPT {
    if (!is_open()) {
        SOCKET_DEBUG("[socket/%6d] recv(%zu bytes) = 0 (socket not open)\n", mSocket, length);
        return 0;
    }

    auto bytes_read = ::recv(mSocket, data, length, MSG_NOSIGNAL);
    SOCKET_DEBUG("[socket/%6d] recv(%zu bytes) = %zd\n", mSocket, length, bytes_read);
    if (bytes_read < 0) {
        SOCKET_DEBUG("[socket/%6d]   failed=%d\n", mSocket, errno);
        set_error(READ);
        return 0;
    }

    return bytes_read;
}

size_t Socket::write(const void* data, size_t length) IF_NOEXCEPT {
    if (!is_open()) {
        SOCKET_DEBUG("[socket/%6d] send(%zu bytes) = 0 (socket not open)\n", mSocket, length);
        return 0;
    }

    auto bytes_written = ::send(mSocket, data, length, MSG_NOSIGNAL);
    SOCKET_DEBUG("[socket/%6d] send(%zu bytes) = %zd\n", mSocket, length, bytes_written);
    if (bytes_written < 0) {
        SOCKET_DEBUG("[socket/%6d]   failed=%d\n", mSocket, errno);
        set_error(WRITE);
        return 0;
    }

    return bytes_written;
}

Socket::Error Socket::error() const IF_NOEXCEPT {
    return mError;
}

bool Socket::is_open() const IF_NOEXCEPT {
    return mSocket >= 0;
}

void Socket::set_non_blocking(bool non_blocking) IF_NOEXCEPT {
    if (!is_open()) {
        return;
    }

    auto flags = fcntl(mSocket, F_GETFL, 0);
    if (flags < 0) {
        return;
    }

    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    fcntl(mSocket, F_SETFL, flags);
}

void Socket::set_error(Error error) IF_NOEXCEPT {
    SOCKET_DEBUG("[socket/%6d] error=%d\n", mSocket, error);
    mError = error;
    close();
}

}  // namespace interface