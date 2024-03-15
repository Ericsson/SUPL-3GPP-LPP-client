#include "tcp_client.hpp"

#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace supl {

#if USE_OPENSSL
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
#if USE_OPENSSL
    OpenSSL_init();

    mSSLMethod  = nullptr;
    mSSLContext = nullptr;
    mSSL        = nullptr;
#endif
}

TcpClient::~TcpClient() {
    disconnect();
}

bool TcpClient::is_connected() {
    return mSocket >= 0;
}

bool TcpClient::initialize_socket() {
    char port_as_string[8];
    snprintf(port_as_string, sizeof(port_as_string), "%i", mPort);

    struct addrinfo hint;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_socktype = SOCK_STREAM;

    struct addrinfo *ailist, *aip;
    auto             err = getaddrinfo(mHost.c_str(), port_as_string, &hint, &ailist);
    if (err != 0) {
        return false;
    }

    int fd = -1;
    for (aip = ailist; aip; aip = aip->ai_next) {
        char host[256];
        err =
            getnameinfo(aip->ai_addr, aip->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        if (err != 0) {
            return false;
        }

        if ((fd = socket(aip->ai_family, SOCK_STREAM, 0)) < 0) {
            err = errno;
        }

        err = ::connect(fd, aip->ai_addr, aip->ai_addrlen);
        if (err != 0) {
            freeaddrinfo(ailist);
            return false;
        }

        break;
    }

    mSocket = fd;
    freeaddrinfo(ailist);
    return true;
}

bool TcpClient::connect(const std::string& host, int port, bool use_ssl) {
    if (is_connected()) {
        return true;
    }

    mHost   = host;
    mPort   = port;
    mUseSSL = use_ssl;

#if USE_OPENSSL
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

#if USE_OPENSSL
    if (mUseSSL) {
        SSL_set_fd(mSSL, mSocket);
        if (SSL_connect(mSSL) == -1) {
            return false;
        }
    }
#endif

    return true;
}

bool TcpClient::disconnect() {
    if (!is_connected()) {
        return true;
    }

#if USE_OPENSSL
    if (mUseSSL) {
        SSL_shutdown(mSSL);
        SSL_free(mSSL);
        SSL_CTX_free(mSSLContext);
    }

    mSSLMethod  = nullptr;
    mSSLContext = nullptr;
    mSSL        = nullptr;
#endif

    close(mSocket);
    mSocket = -1;
    return true;
}

int TcpClient::receive(void* buffer, int size) {
#if USE_OPENSSL
    if (mUseSSL)
        return SSL_read(mSSL, buffer, size);
    else
        return read(mSocket, buffer, size);
#else
    return ::read(mSocket, buffer, size);
#endif
}

int TcpClient::send(const void* buffer, int size) {
#if USE_OPENSSL
    if (mUseSSL)
        return SSL_write(mSSL, buffer, size);
    else
        return write(mSocket, buffer, size);
#else
    return write(mSocket, buffer, size);
#endif
}

}  // namespace supl
