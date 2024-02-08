#pragma once
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include "types.hpp"

namespace interface {

/// Helper class to manage socket address.
class NetworkAddress {
public:
    static NetworkAddress from_addrinfo(const addrinfo* addr) {
        switch (addr->ai_family) {
        case AF_INET: break;
        case AF_INET6: break;
        case AF_UNIX: break;
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

    static NetworkAddress unix_socket_stream(const std::string& path) {
        if (path.size() >= sizeof(sockaddr_un::sun_path)) throw std::runtime_error("Path too long");

        NetworkAddress rtrn{};
        rtrn.mFamily             = AF_UNIX;
        rtrn.mType               = SOCK_STREAM;
        rtrn.mProtocol           = 0;
        rtrn.mAddr.un.sun_family = AF_UNIX;
        strncpy(rtrn.mAddr.un.sun_path, path.c_str(), sizeof(rtrn.mAddr.un.sun_path));
        return rtrn;
    }

    IF_NODISCARD struct sockaddr* ptr() IF_NOEXCEPT { return &mAddr.base; }

    IF_NODISCARD std::size_t length() const IF_NOEXCEPT {
        switch (mFamily) {
        case AF_INET: return sizeof(mAddr.in4);
        case AF_INET6: return sizeof(mAddr.in6);
        case AF_UNIX: return sizeof(mAddr.un);
        default: return 0; /* fail safe in later call */
        }
    }

    IF_NODISCARD int family() const IF_NOEXCEPT { return mFamily; }
    IF_NODISCARD int type() const IF_NOEXCEPT { return mType; }
    IF_NODISCARD int protocol() const IF_NOEXCEPT { return mProtocol; }

    IF_NODISCARD std::string to_string() const IF_NOEXCEPT {
        char buffer[INET6_ADDRSTRLEN];
        switch (mFamily) {
        case AF_INET:
            inet_ntop(mFamily, &mAddr.in4.sin_addr, buffer, INET_ADDRSTRLEN);
            return std::string(buffer) + ":" + std::to_string(ntohs(mAddr.in4.sin_port));
        case AF_INET6:
            inet_ntop(mFamily, &mAddr.in6.sin6_addr, buffer, INET6_ADDRSTRLEN);
            return std::string(buffer) + ":" + std::to_string(ntohs(mAddr.in6.sin6_port));
        case AF_UNIX: return std::string(mAddr.un.sun_path);
        default: return "Unsupported network address";
        }
    }

private:
    int mFamily;
    int mType;
    int mProtocol;

    union {
        sockaddr     base;
        sockaddr_in  in4;
        sockaddr_in6 in6;
        sockaddr_un  un;
    } mAddr;
};

/// Helper class to manage socket.
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
    IF_NODISCARD int   socket() const IF_NOEXCEPT { return mSocket; }

    void set_non_blocking(bool non_blocking) IF_NOEXCEPT;

private:
    IF_NODISCARD bool select(bool read, bool write, bool except, timeval* tv) IF_NOEXCEPT;
    void              set_error(Error error) IF_NOEXCEPT;

    int   mSocket;
    Error mError;
};

}  // namespace interface
