#include "supl.h"

#include <SessionID.h>
#include <SetSessionID.h>
#include <SlpSessionID.h>

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

static void binary_coded_decimal(unsigned long long value, unsigned char* buf, bool switch_order) {
    // `switch_order` = false, this will encode it in the wrong order

    // msisdn, mnd and imsi are a BCD (Binary Coded Decimal) string
    // represent digits from 0 through 9,
    // two digits per octet, each digit encoded 0000 to 1001 (0 to 9)
    // bits 8765 of octet n encoding digit 2n
    // bits 4321 of octet n encoding digit 2(n-1) +1
    // not used digits in the string shall be filled with 1111
    unsigned char digits[16];
    int           i = 0;
    while (value > 0) {
        if (i >= 16) {
            break;
        }
        digits[i] = value % 10;
        value /= 10;
        i++;
    }

    for (int j = 0; j < 8; j++) {
        buf[j] = 0;
    }

    int j = 0;
    while (i > 0) {
        if (j % 2 == (switch_order ? 1 : 0)) {
            buf[j / 2] |= digits[i - 1] << 4;
        } else {
            buf[j / 2] |= digits[i - 1];
        }
        i--;
        j++;
    }

    while (j < 16) {
        if (j % 2 == 0) {
            buf[j / 2] |= 0xf0;
        } else {
            buf[j / 2] |= 0xf;
        }
        j++;
    }
}

std::unique_ptr<SUPL_Session> SUPL_Session::msisdn(long id, unsigned long long msisdn,
                                                   bool switch_order) {
    auto set           = ALLOC_ZERO(SetSessionID);
    set->sessionId     = id;
    set->setId.present = SETId_PR_msisdn;

    unsigned char buf[8];
    binary_coded_decimal(msisdn, buf, switch_order);
    OCTET_STRING_fromBuf(&set->setId.choice.msisdn, reinterpret_cast<char*>(buf), sizeof(buf));

    return std::unique_ptr<SUPL_Session>(new SUPL_Session(set, nullptr));
}

std::unique_ptr<SUPL_Session> SUPL_Session::imsi(long id, unsigned long long imsi,
                                                 bool switch_order) {
    auto set           = ALLOC_ZERO(SetSessionID);
    set->sessionId     = id;
    set->setId.present = SETId_PR_imsi;

    unsigned char buf[8];
    binary_coded_decimal(imsi, buf, switch_order);
    OCTET_STRING_fromBuf(&set->setId.choice.imsi, reinterpret_cast<char*>(buf), sizeof(buf));

    return std::unique_ptr<SUPL_Session>(new SUPL_Session(set, nullptr));
}

std::unique_ptr<SUPL_Session> SUPL_Session::ip_address(long id, std::string const& addr_str) {
    int  b[4];
    auto result = sscanf(addr_str.c_str(), "%d.%d.%d.%d", b, b + 1, b + 2, b + 3);
    assert(result == 4);

    uint32_t addr = (static_cast<uint32_t>(b[0]) << 24) | (static_cast<uint32_t>(b[1]) << 16) |
                    (static_cast<uint32_t>(b[2]) << 8) | static_cast<uint32_t>(b[3]);
    return ip_address(id, addr);
}

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
static T* deepcopy_asn1_object(asn_TYPE_descriptor_s const* descriptor, T* value) {
    if (!value) {
        return nullptr;
    }

    // Deep copy achieved by encoding and decoding the structure.
    void* buffer = nullptr;
    auto  length = uper_encode_to_new_buffer(descriptor, NULL, value, &buffer);
    if (length <= 0) {
        return nullptr;
    }

    T*   copy = nullptr;
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

SUPL_Client::SUPL_Client()
    : mTCP(std::unique_ptr<TCP_Client>(new TCP_Client())), mSession(nullptr), mReceiveLength(0) {}

SUPL_Client::~SUPL_Client() {}

void SUPL_Client::set_session(std::unique_ptr<SUPL_Session> session) {
    mSession = std::move(session);
}

bool SUPL_Client::connect(std::string const& host, int port, bool use_ssl) {
    if (!mSession) {
        printf("ERROR: Missing SUPL session\n");
        return false;
    } else {
        return mTCP->connect(host, port, use_ssl);
    }
}

bool SUPL_Client::disconnect() {
    return mTCP->disconnect();
}

bool SUPL_Client::is_connected() {
    return mTCP->is_connected();
}

SUPL_Message SUPL_Client::process() {
    if (mReceiveLength < 16) {
        return nullptr;
    }

    ULP_PDU* pdu           = nullptr;
    size_t   expected_size = 0;

    // NOTE: Some SUPL messages are very big, e.g. supl.google.com SUPLPOS with
    // ephemeris. This means that sometimes the 'buffer' will not contain the
    // whole message but only apart of it. This means we need to wait for the
    // rest of the data. Try to decode the message and if it failed with
    // RC_WMORE wait for more data and try again.
    auto result = uper_decode_complete(0, &asn_DEF_ULP_PDU, reinterpret_cast<void**>(&pdu),
                                       mReceiveBuffer, mReceiveLength);
    if (result.code == RC_FAIL) {
        mReceiveLength = 0;
        printf("failed to decode UPER\n");
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, pdu);
        return nullptr;
    } else if (result.code == RC_WMORE) {
        expected_size = static_cast<size_t>(pdu->length);
        printf("wait for more data: %zu\n", expected_size);
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, pdu);
        pdu = nullptr;
        if (expected_size > SUPL_CLIENT_RECEIVER_BUFFER_SIZE) {
            // Unable to handle such big messages
            mReceiveLength = 0;
            printf("too big message, unsupported\n");
            return nullptr;
        } else if (expected_size > mReceiveLength) {
            // Not enough data
            printf("wait for more data\n");
            return nullptr;
        } else {
            // We have enough data, but the decoding it failed?
            printf("have enough data, but failed to decode\n");
            mReceiveLength = 0;
            return nullptr;
        }
    } else {
        expected_size = static_cast<size_t>(pdu->length);
        printf("success: %zu (%zu)\n", expected_size, result.consumed);
    }

    printf("receive-length: %zu, expected-size: %zu\n", mReceiveLength, expected_size);
    if (expected_size > mReceiveLength) {
        mReceiveLength = 0;
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, pdu);
        return nullptr;
    }

    memmove(mReceiveBuffer, mReceiveBuffer + expected_size, mReceiveLength - expected_size);
    mReceiveLength -= expected_size;

    // xer_fprint(stdout, &asn_DEF_ULP_PDU, pdu);
    return ASN_Unique<ULP_PDU>(pdu, {});
}

SUPL_Message SUPL_Client::receive2() {
    if (!mTCP->is_connected()) {
        return nullptr;
    }

    // Try to parse the message from the buffer
    auto message = process();
    if (message) {
        return message;
    }

    // If the buffer is full, clear it
    if (mReceiveLength >= SUPL_CLIENT_RECEIVER_BUFFER_SIZE) {
        mReceiveLength = 0;
    }

    // Receive more data
    auto bytes =
        mTCP->receive(mReceiveBuffer + mReceiveLength,
                      static_cast<int>(SUPL_CLIENT_RECEIVER_BUFFER_SIZE - mReceiveLength), 0);
    if (bytes <= 0) {
        printf("ERROR: Failed to receive data\n");
        return nullptr;
    }

    mReceiveLength += static_cast<size_t>(bytes);
    // This should never happen, if we receive more data than the buffer can then
    // mTcp->receive will have overflowed the buffer.
    if (mReceiveLength > SUPL_CLIENT_RECEIVER_BUFFER_SIZE) {
        printf("ERROR: Receive buffer full (%zu >= %d)\n", mReceiveLength,
               SUPL_CLIENT_RECEIVER_BUFFER_SIZE);
        mReceiveLength = 0;
        return nullptr;
    }

    // Try again to parse the message from the buffer
    message = process();
    if (message) {
        return message;
    }

    return nullptr;
}

SUPL_Message SUPL_Client::receive(int milliseconds) {
    if (!mTCP->is_connected()) {
        return nullptr;
    }

    auto bytes = mTCP->receive(mReceiveBuffer, 16, milliseconds);
    if (bytes <= 0) {
        return nullptr;
    }

    ULP_PDU* pdu  = nullptr;
    auto     size = bytes;

    // NOTE: Some SUPL messages are very big, e.g. supl.google.com SUPLPOS with
    // ephemeris. This means that sometimes the 'buffer' will not contain the
    // whole message but only apart of it. This means we need to wait for the
    // rest of the data. Try to decode the message and if it failed with
    // RC_WMORE wait for more data and try again.
    auto result = uper_decode_complete(0, &asn_DEF_ULP_PDU, reinterpret_cast<void**>(&pdu),
                                       mReceiveBuffer, bytes);
    if (result.code == RC_FAIL) {
        return nullptr;
    }

    auto expected_size = static_cast<int>(pdu->length);
    if (expected_size > SUPL_CLIENT_RECEIVER_BUFFER_SIZE) {
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

        result = uper_decode_complete(0, &asn_DEF_ULP_PDU, reinterpret_cast<void**>(&pdu),
                                      mReceiveBuffer, size);
        if (result.code != RC_OK) {
            return nullptr;
        }
    }

    // xer_fprint(stdout, &asn_DEF_ULP_PDU, pdu);
    return ASN_Unique<ULP_PDU>(pdu, {});
}

static int encode_to_length_cb(void const*, size_t, void*) {
    return 0;
}

static asn_enc_rval_t uper_encode_to_length(asn_TYPE_descriptor_t const* td,
                                            asn_per_constraints_t const* constraints,
                                            void const*                  sptr) {
    return uper_encode(td, constraints, sptr, encode_to_length_cb, NULL);
}

SUPL_EncodedMessage SUPL_Client::encode(SUPL_Message& message) {
    auto pdu = message.get();

    // xer_fprint(stdout, &asn_DEF_ULP_PDU, pdu);

    // Determine the PDU length
    auto result = uper_encode_to_length(&asn_DEF_ULP_PDU, NULL, pdu);
    if (result.encoded < 0) {
        printf("ERROR: Failed to determine PDU length\n");
        printf("ERROR: %s\n", result.failed_type->name);
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
        printf("ERROR: Failed to encode PDU\n");
        return nullptr;
    }

    // Make sure that the PDU stayed intact and that length wasn't adjusted.
    if (length != (result.encoded + 7) / 8) {
        printf("ERROR: PDU length mismatch\n");
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
        printf("ERROR: Failed to encode message\n");
        return false;
    }

    auto bytes = mTCP->send(encoded_message->buf, encoded_message->size);
    if (bytes != static_cast<int>(encoded_message->size)) {
        printf("ERROR: Failed to send message\n");
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
