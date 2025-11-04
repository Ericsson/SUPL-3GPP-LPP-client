#include "decode.hpp"
#include "print.hpp"
#include "supl/end.hpp"
#include "supl/pos.hpp"
#include "supl/response.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <FQDN.h>
#include <MCC-MNC-Digit.h>
#include <MCC.h>
#include <MNC.h>
#include <PosProtocolVersion3GPP.h>
#include <PosProtocolVersion3GPP2.h>
#include <SETId.h>
#include <SessionID.h>
#include <SetSessionID.h>
#include <SlpSessionID.h>
#include <ULP-PDU.h>
#include <UTCTime.h>
#include <Ver2-PosProtocol-extension.h>
#pragma GCC diagnostic pop

#include <loglet/loglet.hpp>

LOGLET_MODULE2(supl, decode);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, decode)

namespace supl {

static Identity decode_identity(IPAddress& ip_address) {
    VSCOPE_FUNCTION();
    switch (ip_address.present) {
    case IPAddress_PR_NOTHING:
        VERBOSEF("no IP address present");
        return Identity::unknown();
    case IPAddress_PR_ipv4Address: {
        auto input = ip_address.choice.ipv4Address;
        assert(input.size == 4);
        return Identity::ipv4(input.buf);
    }

    case IPAddress_PR_ipv6Address: {
        auto input = ip_address.choice.ipv6Address;
        assert(input.size == 16);
        return Identity::ipv6(input.buf);
    }
    }

    VERBOSEF("unknown IP address type: %d", ip_address.present);
    return Identity::unknown();
}

static Identity decode_identity(SETId& set_id) {
    VSCOPE_FUNCTION();

    switch (set_id.present) {
    case SETId_PR_msisdn: {
        auto input = set_id.choice.msisdn;
        assert(input.size > 0);

        uint64_t msisdn = 0;
        for (size_t i = 0; i < input.size; i++) {
            auto byte   = input.buf[i];
            auto first  = static_cast<uint64_t>((byte >> 0) & 0xF);
            auto second = static_cast<uint64_t>((byte >> 4) & 0xF);
            if (first != 0xF) {
                msisdn = 10 * msisdn + first;
            }
            if (second != 0xF) {
                msisdn = 10 * msisdn + second;
            }
        }

        VERBOSEF("msisdn: %" PRIu64, msisdn);
        return Identity::msisdn(msisdn);
    }

    case SETId_PR_imsi: {
        auto input = set_id.choice.imsi;
        assert(input.size > 0);

        uint64_t imsi = 0;
        for (size_t i = 0; i < input.size; i++) {
            auto byte   = input.buf[i];
            auto first  = static_cast<uint64_t>((byte >> 0) & 0xF);
            auto second = static_cast<uint64_t>((byte >> 4) & 0xF);
            if (first != 0xF) {
                imsi = 10 * imsi + first;
            }
            if (second != 0xF) {
                imsi = 10 * imsi + second;
            }
        }

        VERBOSEF("imsi: %" PRIu64, imsi);
        return Identity::imsi(imsi);
    }

    case SETId_PR_iPAddress: {
        auto ip_address = set_id.choice.iPAddress;
        return decode_identity(ip_address);
    }

    case SETId_PR_nai:
    case SETId_PR_min:
    case SETId_PR_mdn:
    case SETId_PR_NOTHING:
        WARNF("unsupported identity type: %d", set_id.present);
        return Identity::unknown();
    }

    VERBOSEF("unknown SETId type: %d", set_id.present);
    return Identity::unknown();
}

static Identity decode_identity(SLPAddress_t& slp_address) {
    VSCOPE_FUNCTION();
    switch (slp_address.present) {
    case SLPAddress_PR_NOTHING:
    case SLPAddress_PR_fQDN: {
        auto& fQDN = slp_address.choice.fQDN;
        auto value = std::string{reinterpret_cast<char*>(fQDN.buf), static_cast<size_t>(fQDN.size)};
        return Identity::fQDN(value);
    }

    case SLPAddress_PR_iPAddress: {
        auto& ip_address = slp_address.choice.iPAddress;
        return decode_identity(ip_address);
    }
    }

    VERBOSEF("unknown SLPAddress type: %d", slp_address.present);
    return Identity::unknown();
}

static bool decode_session(Session::SET& set, Session::SLP& slp, ULP_PDU* pdu) {
    VSCOPE_FUNCTION();

    if (!pdu) {
        VERBOSEF("pdu is null");
        return false;
    }

    auto set_session = pdu->sessionID.setSessionID;
    auto slp_session = pdu->sessionID.slpSessionID;
    if (!set_session || !slp_session) {
        VERBOSEF("missing session: set=%p slp=%p", set_session, slp_session);
        return false;
    }

    set.is_active = true;
    set.id        = set_session->sessionId;
    set.identity  = decode_identity(set_session->setId);

    auto slp_id = slp_session->sessionID;
    assert(slp_id.size == 4);
    slp.is_active = true;
    memcpy(slp.id, slp_id.buf, 4);
    slp.identity = decode_identity(slp_session->slpId);

    return true;
}

bool decode_response(Session::SET& set, Session::SLP& slp, RESPONSE& response, ULP_PDU* pdu) {
    VSCOPE_FUNCTION();

    if (!pdu) {
        VERBOSEF("pdu is null");
        return false;
    }

    if (pdu->message.present != UlpMessage_PR_msSUPLRESPONSE) {
        VERBOSEF("not SUPLRESPONSE: %d", pdu->message.present);
        return false;
    }

    if (!decode_session(set, slp, pdu)) {
        VERBOSEF("decode_session failed");
        return false;
    }

    print(loglet::Level::Verbose, pdu);
    auto message       = &pdu->message.choice.msSUPLRESPONSE;
    response.posMethod = static_cast<RESPONSE::PosMethod>(message->posMethod);
    return true;
}

bool decode_end(Session::SET& set, Session::SLP& slp, END&, ULP_PDU* pdu) {
    VSCOPE_FUNCTION();

    if (!pdu) {
        VERBOSEF("pdu is null");
        return false;
    }

    if (pdu->message.present != UlpMessage_PR_msSUPLEND) {
        VERBOSEF("not SUPLEND: %d", pdu->message.present);
        return false;
    }

    if (!decode_session(set, slp, pdu)) {
        VERBOSEF("decode_session failed");
        return false;
    }

    print(loglet::Level::Verbose, pdu);
    return true;
}

Payload decode_payload(Payload::Type type, const OCTET_STRING& data) {
    VSCOPE_FUNCTION();
    Payload payload{};
    payload.type = type;
    payload.data.resize(data.size);
    memcpy(payload.data.data(), data.buf, data.size);
    return payload;
}

bool decode_pos(Session::SET& set, Session::SLP& slp, POS& pos, ULP_PDU* pdu) {
    VSCOPE_FUNCTION();

    if (!pdu) {
        VERBOSEF("pdu is null");
        return false;
    }

    if (pdu->message.present != UlpMessage_PR_msSUPLPOS) {
        VERBOSEF("not SUPLPOS: %d", pdu->message.present);
        return false;
    }

    if (!decode_session(set, slp, pdu)) {
        VERBOSEF("decode_session failed");
        return false;
    }

    auto message = &pdu->message.choice.msSUPLPOS;
    if (message->posPayLoad.present == PosPayLoad_PR_tia801payload) {
        pos.payloads.emplace_back(
            decode_payload(Payload::Type::TIA801, message->posPayLoad.choice.rrlpPayload));
    } else if (message->posPayLoad.present == PosPayLoad_PR_rrcPayload) {
        pos.payloads.emplace_back(
            decode_payload(Payload::Type::RRC, message->posPayLoad.choice.rrlpPayload));
    } else if (message->posPayLoad.present == PosPayLoad_PR_rrlpPayload) {
        pos.payloads.emplace_back(
            decode_payload(Payload::Type::RRLP, message->posPayLoad.choice.rrlpPayload));
    } else if (message->posPayLoad.present == PosPayLoad_PR_ver2_PosPayLoad_extension) {
        auto ver2 = &message->posPayLoad.choice.ver2_PosPayLoad_extension;
        if (ver2->lPPPayload) {
            for (int i = 0; i < ver2->lPPPayload->list.count; i++) {
                if (ver2->lPPPayload->list.array[i] != nullptr) {
                    pos.payloads.emplace_back(
                        decode_payload(Payload::Type::LPP, *ver2->lPPPayload->list.array[i]));
                }
            }
        }

        if (ver2->tIA801Payload) {
            for (int i = 0; i < ver2->tIA801Payload->list.count; i++) {
                if (ver2->tIA801Payload->list.array[i] != nullptr) {
                    pos.payloads.emplace_back(
                        decode_payload(Payload::Type::TIA801, *ver2->tIA801Payload->list.array[i]));
                }
            }
        }
    }

    print(loglet::Level::Verbose, pdu);
    return true;
}

}  // namespace supl
