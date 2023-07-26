#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#if USE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif

#include <string>

class TCP_Client {
public:
    TCP_Client();
    ~TCP_Client();

    bool connect(const std::string& host, int port, bool use_ssl);
    bool disconnect();
    bool is_connected();

    int receive(void* buffer, int size, int milliseconds);
    int send(void* buffer, int size);

protected:
    bool initialize_socket();

private:
    std::string mHost;
    int         mPort;
    bool        mUseSSL;

    int mSocket;

#if USE_OPENSSL
    const SSL_METHOD* mSSLMethod;
    SSL_CTX*          mSSLContext;
    SSL*              mSSL;
#endif
};
