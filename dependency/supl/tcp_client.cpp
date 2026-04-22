#include "tcp_client.hpp"
#include "supl.hpp"

#include <cstring>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(supl, tcp);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, tcp)

namespace supl {

TcpClient::TcpClient() : mSocket(-1), mState(State::DISCONNECTED) {
    VSCOPE_FUNCTION();
}

TcpClient::~TcpClient() {
    VSCOPE_FUNCTION();
    disconnect();
    unitialize_socket();
}

void TcpClient::unitialize_socket() {
    VSCOPE_FUNCTION();

    if (mTls) {
        mTls->shutdown();
        mTls.reset();
    }
    mTlsStarted = false;

    if (mSocket >= 0) {
        auto result = shutdown(mSocket, SHUT_RDWR);
        VERBOSEF("::shutdown(%d, SHUT_RDWR) = %d", mSocket, result);

        VERBOSEF("closing socket %d", mSocket);
        close(mSocket);
        VERBOSEF("::close(%d)", mSocket);
        mSocket = -1;
    }
}

static std::string addr_to_string(const struct sockaddr* addr, socklen_t addrlen) {
    VSCOPE_FUNCTION();
    if (!addr) {
        return "null";
    }

    char host[1024];
    host[0]     = '\0';
    auto result = ::getnameinfo(addr, addrlen, host, sizeof(host), nullptr, 0, NI_NUMERICHOST);
    VERBOSEF("::getnameinfo(%p, %u, %p, %zu, %p, %d, NI_NUMERICHOST) = %d", addr, addrlen, host,
             sizeof(host), nullptr, 0, result);
    if (result != 0) {
        return "unknown";
    }
    host[sizeof(host) - 1] = '\0';
    return std::string(host);
}

bool TcpClient::initialize_socket() {
    VSCOPE_FUNCTION();

    unitialize_socket();

    char port_as_string[8];
    snprintf(port_as_string, sizeof(port_as_string), "%i", mPort);
    VERBOSEF("connecting to %s:%s", mHost.c_str(), port_as_string);

    struct addrinfo hint;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    struct addrinfo *ailist, *aip;
    auto             result = ::getaddrinfo(mHost.c_str(), port_as_string, &hint, &ailist);
    VERBOSEF("::getaddrinfo(\"%s\", \"%s\", %p, %p) = %d", mHost.c_str(), port_as_string, &hint,
             &ailist, result);
    if (result != 0) {
        WARNF("failed to get address info: \"%s\" %d (%s)", mHost.c_str(), result,
              gai_strerror(result));
        return false;
    }

    SUPL_DEFER {
        ::freeaddrinfo(ailist);
        VERBOSEF("::freeaddrinfo(%p)", ailist);
    };

    for (aip = ailist; aip; aip = aip->ai_next) {
        if (!aip->ai_addr || aip->ai_addrlen == 0) {
            WARNF("invalid address info: addr=%p addrlen=%u", aip->ai_addr, aip->ai_addrlen);
            continue;
        }

        auto fd = socket(aip->ai_family, aip->ai_socktype | SOCK_NONBLOCK, aip->ai_protocol);
        VERBOSEF("::socket(%d, %d | SOCK_NONBLOCK, %d) = %d", aip->ai_family, aip->ai_socktype,
                 aip->ai_protocol, fd);
        if (fd < 0) {
            WARNF("failed to create socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            continue;
        }

        if (mInterface.length() > 0) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, mInterface.c_str(), IFNAMSIZ - 1);
            ifr.ifr_name[IFNAMSIZ - 1] = '\0';
            auto set_result            = ::setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
                                                      reinterpret_cast<void*>(&ifr), sizeof(ifr));
            if (set_result < 0) {
                WARNF("failed to bind socket to interface \"%s\": " ERRNO_FMT, mInterface.c_str(),
                      ERRNO_ARGS(errno));
                ::close(fd);
                VERBOSEF("::close(%d)", fd);
                continue;
            }
        }

        VERBOSEF("created socket %d", fd);

        auto addr_str    = addr_to_string(aip->ai_addr, aip->ai_addrlen);
        result           = ::connect(fd, aip->ai_addr, aip->ai_addrlen);
        auto saved_errno = errno;
        VERBOSEF("::connect(%d, %p, %u) = %d", fd, aip->ai_addr, aip->ai_addrlen, result);
        if (result != 0) {
            if (saved_errno == EINPROGRESS) {
                VERBOSEF("connection inprogress to %s", addr_str.c_str());
                mSocket = fd;
                return true;
            }

            WARNF("failed to connect to %s: " ERRNO_FMT, addr_str.c_str(), ERRNO_ARGS(saved_errno));
            VERBOSEF("closing socket %d", fd);
            ::close(fd);
            VERBOSEF("::close(%d)", fd);
            continue;
        } else {
            mSocket = fd;
            return true;
        }
    }

    WARNF("no connection could be established to any address");
    return false;
}

bool TcpClient::connect(std::string const& host, int port, std::string const& interface,
                        TlsConfig const& tls) {
    VSCOPE_FUNCTION();
    if (!is_disconnected()) {
        DEBUGF("client is already connected");
        return false;
    }

    mHost      = host;
    mPort      = port;
    mInterface = interface;

    if (!initialize_socket()) {
        return false;
    }

    mTls = create_tls_backend(tls);
    if (tls.enabled && !mTls) {
        WARNF("TLS requested but no TLS backend available");
        unitialize_socket();
        return false;
    }

    // set socket keepalive options
    int enable       = 1;
    int idle         = 1;     // First probe after 1 second idle
    int interval     = 2;     // Subsequent probes every 2 seconds
    int count        = 3;     // 3 probes before giving up
    int user_timeout = 8000;  // 8 seconds total timeout

    auto result = setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
    VERBOSEF("::setsockopt(%d, SOL_SOCKET, SO_KEEPALIVE, *%p=%d, %zu) = %d", mSocket, &enable,
             enable, sizeof(enable), result);

    // Configure keep-alive parameters
    result = setsockopt(mSocket, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
    VERBOSEF("::setsockopt(%d, IPPROTO_TCP, TCP_KEEPIDLE, *%p=%d, %zu) = %d", mSocket, &idle, idle,
             sizeof(idle), result);

    result = setsockopt(mSocket, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    VERBOSEF("::setsockopt(%d, IPPROTO_TCP, TCP_KEEPINTVL, *%p=%d, %zu) = %d", mSocket, &interval,
             interval, sizeof(interval), result);

    result = setsockopt(mSocket, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
    VERBOSEF("::setsockopt(%d, IPPROTO_TCP, TCP_KEEPCNT, *%p=%d, %zu) = %d", mSocket, &count, count,
             sizeof(count), result);

    result =
        setsockopt(mSocket, IPPROTO_TCP, TCP_USER_TIMEOUT, &user_timeout, sizeof(user_timeout));
    VERBOSEF("::setsockopt(%d, IPPROTO_TCP, TCP_USER_TIMEOUT, *%p=%d, %zu) = %d", mSocket,
             &user_timeout, user_timeout, sizeof(user_timeout), result);

    mState = State::CONNECTING;
    return true;
}

TcpClient::Progress TcpClient::handle_connection() {
    VSCOPE_FUNCTION();
    if (is_connected()) {
        return Progress::Done;
    }
    if (!is_connecting()) {
        return Progress::Failed;
    }

    // Check TCP-level connection result only once (when we haven't started TLS yet)
    if (!mTls || !mTlsStarted) {
        int       error  = 0;
        socklen_t len    = sizeof(error);
        auto      result = ::getsockopt(mSocket, SOL_SOCKET, SO_ERROR, &error, &len);
        VERBOSEF("::getsockopt(%d, SOL_SOCKET, SO_ERROR, %p, %p) = %d", mSocket, &error, &len,
                 result);
        if (result < 0 || error != 0) {
            disconnect();
            return Progress::Failed;
        }

        if (!mTls) {
            VERBOSEF("connected to %s:%d", mHost.c_str(), mPort);
            mState = State::CONNECTED;
            return Progress::Done;
        }
        mTlsStarted = true;
    }

    auto r = mTls->handshake(mSocket, mHost);
    switch (r) {
    case TlsBackend::HandshakeResult::Ok:
        VERBOSEF("TLS connected to %s:%d", mHost.c_str(), mPort);
        mState = State::CONNECTED;
        return Progress::Done;
    case TlsBackend::HandshakeResult::WantRead: return Progress::WantRead;
    case TlsBackend::HandshakeResult::WantWrite: return Progress::WantWrite;
    case TlsBackend::HandshakeResult::Error:
    default: disconnect(); return Progress::Failed;
    }
}

bool TcpClient::disconnect() {
    VSCOPE_FUNCTION();
    if (is_disconnected()) {
        return true;
    }

    mState = State::DISCONNECTED;
    return true;
}

int TcpClient::receive(void* buffer, int size) {
    VSCOPE_FUNCTIONF("%d", size);
    if (!is_connected()) {
        return -1;
    }

    if (mTls) return mTls->read(buffer, size);

    auto result = ::read(mSocket, buffer, static_cast<size_t>(size));
    VERBOSEF("::read(%d, %p, %d) = %d", mSocket, buffer, size, result);
    return static_cast<int>(result);
}

int TcpClient::send(void const* buffer, int size) {
    VSCOPE_FUNCTIONF("%d", size);
    if (!is_connected()) {
        return -1;
    }

    if (mTls) return mTls->write(buffer, size);

    auto result = ::send(mSocket, buffer, static_cast<size_t>(size), MSG_NOSIGNAL);
    VERBOSEF("::send(%d, %p, %d, MSG_NOSIGNAL) = %d", mSocket, buffer, size, result);
    return static_cast<int>(result);
}

}  // namespace supl
