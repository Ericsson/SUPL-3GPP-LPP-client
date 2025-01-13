#include "tcp_client.hpp"
#include "supl.hpp"

#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "supl"

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
    auto result =
        getnameinfo(addr, sizeof(struct sockaddr), host, sizeof(host), nullptr, 0, NI_NUMERICHOST);
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
    auto             result = getaddrinfo(mHost.c_str(), port_as_string, &hint, &ailist);
    if (result != 0) {
        WARNF("failed to get address info: \"%s\" %d (%s)", mHost.c_str(), result,
              gai_strerror(result));
        return false;
    }

    SUPL_DEFER {
        freeaddrinfo(ailist);
    };

    for (aip = ailist; aip; aip = aip->ai_next) {
        auto fd = socket(aip->ai_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (fd < 0) {
            WARNF("failed to create socket: %d (%s)", errno, strerror(errno));
            continue;
        }

        VERBOSEF("created socket %d", fd);

        auto addr_str = addr_to_string(aip->ai_addr);
        result        = ::connect(fd, aip->ai_addr, aip->ai_addrlen);
        if (result != 0) {
            if (errno == EINPROGRESS) {
                VERBOSEF("connection inprogress to %s", addr_str.c_str());
                mSocket = fd;
                return true;
            }

            WARNF("failed to connect to %s: %d (%s)", addr_str.c_str(), errno, strerror(errno));
            VERBOSEF("closing socket %d", fd);
            close(fd);
            continue;
        } else {
            mSocket = fd;
            return true;
        }
    }

    WARNF("no connection could be established to any address");
    return false;
}

bool TcpClient::connect(std::string const& host, int port, bool use_ssl) {
    VSCOPE_FUNCTION();
    if (is_disconnected()) {
        return false;
    }

    mHost   = host;
    mPort   = port;
    mUseSSL = use_ssl;

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
    int       error = 0;
    socklen_t len   = sizeof(error);
    if (getsockopt(mSocket, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
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

    VERBOSEF("closing socket %d", mSocket);
    close(mSocket);
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
    return static_cast<int>(::read(mSocket, buffer, static_cast<size_t>(size)));
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
    return static_cast<int>(::send(mSocket, buffer, static_cast<size_t>(size), MSG_NOSIGNAL));
#endif
}

}  // namespace supl
