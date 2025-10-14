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
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, tcp)

namespace supl {

#if defined(USE_OPENSSL)
static bool OpenSSL_initialized = false;

static void OpenSSL_init() {
    if (OpenSSL_initialized) {
        return;
    }

    // OpenSSL is one of the worst libraries I have ever worked with. Too much
    // global state and no good documentation, this stackoverflow thread goes
    // through how to initialize and cleanup OpenSSL.
    // https://stackoverflow.com/questions/29845527/how-to-properly-uninitialize-openssl/29927669#29927669
    SSL_library_init();
    SSL_load_error_strings();

    OpenSSL_initialized = true;
}

static void OpenSSL_cleanup() {
    if (!OpenSSL_initialized) {
        return;
    }

    ERR_free_strings();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();

    OpenSSL_initialized = false;
}
#endif

TcpClient::TcpClient() : mSocket(-1) {
    VSCOPE_FUNCTION();
    mState = State::DISCONNECTED;
#if defined(USE_OPENSSL)
    OpenSSL_init();

    mSSLMethod  = nullptr;
    mSSLContext = nullptr;
    mSSL        = nullptr;
#endif
}

TcpClient::~TcpClient() {
    VSCOPE_FUNCTION();
    disconnect();
}

static std::string addr_to_string(const struct sockaddr* addr) {
    char host[1024];
    auto result = ::getnameinfo(addr, sizeof(struct sockaddr), host, sizeof(host), nullptr, 0,
                                NI_NUMERICHOST);
    VERBOSEF("::getnameinfo(%p, %zu, %p, %zu, %p, %d, NI_NUMERICHOST) = %d", addr,
             sizeof(struct sockaddr), host, sizeof(host), nullptr, 0, result);
    if (result != 0) {
        return "unknown";
    } else {
        return host;
    }
}

bool TcpClient::initialize_socket() {
    VSCOPE_FUNCTION();

    char port_as_string[8];
    snprintf(port_as_string, sizeof(port_as_string), "%i", mPort);
    VERBOSEF("connecting to %s:%s", mHost.c_str(), port_as_string);

    struct addrinfo hint;
    memset(&hint, 0, sizeof(struct addrinfo));
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
        auto fd = socket(aip->ai_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
        VERBOSEF("::socket(%d, SOCK_STREAM | SOCK_NONBLOCK, 0) = %d", aip->ai_family, fd);
        if (fd < 0) {
            WARNF("failed to create socket: " ERRNO_FMT, ERRNO_ARGS(errno));
            continue;
        }

        if (mInterface.length() > 0) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, mInterface.c_str(), IFNAMSIZ - 1);
            ifr.ifr_name[IFNAMSIZ - 1] = '\0';
            auto set_result = ::setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,
                                           reinterpret_cast<void*>(&ifr), sizeof(ifr));
            if (set_result < 0) {
                WARNF("failed to set SO_BINDTODEVICE: " ERRNO_FMT, ERRNO_ARGS(errno));
                continue;
            }
        }

        VERBOSEF("created socket %d", fd);

        auto addr_str = addr_to_string(aip->ai_addr);
        result        = ::connect(fd, aip->ai_addr, aip->ai_addrlen);
        VERBOSEF("::connect(%d, %p, %d) = %d", fd, aip->ai_addr, aip->ai_addrlen, result);
        if (result != 0) {
            if (errno == EINPROGRESS) {
                VERBOSEF("connection inprogress to %s", addr_str.c_str());
                mSocket = fd;
                return true;
            }

            WARNF("failed to connect to %s: " ERRNO_FMT, addr_str.c_str(), ERRNO_ARGS(errno));
            VERBOSEF("closing socket %d", fd);
            close(fd);
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
                        bool use_ssl) {
    VSCOPE_FUNCTION();
    if (!is_disconnected()) {
        DEBUGF("client is already connected");
        return false;
    }

    mHost      = host;
    mPort      = port;
    mInterface = interface;
    mUseSSL    = use_ssl;

#if defined(USE_OPENSSL)
    if (mUseSSL) {
        mSSLMethod =
#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
            TLSv1_2_client_method();
#else
            TLS_client_method();
#endif
        mSSLContext = SSL_CTX_new(mSSLMethod);
        if (!mSSLContext) {
            return false;
        }

        mSSL = SSL_new(mSSLContext);
        if (!mSSL) {
            return false;
        }
    }
#endif

    if (!initialize_socket()) {
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

bool TcpClient::handle_connection() {
    VSCOPE_FUNCTION();
    if (!is_connecting()) {
        disconnect();
        return false;
    }

    // check that the connection was successful
    int       error  = 0;
    socklen_t len    = sizeof(error);
    auto      result = ::getsockopt(mSocket, SOL_SOCKET, SO_ERROR, &error, &len);
    VERBOSEF("::getsockopt(%d, SOL_SOCKET, SO_ERROR, %p, %p) = %d", mSocket, &error, &len, result);
    if (result < 0) {
        disconnect();
        return false;
    }

    if (error != 0) {
        disconnect();
        return false;
    }

#if defined(USE_OPENSSL)
    if (mUseSSL) {
        SSL_set_fd(mSSL, mSocket);
        if (SSL_connect(mSSL) == -1) {
            disconnect();
            return false;
        }
    }
#endif

    VERBOSEF("connected to %s:%d", mHost.c_str(), mPort);
    mState = State::CONNECTED;
    return true;
}

bool TcpClient::disconnect() {
    VSCOPE_FUNCTION();
    if (is_disconnected()) {
        return true;
    }

#if defined(USE_OPENSSL)
    if (mUseSSL) {
        SSL_shutdown(mSSL);
        SSL_free(mSSL);
        SSL_CTX_free(mSSLContext);
    }

    mSSLMethod  = nullptr;
    mSSLContext = nullptr;
    mSSL        = nullptr;
#endif

    auto result = shutdown(mSocket, SHUT_RDWR);
    VERBOSEF("::shutdown(%d, SHUT_RDWR) = %d", mSocket, result);

    VERBOSEF("closing socket %d", mSocket);
    close(mSocket);
    VERBOSEF("::close(%d)", mSocket);
    mSocket = -1;
    mState  = State::DISCONNECTED;
    return true;
}

int TcpClient::receive(void* buffer, int size) {
    VSCOPE_FUNCTIONF("%d", size);
    if (!is_connected()) {
        return -1;
    }

#if defined(USE_OPENSSL)
    if (mUseSSL)
        return SSL_read(mSSL, buffer, size);
    else
        return read(mSocket, buffer, size);
#else
    auto result = ::read(mSocket, buffer, static_cast<size_t>(size));
    VERBOSEF("::read(%d, %p, %d) = %d", mSocket, buffer, size, result);
    return static_cast<int>(result);
#endif
}

int TcpClient::send(void const* buffer, int size) {
    VSCOPE_FUNCTIONF("%d", size);
    if (!is_connected()) {
        return -1;
    }

#if defined(USE_OPENSSL)
    if (mUseSSL)
        return SSL_write(mSSL, buffer, size);
    else
        return write(mSocket, buffer, size);
#else
    auto result = ::send(mSocket, buffer, static_cast<size_t>(size), MSG_NOSIGNAL);
    VERBOSEF("::send(%d, %p, %d, MSG_NOSIGNAL) = %d", mSocket, buffer, size, result);
    return static_cast<int>(result);
#endif
}

}  // namespace supl
