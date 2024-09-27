#pragma once
#include "supl.hpp"

#if defined(USE_OPENSSL)
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

#include <string>

namespace supl {

class TcpClient {
public:
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
    };

    TcpClient();
    ~TcpClient();

    bool connect(const std::string& host, int port, bool use_ssl);
    bool handle_connection();
    bool disconnect();

    NODISCARD bool is_disconnected() const { return mState == State::DISCONNECTED; }
    NODISCARD bool is_connecting() const { return mState == State::CONNECTING; }
    NODISCARD bool is_connected() const { return mState == State::CONNECTED; }

    int receive(void* buffer, int size);
    int send(const void* buffer, int size);

    int fd() const { return mSocket; }

protected:
    bool initialize_socket();

private:
    std::string mHost;
    int         mPort;
    bool        mUseSSL;
    int         mSocket;
    State       mState;

#if defined(USE_OPENSSL)
    const SSL_METHOD* mSSLMethod;
    SSL_CTX*          mSSLContext;
    SSL*              mSSL;
#endif
};

}  // namespace supl
