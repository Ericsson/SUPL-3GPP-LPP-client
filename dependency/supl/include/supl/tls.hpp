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

    enum class IoStatus {
        Ok,         // bytes transferred (see IoResult::bytes)
        WantRead,   // retry when socket is readable
        WantWrite,  // retry when socket is writable
        Closed,     // peer performed a clean TLS shutdown
        Error,      // fatal TLS/IO error
    };

    struct IoResult {
        IoStatus status;
        int      bytes;  // valid (>= 0) when status == Ok
    };

    virtual ~TlsBackend() = default;

    virtual HandshakeResult handshake(int fd, std::string const& sni) = 0;
    virtual IoResult        read(void* buffer, int size)              = 0;
    virtual IoResult        write(void const* buffer, int size)       = 0;
    virtual void            shutdown()                                = 0;

    // Non-zero if the backend has already decrypted plaintext available for
    // immediate read, even though the underlying socket may not be reported
    // readable by epoll. Callers must drain this before returning to the
    // event loop, otherwise they deadlock waiting for socket readability
    // that will never come.
    virtual bool has_pending_plaintext() const = 0;
};

std::unique_ptr<TlsBackend> create_tls_backend(TlsConfig const& config);

}  // namespace supl
