#include "supl.h"

#include <utility/cpp.h>

template <>
void ASN_Deleter<ULP_PDU>::operator()(ULP_PDU* ptr) {
    if (ptr) {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ptr);
    }
}

template <>
void ASN_Deleter<OCTET_STRING>::operator()(OCTET_STRING* ptr) {
    if (ptr) {
        ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, ptr);
    }
}

//
//
//

std::unique_ptr<SUPL_Session> SUPL_Session::ip_address(long id, uint32_t addr) {
    auto set           = ALLOC_ZERO(SetSessionID);
    set->sessionId     = id;
    set->setId.present = SETId_PR_iPAddress;

    auto ip     = &set->setId.choice.iPAddress;
    ip->present = IPAddress_PR_ipv4Address;
    OCTET_STRING_fromBuf(&ip->choice.ipv4Address, reinterpret_cast<char*>(&addr), sizeof(addr));

    return std::unique_ptr<SUPL_Session>(new SUPL_Session(set, nullptr));
}

template <typename T>
static T* deepcopy_asn1_object(const asn_TYPE_descriptor_s* descriptor, T* value) {
    if (!value) {
        return nullptr;
    }

    // Deep copy achieved by encoding and decoding the structure.
    auto buffer = (void*)nullptr;
    auto length = uper_encode_to_new_buffer(descriptor, NULL, value, &buffer);
    if (length <= 0) {
        return nullptr;
    }

    auto copy = (T*)nullptr;
    auto result =
        uper_decode_complete(0, descriptor, reinterpret_cast<void**>(&copy), buffer, length);
    if (result.code != RC_OK) {
        free(buffer);
        return nullptr;
    }

    free(buffer);
    return copy;
}

SetSessionID* SUPL_Session::copy_set() {
    return deepcopy_asn1_object(&asn_DEF_SetSessionID, mSet);
}

SlpSessionID* SUPL_Session::copy_slp() {
    return deepcopy_asn1_object(&asn_DEF_SlpSessionID, mSlp);
}

void SUPL_Session::harvest(SUPL_Message& message) {
    if (!mSlp) {
        if (message->sessionID.slpSessionID) {
            mSlp = deepcopy_asn1_object(&asn_DEF_SlpSessionID, message->sessionID.slpSessionID);
        }
    }
}

SUPL_Session::~SUPL_Session() {
    if (mSet) {
        ASN_STRUCT_FREE(asn_DEF_SetSessionID, mSet);
    }

    if (mSlp) {
        ASN_STRUCT_FREE(asn_DEF_SlpSessionID, mSlp);
    }
}

//
//
//

SUPL_Client::SUPL_Client(std::unique_ptr<SUPL_Session> session)
    : mTCP(std::make_unique<TCP_Client>()), mSession(std::move(session)) {}

SUPL_Client::~SUPL_Client() {}

bool SUPL_Client::connect(const std::string& host, int port, bool use_ssl) {
    return mTCP->connect(host, port, use_ssl);
}

bool SUPL_Client::disconnect() {
    return mTCP->disconnect();
}

bool SUPL_Client::is_connected() {
    return mTCP->is_connected();
}

SUPL_Message SUPL_Client::receive(int milliseconds) {
    if (!mTCP->is_connected()) {
        return nullptr;
    }

    auto bytes = mTCP->receive(mReceiveBuffer, 16, milliseconds);
    if (bytes <= 0) {
        return nullptr;
    }

    auto pdu  = (ULP_PDU*)nullptr;
    auto size = bytes;

    // NOTE: Some SUPL messages are very big, e.g. supl.google.com SUPLPOS with
    // ephemeris. This means that sometimes the 'buffer' will not contain the
    // whole message but only apart of it. This means we need to wait for the
    // rest of the data. Try to decode the message and if it failed with
    // RC_WMORE wait for more data and try again.
    auto result = uper_decode_complete(0, &asn_DEF_ULP_PDU, (void**)&pdu, mReceiveBuffer, bytes);
    if (result.code == RC_FAIL) {
        return nullptr;
    }

    auto expected_size = pdu->length;
    if (expected_size > sizeof(mReceiveBuffer)) {
        return nullptr;
    }

    if (result.code == RC_WMORE) {
        while (size < expected_size) {
            bytes = mTCP->receive(mReceiveBuffer + size, expected_size - size, -1);
            if (bytes <= 0) {
                return nullptr;
            }

            size += bytes;
        }

        result = uper_decode_complete(0, &asn_DEF_ULP_PDU, (void**)&pdu, mReceiveBuffer, size);
        if (result.code != RC_OK) {
            return nullptr;
        }
    }

    return ASN_Unique<ULP_PDU>(pdu, {});
}

static int encode_to_length_cb(const void* buffer, size_t size, void* key) {
    return 0;
}

static asn_enc_rval_t uper_encode_to_length(const asn_TYPE_descriptor_t* td,
                                            const asn_per_constraints_t* constraints,
                                            const void*                  sptr) {
    return uper_encode(td, constraints, sptr, encode_to_length_cb, NULL);
}

SUPL_EncodedMessage SUPL_Client::encode(SUPL_Message& message) {
    auto pdu = message.get();

    // Determine the PDU length
    auto result = uper_encode_to_length(&asn_DEF_ULP_PDU, NULL, pdu);
    if (result.encoded < 0) {
        return nullptr;
    }

    auto length = (result.encoded + 7) / 8;
    auto buffer = (uint8_t*)calloc(length, sizeof(uint8_t));

    auto octet_string   = (OCTET_STRING*)calloc(1, sizeof(OCTET_STRING));
    octet_string->buf   = buffer;
    octet_string->size  = length;
    auto encode_message = ASN_Unique<OCTET_STRING>(octet_string, {});

    pdu->length = length;

    // Encode PDU as UPER
    result = uper_encode_to_buffer(&asn_DEF_ULP_PDU, NULL, pdu, buffer, length);
    if (result.encoded < 0) {
        return nullptr;
    }

    // Make sure that the PDU stayed intact and that length wasn't adjusted.
    if (length != (result.encoded + 7) / 8) {
        return nullptr;
    }

    return encode_message;
}

bool SUPL_Client::send(SUPL_Message& message) {
    if (!mTCP->is_connected()) {
        return false;
    }

    auto encoded_message = encode(message);
    if (!encoded_message) {
        return false;
    }

    auto bytes = mTCP->send(encoded_message->buf, encoded_message->size);
    if (bytes != encoded_message->size) {
        return false;
    }

    return true;
}

void SUPL_Client::harvest(SUPL_Message& message) {
    if (mSession) {
        mSession->harvest(message);
    }
}

SUPL_Message SUPL_Client::create_message(UlpMessage_PR present) {
    auto ptr     = ALLOC_ZERO(ULP_PDU);
    auto message = ASN_Unique<ULP_PDU>(ptr, {});

    message->length          = 0;
    message->version.maj     = 2;
    message->version.min     = 1;
    message->version.servind = 0;

    message->message.present = present;

    message->sessionID.setSessionID = mSession->copy_set();
    message->sessionID.slpSessionID = mSession->copy_slp();

    return message;
}
