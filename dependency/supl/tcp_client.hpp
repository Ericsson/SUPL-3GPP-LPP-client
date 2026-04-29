#pragma once
#include "supl.hpp"
#include "supl/tls.hpp"

#include <memory>
#include <string>

namespace supl {

class TcpClient {
public:
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
    };

    enum class Progress {
        Done,       // TCP+TLS handshake complete
        WantRead,   // waiting on readable event to continue
        WantWrite,  // waiting on writable event to continue
        Failed,     // fatal error
    };

    enum class IoStatus {
        Ok,         // bytes transferred (see IoResult::bytes)
        WantRead,   // retry when the fd is readable
        WantWrite,  // retry when the fd is writable
        Closed,     // peer closed (TCP EOF or TLS clean shutdown)
        Error,      // fatal error; caller should disconnect
    };

    struct IoResult {
        IoStatus status;
        int      bytes;  // meaningful when status == Ok; 0 otherwise
    };

    TcpClient();
    ~TcpClient();

    bool     connect(std::string const& host, int port, std::string const& interface,
                     TlsConfig const& tls);
    Progress handle_connection();
    bool     disconnect();

    NODISCARD bool is_disconnected() const { return mState == State::DISCONNECTED; }
    NODISCARD bool is_connecting() const { return mState == State::CONNECTING; }
    NODISCARD bool is_connected() const { return mState == State::CONNECTED; }

    IoResult receive(void* buffer, int size);
    IoResult send(void const* buffer, int size);

    // True if the transport has already-decrypted data buffered that must be
    // drained before returning to the epoll event loop. Always false for
    // plain TCP (the kernel buffer is what epoll reports on). For TLS, this
    // reflects OpenSSL's internal plaintext buffer.
    NODISCARD bool has_pending_data() const;

    NODISCARD int fd() const { return mSocket; }

protected:
    bool initialize_socket();
    void unitialize_socket();

private:
    std::string                 mHost;
    int                         mPort;
    std::string                 mInterface;
    int                         mSocket;
    State                       mState;
    bool                        mTlsStarted = false;
    std::unique_ptr<TlsBackend> mTls;
};

}  // namespace supl
