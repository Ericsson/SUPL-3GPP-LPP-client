#pragma once
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/time.h>
#include "types.hpp"

namespace interface {

/// @brief Helper class to manage socket address.
class NetworkAddress {
public:
    static NetworkAddress from_addrinfo(const addrinfo* addr) {
        switch (addr->ai_family) {
        case AF_INET: break;
        case AF_INET6: break;
        default: throw std::runtime_error("Unsupported network address");
        }
        NetworkAddress rtrn{};
        rtrn.mFamily   = addr->ai_family;
        rtrn.mType     = addr->ai_socktype;
        rtrn.mProtocol = addr->ai_protocol;
        if (addr->ai_addrlen <= sizeof(rtrn.mAddr))
            memcpy(&rtrn.mAddr, addr->ai_addr, addr->ai_addrlen);
        else
            throw std::runtime_error("Unsupported network address");
        return rtrn;
    }

    IF_NODISCARD struct sockaddr* ptr() IF_NOEXCEPT { return &mAddr.base; }

    IF_NODISCARD std::size_t length() const IF_NOEXCEPT {
        switch (mFamily) {
        case AF_INET: return sizeof(mAddr.in4);
        case AF_INET6: return sizeof(mAddr.in6);
        default: return 0; /* fail safe in later call */
        }
    }

    IF_NODISCARD int family() const IF_NOEXCEPT { return mFamily; }
    IF_NODISCARD int type() const IF_NOEXCEPT { return mType; }
    IF_NODISCARD int protocol() const IF_NOEXCEPT { return mProtocol; }

private:
    int mFamily;
    int mType;
    int mProtocol;

    union {
        sockaddr     base;
        sockaddr_in  in4;
        sockaddr_in6 in6;
    } mAddr;
};

/// @brief Helper class to manage socket.
class Socket {
public:
    IF_EXPLICIT Socket() IF_NOEXCEPT;
    IF_EXPLICIT Socket(int socket) IF_NOEXCEPT;
    ~Socket() IF_NOEXCEPT;

    Socket(const Socket&)            = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) IF_NOEXCEPT;
    Socket& operator=(Socket&& other) IF_NOEXCEPT;

    void close() IF_NOEXCEPT;

    IF_NODISCARD bool can_read() IF_NOEXCEPT;
    IF_NODISCARD bool can_write() IF_NOEXCEPT;

    bool wait_for_read() IF_NOEXCEPT;
    bool wait_for_write() IF_NOEXCEPT;

    IF_NODISCARD size_t read(void* data, size_t length) IF_NOEXCEPT;
    IF_NODISCARD size_t write(const void* data, size_t length) IF_NOEXCEPT;

    enum Error {
        NONE                = 0,
        BAD_FILE_DESCRIPTOR = 1,
        READ                = 2,
        WRITE               = 3,
        SELECT              = 4,
    };

    IF_NODISCARD Error error() const IF_NOEXCEPT;
    IF_NODISCARD bool  is_open() const IF_NOEXCEPT;

    void set_non_blocking(bool non_blocking) IF_NOEXCEPT;
private:
    IF_NODISCARD bool select(bool read, bool write, bool except, timeval* tv) IF_NOEXCEPT;
    void              set_error(Error error) IF_NOEXCEPT;

    int   mSocket;
    Error mError;
};

}  // namespace interface
