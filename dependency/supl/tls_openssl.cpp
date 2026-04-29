#include "supl/tls.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <poll.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE_FORWARD_REF2(supl, tls);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, tls)

namespace supl {
namespace {

class OpenSSLBackend : public TlsBackend {
public:
    explicit OpenSSLBackend(TlsConfig config) : mConfig(std::move(config)) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        SSL_library_init();
        SSL_load_error_strings();
#endif
    }

    ~OpenSSLBackend() override { shutdown(); }

    HandshakeResult handshake(int fd, std::string const& sni) override {
        if (!mCtx) {
            mCtx = SSL_CTX_new(TLS_client_method());
            if (!mCtx) {
                ERRORF("SSL_CTX_new failed");
                return HandshakeResult::Error;
            }

            if (mConfig.skip_verify) {
                SSL_CTX_set_verify(mCtx, SSL_VERIFY_NONE, nullptr);
                WARNF("TLS server verification disabled");
            } else {
                SSL_CTX_set_verify(mCtx, SSL_VERIFY_PEER, nullptr);
                if (!mConfig.ca_cert_path.empty()) {
                    if (SSL_CTX_load_verify_locations(mCtx, mConfig.ca_cert_path.c_str(),
                                                      nullptr) != 1) {
                        ERRORF("failed to load CA cert: %s", mConfig.ca_cert_path.c_str());
                        return HandshakeResult::Error;
                    }
                } else {
                    if (SSL_CTX_set_default_verify_paths(mCtx) != 1) {
                        ERRORF("failed to set default verify paths");
                        return HandshakeResult::Error;
                    }
                }
            }

            if (!mConfig.client_cert_path.empty() && !mConfig.client_key_path.empty()) {
                if (SSL_CTX_use_certificate_file(mCtx, mConfig.client_cert_path.c_str(),
                                                 SSL_FILETYPE_PEM) != 1) {
                    ERRORF("failed to load client cert: %s", mConfig.client_cert_path.c_str());
                    return HandshakeResult::Error;
                }
                if (SSL_CTX_use_PrivateKey_file(mCtx, mConfig.client_key_path.c_str(),
                                                SSL_FILETYPE_PEM) != 1) {
                    ERRORF("failed to load client key: %s", mConfig.client_key_path.c_str());
                    return HandshakeResult::Error;
                }
                if (SSL_CTX_check_private_key(mCtx) != 1) {
                    ERRORF("client cert and key do not match");
                    return HandshakeResult::Error;
                }
            }

            mSsl = SSL_new(mCtx);
            if (!mSsl) {
                ERRORF("SSL_new failed");
                return HandshakeResult::Error;
            }

            SSL_set_fd(mSsl, fd);
            if (!sni.empty()) SSL_set_tlsext_host_name(mSsl, sni.c_str());
            if (!mConfig.skip_verify && !sni.empty()) {
                SSL_set1_host(mSsl, sni.c_str());
            }
        }

        ERR_clear_error();
        auto ret = SSL_connect(mSsl);
        if (ret == 1) {
            INFOF("TLS handshake ok: %s %s", SSL_get_version(mSsl), SSL_get_cipher(mSsl));
            return HandshakeResult::Ok;
        }

        auto err = SSL_get_error(mSsl, ret);
        if (err == SSL_ERROR_WANT_READ) return HandshakeResult::WantRead;
        if (err == SSL_ERROR_WANT_WRITE) return HandshakeResult::WantWrite;

        auto queued = ERR_get_error();
        char buf[256];
        ERR_error_string_n(queued, buf, sizeof(buf));
        ERRORF("SSL_connect failed: ssl_err=%d %s", err, buf);
        return HandshakeResult::Error;
    }

    IoResult read(void* buffer, int size) override {
        if (!mSsl) return {IoStatus::Error, 0};
        ERR_clear_error();
        auto ret = SSL_read(mSsl, buffer, size);
        if (ret > 0) return {IoStatus::Ok, ret};

        auto err = SSL_get_error(mSsl, ret);
        switch (err) {
        case SSL_ERROR_WANT_READ: return {IoStatus::WantRead, 0};
        case SSL_ERROR_WANT_WRITE: return {IoStatus::WantWrite, 0};
        case SSL_ERROR_ZERO_RETURN: return {IoStatus::Closed, 0};
        default: break;
        }

        auto queued = ERR_get_error();
        char buf[256];
        ERR_error_string_n(queued, buf, sizeof(buf));
        ERRORF("SSL_read failed: ssl_err=%d %s", err, buf);
        return {IoStatus::Error, 0};
    }

    IoResult write(void const* buffer, int size) override {
        if (!mSsl) return {IoStatus::Error, 0};
        ERR_clear_error();
        auto ret = SSL_write(mSsl, buffer, size);
        if (ret > 0) return {IoStatus::Ok, ret};

        auto err = SSL_get_error(mSsl, ret);
        switch (err) {
        case SSL_ERROR_WANT_READ: return {IoStatus::WantRead, 0};
        case SSL_ERROR_WANT_WRITE: return {IoStatus::WantWrite, 0};
        case SSL_ERROR_ZERO_RETURN: return {IoStatus::Closed, 0};
        default: break;
        }

        auto queued = ERR_get_error();
        char buf[256];
        ERR_error_string_n(queued, buf, sizeof(buf));
        ERRORF("SSL_write failed: ssl_err=%d %s", err, buf);
        return {IoStatus::Error, 0};
    }

    void shutdown() override {
        if (mSsl) {
            SSL_shutdown(mSsl);
            SSL_free(mSsl);
            mSsl = nullptr;
        }
        if (mCtx) {
            SSL_CTX_free(mCtx);
            mCtx = nullptr;
        }
    }

    bool has_pending_plaintext() const override {
        // SSL_pending returns the number of bytes already decrypted and
        // buffered inside OpenSSL, available for immediate SSL_read without
        // any further socket I/O. This is the key signal that epoll alone is
        // insufficient to drive TLS reads: we must keep reading while > 0.
        return mSsl && SSL_pending(mSsl) > 0;
    }

private:
    TlsConfig mConfig;
    SSL_CTX*  mCtx = nullptr;
    SSL*      mSsl = nullptr;
};

}  // namespace

std::unique_ptr<TlsBackend> create_tls_backend_openssl(TlsConfig const& config) {
    return std::unique_ptr<TlsBackend>(new OpenSSLBackend(config));
}

}  // namespace supl
