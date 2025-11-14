#pragma once
#include <supl/identity.hpp>
#include <supl/version.hpp>

struct ULP_PDU;

namespace supl {

struct START;
struct RESPONSE;
struct END;
struct POSINIT;
struct POS;

class TcpClient;
class Session {
public:
    struct SET {
        bool     is_active;
        int64_t  id;
        Identity identity;
    };

    struct SLP {
        bool     is_active;
        uint8_t  id[4];
        Identity identity;
    };

    enum class Received {
        UNKNOWN = 0,
        InvalidSession,
        UnableToDecode,
        SessionTerminated,
        NoData,
        RESPONSE,
        END,
        POS,
    };

    enum class State {
        UNKNOWN,
        CONNECTING,
        CONNECTED,
        WaitForHandshake,
        DISCONNECTED,
    };

    enum class Handshake {
        OK = 0,
        ERROR,
        NoData,
    };

    explicit Session(Version version, Identity identity);
    ~Session();

    bool connect(std::string const& ip, uint16_t port, std::string const& interface);
    bool handle_connection();

    void           disconnect();
    NODISCARD bool is_connected() const;
    NODISCARD bool is_disconnected() const { return mState == State::DISCONNECTED; }

    bool      handshake(const START& message);
    Handshake handle_handshake();

    bool send(const START& message);
    bool send(const POSINIT& message);
    bool send(const POS& message);

    Received block_receive(RESPONSE* response, END* end, POS* pos);
    Received try_receive(RESPONSE* response, END* end, POS* pos);

    NODISCARD const Version& version() const { return mVersion; }
    NODISCARD const SET&     set() const { return mSETSession; }
    NODISCARD const SLP&     slp() const { return mSLPSession; }

    NODISCARD int fd() const;
    bool          fill_receive_buffer();

protected:
    ULP_PDU* parse_receive_buffer();
    ULP_PDU* wait_for_ulp_pdu();
    Received parse_message(ULP_PDU* ulp_pdu, RESPONSE* response, END* end, POS* pos);

    void rb_consume(size_t bytes);

private:
    Version mVersion;
    State   mState;

    SET mSETSession;
    SLP mSLPSession;

    uint8_t* mReceiveBuffer;
    size_t   mReceiveBufferSize;
    size_t   mReceiveBufferOffset;

    TcpClient* mTcpClient;
};

}  // namespace supl
