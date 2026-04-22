#pragma once
#include <memory>
#include <string>

namespace supl {

struct TlsConfig {
    bool        enabled          = false;
    bool        skip_verify      = false;
    std::string ca_cert_path     = "";
    std::string client_cert_path = "";
    std::string client_key_path  = "";
};

class TlsBackend {
public:
    enum class HandshakeResult {
        Ok,
        WantRead,
        WantWrite,
        Error,
    };

    virtual ~TlsBackend() = default;

    virtual HandshakeResult handshake(int fd, std::string const& sni) = 0;
    virtual int             read(void* buffer, int size)              = 0;
    virtual int             write(void const* buffer, int size)       = 0;
    virtual void            shutdown()                                = 0;
};

std::unique_ptr<TlsBackend> create_tls_backend(TlsConfig const& config);

}  // namespace supl
