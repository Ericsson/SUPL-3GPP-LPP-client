#include "supl/session.hpp"
#include "decode.hpp"
#include "encode.hpp"
#include "supl.hpp"
#include "supl/end.hpp"
#include "supl/response.hpp"
#include "supl/start.hpp"
#include "tcp_client.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <OCTET_STRING.h>
#include <ULP-PDU.h>
EXTERNAL_WARNINGS_POP

#include <chrono>
#include <loglet/loglet.hpp>

#define UPER_DECODE_BUFFER_SIZE (64 * 1024)

LOGLET_MODULE2(supl, session);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, session)

namespace supl {

Session::Session(Version version, Identity identity)
    : mVersion(std::move(version)), mState(State::UNKNOWN) {
    VSCOPE_FUNCTION();

    mReceiveBufferSize   = UPER_DECODE_BUFFER_SIZE;
    mReceiveBufferOffset = 0;
    mReceiveBuffer       = new uint8_t[mReceiveBufferSize];

    mTcpClient = nullptr;

    // TODO: How should the Session ID be determined?
    mSETSession           = {};
    mSETSession.is_active = true;
    mSETSession.id        = 1024;
    mSETSession.identity  = std::move(identity);
    DEBUGF("set session: %lld", mSETSession.id);

    mSLPSession           = {};
    mSLPSession.is_active = false;
}

Session::~Session() {
    VSCOPE_FUNCTION();
    delete[] mReceiveBuffer;

    if (mTcpClient) {
        delete mTcpClient;
        mTcpClient = nullptr;
    }
}

bool Session::connect(std::string const& ip, uint16_t port, std::string const& interface) {
    VSCOPE_FUNCTIONF("\"%s\",%d,\"%s\"", ip.c_str(), port, interface.c_str());

    if (mState != State::UNKNOWN) {
        WARNF("mState != State::UNKNOWN");
        return false;
    }

    mTcpClient = new TcpClient();
    if (!mTcpClient) {
        WARNF("failed to create TcpClient");
        return false;
    }

    if (!mTcpClient->connect(ip, port, interface, false)) {
        WARNF("connect failed");
        delete mTcpClient;
        mTcpClient = nullptr;
        return false;
    }

    mState = State::CONNECTING;
    return true;
}

bool Session::handle_connection() {
    if (mState != State::CONNECTING) {
        WARNF("mState != State::CONNECTING");
        return false;
    }

    if (!mTcpClient) {
        WARNF("mTcpClient is null");
        return false;
    }

    if (mTcpClient->handle_connection()) {
        mState                = State::CONNECTED;
        mSETSession.is_active = true;
        mSLPSession.is_active = false;
        return true;
    } else {
        mState                = State::DISCONNECTED;
        mSETSession.is_active = false;
        mSLPSession.is_active = false;
        delete mTcpClient;
        mTcpClient = nullptr;
        return false;
    }
}

void Session::disconnect() {
    VSCOPE_FUNCTION();
    if (mTcpClient) {
        mSETSession.is_active = false;
        mSLPSession.is_active = false;
        mTcpClient->disconnect();
    }
}

bool Session::is_connected() const {
    return mTcpClient && mTcpClient->is_connected();
}

bool Session::handshake(const START& message) {
    VSCOPE_FUNCTION();

    if (mState != State::CONNECTED) {
        WARNF("mState != State::CONNECTED");
        return false;
    }

    if (!send(message)) {
        WARNF("send failed");
        return false;
    }

    mState = State::WaitForHandshake;
    return true;
}

Session::Handshake Session::handle_handshake() {
    VSCOPE_FUNCTION();

    if (mState != State::WaitForHandshake) {
        WARNF("mState != State::WaitForHandshake");
        return Handshake::ERROR;
    }

    if (!fill_receive_buffer()) {
        return Handshake::ERROR;
    }

    RESPONSE response{};
    END      end{};
    auto     received = try_receive(&response, &end, nullptr);
    if (received == Received::NoData) {
        return Handshake::NoData;
    } else if (received != Received::RESPONSE) {
        WARNF("expected SUPLRESPONSE");
        return Handshake::ERROR;
    }

    mState = State::CONNECTED;
    return Handshake::OK;
}

bool Session::send(const START& message) {
    FUNCTION_SCOPEN("START");
    if (!is_connected()) {
        WARNF("not connected");
        return false;
    }

    auto encoded_message = encode(mVersion, mSETSession, mSLPSession, message);
    if (encoded_message.size() == 0) {
        WARNF("encode failed");
        return false;
    }

    auto sent = mTcpClient->send(encoded_message.data(), static_cast<int>(encoded_message.size()));
    return static_cast<size_t>(sent) == encoded_message.size();
}

bool Session::send(const POSINIT& message) {
    FUNCTION_SCOPEN("POSINIT");
    if (!is_connected()) {
        WARNF("not connected");
        return false;
    }

    auto encoded_message = encode(mVersion, mSETSession, mSLPSession, message);
    if (encoded_message.size() == 0) {
        WARNF("encode failed");
        return false;
    }

    auto sent = mTcpClient->send(encoded_message.data(), static_cast<int>(encoded_message.size()));
    return static_cast<size_t>(sent) == encoded_message.size();
}

bool Session::send(const POS& message) {
    FUNCTION_SCOPEN("POS");
    if (!is_connected()) {
        WARNF("not connected");
        return false;
    }

    auto encoded_message = encode(mVersion, mSETSession, mSLPSession, message);
    if (encoded_message.size() == 0) {
        WARNF("encode failed");
        return false;
    }

    auto sent = mTcpClient->send(encoded_message.data(), static_cast<int>(encoded_message.size()));
    return static_cast<size_t>(sent) == encoded_message.size();
}

void Session::rb_consume(size_t bytes) {
    VERBOSEF("consume %zd bytes", bytes);
    if (bytes >= mReceiveBufferOffset) {
        mReceiveBufferOffset = 0;
    } else {
        auto discard_length = bytes;
        auto shift_length   = mReceiveBufferOffset - discard_length;
        auto from_ptr       = mReceiveBuffer + discard_length;
        auto to_ptr         = mReceiveBuffer;
        memmove(to_ptr, from_ptr, shift_length);
        mReceiveBufferOffset -= discard_length;
    }
}

ULP_PDU* Session::parse_receive_buffer() {
    VSCOPE_FUNCTIONF("%zd/%zd", mReceiveBufferOffset, mReceiveBufferSize);
    if (mReceiveBufferOffset < 4) {
        return nullptr;
    }

    ULP_PDU* ulp_pdu{};
    auto     size   = mReceiveBufferOffset;
    auto     buffer = mReceiveBuffer;

    // NOTE: Increase default max stack size to handle large messages.
    // TODO(ewasjon): Is this correct?
    asn_codec_ctx_t stack_ctx{};
    stack_ctx.max_stack_size = 1024 * 1024 * 4;

    auto decode_start = std::chrono::steady_clock::now();
    auto result       = uper_decode_complete(&stack_ctx, &asn_DEF_ULP_PDU,
                                             reinterpret_cast<void**>(&ulp_pdu), buffer, size);
    auto decode_end   = std::chrono::steady_clock::now();
    auto decode_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(decode_end - decode_start).count();

    if (decode_ms > 100) {
        WARNF("SUPL decode took %lld ms (buffer size: %zu bytes)", decode_ms, size);
    } else {
        VERBOSEF("SUPL decode took %lld ms (buffer size: %zu bytes)", decode_ms, size);
    }

    DEBUGF("uper_decode_complete(): %s %zd",
           (result.code == RC_FAIL ? "RC_FAIL" : (result.code == RC_WMORE ? "RC_WMORE" : "RC_OK")),
           result.consumed);
    if (result.code == RC_FAIL) {
        WARNF("failed to decode uper: %zd bytes consumed", result.consumed);
        rb_consume(result.consumed);
        return nullptr;
    } else if (result.code == RC_WMORE) {
        VERBOSEF("more data is needed to decode the fully message (%zd of %ld)", size,
                 ulp_pdu->length);
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
        return nullptr;
    } else {
        rb_consume(result.consumed);
        return ulp_pdu;
    }
}

bool Session::fill_receive_buffer() {
    VSCOPE_FUNCTION();

    auto buffer = mReceiveBuffer + mReceiveBufferOffset;
    auto size   = mReceiveBufferSize - mReceiveBufferOffset;
    auto bytes  = mTcpClient->receive(buffer, static_cast<int>(size));
    if (bytes <= 0) {
        WARNF("receive failed");
        return false;
    }

    VERBOSEF("received %zd bytes", bytes);
    mReceiveBufferOffset += static_cast<size_t>(bytes);
    return true;
}

ULP_PDU* Session::wait_for_ulp_pdu() {
    VSCOPE_FUNCTION();

    for (;;) {
        auto ulp_pdu = parse_receive_buffer();
        if (ulp_pdu) return ulp_pdu;

        if (!fill_receive_buffer()) {
            return nullptr;
        }
    }

    UNREACHABLE();
}

bool operator==(Identity const& a, Identity const& b) {
    if (a.type != b.type) return false;

    switch (a.type) {
    case Identity::UNKNOWN: return false;
    case Identity::MSISDN: return a.data.msisdn == b.data.msisdn;
    case Identity::IMSI: return a.data.imsi == b.data.imsi;
    case Identity::IPV4: return memcmp(a.data.ipv4, a.data.ipv4, 4) == 0;
    case Identity::IPV6: return memcmp(a.data.ipv6, a.data.ipv6, 16) == 0;
    case Identity::FQDN: return a.data.fqdn == b.data.fqdn;
    }

    return false;
}

static bool operator==(Session::SET const& a, Session::SET const& b) {
    if (a.id != b.id) return false;
    if (a.is_active != b.is_active) return false;
    return a.identity == b.identity;
}

static bool operator==(Session::SLP const& a, Session::SLP const& b) {
    if (memcmp(a.id, b.id, 4) != 0) return false;
    if (a.is_active != b.is_active) return false;
    return a.identity == b.identity;
}

static bool operator!=(Session::SET const& a, Session::SET const& b) {
    return !(a == b);
}

static bool operator!=(Session::SLP const& a, Session::SLP const& b) {
    return !(a == b);
}

Session::Received Session::block_receive(RESPONSE* response, END* end, POS* pos) {
    VSCOPE_FUNCTIONF("%s,%s,%s", response ? "RESPONSE" : "-", end ? "END" : "-", pos ? "POS" : "-");
    if (!is_connected()) {
        WARNF("not connected");
        return Received::SessionTerminated;
    }

    auto ulp_pdu = wait_for_ulp_pdu();
    if (!ulp_pdu) {
        WARNF("unable to find or decode message");
        return Received::UnableToDecode;
    }

    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    return parse_message(ulp_pdu, response, end, pos);
}

Session::Received Session::try_receive(RESPONSE* response, END* end, POS* pos) {
    VSCOPE_FUNCTIONF("%s,%s,%s", response ? "RESPONSE" : "-", end ? "END" : "-", pos ? "POS" : "-");
    if (!is_connected()) {
        WARNF("not connected");
        return Received::SessionTerminated;
    }

    auto ulp_pdu = parse_receive_buffer();
    if (!ulp_pdu) {
        return Received::NoData;
    }

    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    return parse_message(ulp_pdu, response, end, pos);
}

Session::Received Session::parse_message(ULP_PDU* ulp_pdu, RESPONSE* response, END* end, POS* pos) {
    VSCOPE_FUNCTIONF("%s,%s,%s", response ? "RESPONSE" : "-", end ? "END" : "-", pos ? "POS" : "-");

    SET      receive_set{};
    SLP      receive_slp{};
    Received received = Received::UNKNOWN;
    if (response && decode_response(receive_set, receive_slp, *response, ulp_pdu)) {
        received = Received::RESPONSE;
    } else if (end && decode_end(receive_set, receive_slp, *end, ulp_pdu)) {
        received = Received::END;
    } else if (pos && decode_pos(receive_set, receive_slp, *pos, ulp_pdu)) {
        received = Received::POS;
    } else {
        WARNF("unsupported message");
        return Received::UNKNOWN;
    }

    assert(mSETSession.is_active);
    if (receive_set != mSETSession) {
        WARNF("invalid SET session");
        return Received::InvalidSession;
    }

    if (!mSLPSession.is_active) {
        mSLPSession = receive_slp;
        assert(mSLPSession.is_active);
    } else if (receive_slp != mSLPSession) {
        WARNF("invalid SLP session");
        return Received::InvalidSession;
    }

    return received;
}

int Session::fd() const {
    if (mTcpClient) {
        return mTcpClient->fd();
    } else {
        return -1;
    }
}

}  // namespace supl
