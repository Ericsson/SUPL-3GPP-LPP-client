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

    TcpClient();
    ~TcpClient();

    bool     connect(std::string const& host, int port, std::string const& interface,
                     TlsConfig const& tls);
    Progress handle_connection();
    bool     disconnect();

    NODISCARD bool is_disconnected() const { return mState == State::DISCONNECTED; }
    NODISCARD bool is_connecting() const { return mState == State::CONNECTING; }
    NODISCARD bool is_connected() const { return mState == State::CONNECTED; }

    int receive(void* buffer, int size);
    int send(void const* buffer, int size);

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
