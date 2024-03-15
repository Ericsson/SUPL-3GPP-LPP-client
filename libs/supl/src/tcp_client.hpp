#pragma once
#if USE_OPENSSL
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
    TcpClient();
    ~TcpClient();

    bool connect(const std::string& host, int port, bool use_ssl);
    bool disconnect();
    bool is_connected();

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

#if USE_OPENSSL
    const SSL_METHOD* mSSLMethod;
    SSL_CTX*          mSSLContext;
    SSL*              mSSL;
#endif
};

}  // namespace supl
