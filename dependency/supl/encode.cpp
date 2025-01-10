#include "encode.hpp"
#include "print.hpp"
#include "supl.hpp"

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
#include <OCTET_STRING.h>
#include <PosProtocolVersion3GPP.h>
#include <PosProtocolVersion3GPP2.h>
#include <SETId.h>
#include <SessionID.h>
#include <SetSessionID.h>
#include <SlpSessionID.h>
#include <ULP-PDU.h>
#include <UTCTime.h>
#include <Ver2-PosProtocol-extension.h>
#include <per_encoder.h>
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>
#include <asn.1/helper.hpp>

namespace supl {

// NOTE: Empty encode used for determine the final length of the ULP/PDU.
static int encode_to_length_cb(void const*, size_t, void*) {
    return 0;
}

static asn_enc_rval_t uper_encode_to_length(asn_TYPE_descriptor_t const* td,
                                            asn_per_constraints_t const* constraints,
                                            void const*                  sptr) {
    return uper_encode(td, constraints, sptr, encode_to_length_cb, NULL);
}

static EncodedMessage encode_uper(ULP_PDU* pdu) {
    // Determine the PDU length
    auto result = uper_encode_to_length(&asn_DEF_ULP_PDU, nullptr, pdu);
    if (result.encoded < 0) {
        return {};
    }

    auto length  = static_cast<size_t>((result.encoded + 7) / 8);
    auto buffer  = new uint8_t[length];
    auto message = EncodedMessage{buffer, length};

    pdu->length = length;

    // Encode PDU as UPER
    result = uper_encode_to_buffer(&asn_DEF_ULP_PDU, nullptr, pdu, buffer, length);
    if (result.encoded < 0) {
        return {};
    }

    // Make sure that the PDU stayed intact and that length wasn't adjusted.
    auto new_length = static_cast<size_t>((result.encoded + 7) / 8);
    if (length != new_length) {
        return {};
    }

    return message;
}

//
//
//

static ULP_PDU* create_message(UlpMessage_PR present, Version version) {
    auto message             = (ULP_PDU*)calloc(1, sizeof(ULP_PDU));
    message->length          = 0;
    message->version.maj     = version.major;
    message->version.min     = version.minor;
    message->version.servind = version.servind;

    message->message.present = present;
    return message;
}

static OCTET_STRING binary_encoded_octet(size_t max_length, int64_t from) {
    OCTET_STRING octet{};
    octet.size = max_length;
    octet.buf  = (uint8_t*)calloc(1, octet.size);
    memset(octet.buf, 0xFF, octet.size);

    auto index = 0;
    while (from > 0) {
        auto value = static_cast<uint8_t>((from % 10) & 0xF);
        from /= 10;

        auto byte_index = 8 - 1 - (index / 2);
        if (index % 2 == 0) {
            octet.buf[byte_index] &= 0xF0;
            octet.buf[byte_index] |= value;
        } else {
            octet.buf[byte_index] &= 0x0F;
            octet.buf[byte_index] |= value << 4;
        }

        index++;
    }

    return octet;
}

static OCTET_STRING octet_string_from(uint8_t const* data, size_t size) {
    OCTET_STRING octet{};
    octet.size = size;
    octet.buf  = (uint8_t*)calloc(1, octet.size);
    memcpy(octet.buf, data, size);
    return octet;
}

static IPAddress encode_ipaddress(Identity identity) {
    if (identity.type == Identity::Type::IPV4) {
        IPAddress result{};
        result.present            = IPAddress_PR_ipv4Address;
        result.choice.ipv4Address = octet_string_from(identity.data.ipv4, 4);
        return result;
    } else if (identity.type == Identity::Type::IPV6) {
        IPAddress result{};
        result.present            = IPAddress_PR_ipv6Address;
        result.choice.ipv4Address = octet_string_from(identity.data.ipv6, 16);
        return result;
    }

    UNREACHABLE();
    return {};
}

static SETId encode_setid(Identity identity) {
    switch (identity.type) {
    case Identity::Type::MSISDN: {
        SETId result{};
        result.present       = SETId_PR_msisdn;
        result.choice.msisdn = binary_encoded_octet(8, identity.data.msisdn);
        return result;
    }

    case Identity::Type::IMSI: {
        SETId result{};
        result.present     = SETId_PR_imsi;
        result.choice.imsi = binary_encoded_octet(8, identity.data.imsi);
        return result;
    }

    case Identity::Type::IPV6:
    case Identity::Type::IPV4: {
        SETId result{};
        result.present          = SETId_PR_iPAddress;
        result.choice.iPAddress = encode_ipaddress(identity);
        return result;
    }

    default: UNREACHABLE(); return {};
    }
}

static SLPAddress encode_slp_address(Identity identity) {
    switch (identity.type) {
    case Identity::Type::IPV6:
    case Identity::Type::IPV4: {
        SLPAddress result{};
        result.present          = SLPAddress_PR_iPAddress;
        result.choice.iPAddress = encode_ipaddress(identity);
        return result;
    }

    case Identity::Type::FQDN: {
        SLPAddress result{};
        result.present = SLPAddress_PR_fQDN;
        result.choice.fQDN =
            octet_string_from(reinterpret_cast<uint8_t const*>(identity.data.fQDN.data()),
                              static_cast<int>(identity.data.fQDN.size()));
        return result;
    }

    default: assert(false); return {};
    }
}

static MCC* encode_mcc(int64_t mcc_value) {
    assert(mcc_value >= 0 && mcc_value <= 999);

    char tmp[8];
    sprintf(tmp, "%03ld", mcc_value);

    auto mcc = helper::asn1_allocate<MCC>();
    for (size_t i = 0; i < strlen(tmp); i++) {
        auto d = helper::asn1_allocate<MCC_MNC_Digit_t>();
        *d     = tmp[i] - '0';
        ASN_SEQUENCE_ADD(mcc, d);
    }

    return mcc;
}

static MNC encode_mnc(int64_t mnc_value) {
    assert(mnc_value >= 0 && mnc_value <= 999);

    char tmp[8];
    sprintf(tmp, "%02ld", mnc_value);

    MNC mnc{};
    for (size_t i = 0; i < strlen(tmp); i++) {
        auto d = helper::asn1_allocate<MCC_MNC_Digit_t>();
        *d     = tmp[i] - '0';
        ASN_SEQUENCE_ADD(&mnc, d);
    }

    return mnc;
}

static CellGlobalIdEUTRA_t encode_cellGlobalIdEUTRA(int64_t mcc, int64_t mnc, int64_t ci) {
    CellIdentity_t cellIdentity{};
    helper::BitStringBuilder{}.integer(0, 28, ci).into_bit_string(28, &cellIdentity);

    CellGlobalIdEUTRA_t result{};
    result.plmn_Identity.mcc = encode_mcc(mcc);
    result.plmn_Identity.mnc = encode_mnc(mnc);
    result.cellIdentity      = cellIdentity;
    return result;
}

static PhysCellId_t encode_physCellId(int64_t id) {
    return static_cast<long>(id);
}

static TrackingAreaCode_t encode_trackingAreaCode(int64_t tac) {
    TrackingAreaCode_t tracking_area_code{};
    helper::BitStringBuilder{}.integer(0, 16, tac).into_bit_string(16, &tracking_area_code);
    return tracking_area_code;
}

static CellInfo encode_cellinfo(Cell cell) {
    if (cell.type == Cell::Type::GSM) {
        // TODO: Unsupported
        UNIMPLEMENTED("GSM is not supported");
    } else if (cell.type == Cell::Type::LTE) {
        CellInfo result{};
        result.present                                = CellInfo_PR_ver2_CellInfo_extension;
        result.choice.ver2_CellInfo_extension.present = Ver2_CellInfo_extension_PR_lteCell;

        auto& lte_cell = result.choice.ver2_CellInfo_extension.choice.lteCell;
        lte_cell.cellGlobalIdEUTRA =
            encode_cellGlobalIdEUTRA(cell.data.lte.mcc, cell.data.lte.mnc, cell.data.lte.ci);
        lte_cell.physCellId       = encode_physCellId(cell.data.lte.phys_id);
        lte_cell.trackingAreaCode = encode_trackingAreaCode(cell.data.lte.tac);
        return result;
    } else if (cell.type == Cell::Type::NR) {
        // TODO: Unsupported
        UNIMPLEMENTED("NR is not supported");
    }

    assert(false);
    return {};
}

static void encode_session(ULP_PDU* pdu, Session::SET& set, Session::SLP& slp) {
    if (set.is_active) {
        auto setSessionID           = (SetSessionID*)calloc(1, sizeof(SetSessionID));
        setSessionID->sessionId     = set.id;
        setSessionID->setId         = encode_setid(set.identity);
        pdu->sessionID.setSessionID = setSessionID;
    }

    if (slp.is_active) {
        auto slpSessionID           = (SlpSessionID*)calloc(1, sizeof(SlpSessionID));
        slpSessionID->sessionID     = octet_string_from(slp.id, 4);
        slpSessionID->slpId         = encode_slp_address(slp.identity);
        pdu->sessionID.slpSessionID = slpSessionID;
    }
}

static ::SETCapabilities encode_setcapabilities(supl::SETCapabilities const& sETCapabilities) {
    ::PosProtocol posProtocol{};
    posProtocol.ver2_PosProtocol_extension = helper::asn1_allocate<Ver2_PosProtocol_extension>();

    auto& rrc = sETCapabilities.posProtocol.rrc;
    if (rrc.enabled) {
        auto version                   = helper::asn1_allocate<PosProtocolVersion3GPP>();
        version->majorVersionField     = rrc.majorVersionField;
        version->technicalVersionField = rrc.technicalVersionField;
        version->editorialVersionField = rrc.editorialVersionField;

        posProtocol.rrc                                               = true;
        posProtocol.ver2_PosProtocol_extension->posProtocolVersionRRC = version;
    }

    auto& rrlp = sETCapabilities.posProtocol.rrlp;
    if (rrlp.enabled) {
        auto version                   = helper::asn1_allocate<PosProtocolVersion3GPP>();
        version->majorVersionField     = rrlp.majorVersionField;
        version->technicalVersionField = rrlp.technicalVersionField;
        version->editorialVersionField = rrlp.editorialVersionField;

        posProtocol.rrlp                                               = true;
        posProtocol.ver2_PosProtocol_extension->posProtocolVersionRRLP = version;
    }

    auto& lpp = sETCapabilities.posProtocol.lpp;
    if (lpp.enabled) {
        auto version                   = helper::asn1_allocate<PosProtocolVersion3GPP>();
        version->majorVersionField     = lpp.majorVersionField;
        version->technicalVersionField = lpp.technicalVersionField;
        version->editorialVersionField = lpp.editorialVersionField;

        posProtocol.ver2_PosProtocol_extension->lpp                   = true;
        posProtocol.ver2_PosProtocol_extension->posProtocolVersionLPP = version;
    }

    ::SETCapabilities result{};
    result.posTechnology.agpsSETassisted = sETCapabilities.posTechnology.agpsSETassisted;
    result.posTechnology.agpsSETBased    = sETCapabilities.posTechnology.agpsSETBased;
    result.posTechnology.autonomousGPS   = sETCapabilities.posTechnology.autonomousGPS;
    result.posTechnology.aFLT            = sETCapabilities.posTechnology.aFLT;
    result.posTechnology.eCID            = sETCapabilities.posTechnology.eCID;
    result.posTechnology.eOTD            = sETCapabilities.posTechnology.eOTD;
    result.posTechnology.oTDOA           = sETCapabilities.posTechnology.oTDOA;
    result.prefMethod                    = static_cast<PrefMethod_t>(sETCapabilities.prefMethod);
    result.posProtocol                   = posProtocol;
    return result;
}

static ::LocationId encode_locationid(supl::LocationID const& locationID) {
    ::LocationId result{};
    result.cellInfo = encode_cellinfo(locationID.cell);
    result.status   = Status_current;
    return result;
}

static ::PosPayLoad encode_pospayload(std::vector<supl::Payload> const& payloads) {
    auto lpppayload =
        helper::asn1_allocate<Ver2_PosPayLoad_extension::Ver2_PosPayLoad_extension__lPPPayload>();
    asn_sequence_empty(&lpppayload->list);
    for (auto& payload : payloads) {
        auto os   = helper::asn1_allocate<OCTET_STRING>();
        auto data = (uint8_t*)malloc(payload.data.size());
        memcpy(data, payload.data.data(), payload.data.size());
        os->buf  = data;
        os->size = (int)payload.data.size();
        asn_sequence_add(&lpppayload->list, os);
    }

    ::PosPayLoad pos_payload{};
    pos_payload.present = PosPayLoad_PR_ver2_PosPayLoad_extension;
    pos_payload.choice.ver2_PosPayLoad_extension.lPPPayload = lpppayload;
    return pos_payload;
}

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp, const START& message) {
    VSCOPE_FUNCTIONF("START");

    auto ulp_pdu = create_message(UlpMessage_PR_msSUPLSTART, version);
    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    encode_session(ulp_pdu, set, slp);

    auto& pdu_message           = ulp_pdu->message.choice.msSUPLSTART;
    pdu_message.sETCapabilities = encode_setcapabilities(message.sETCapabilities);
    pdu_message.locationId      = encode_locationid(message.locationID);

    print(loglet::Level::Verbose, ulp_pdu);
    return encode_uper(ulp_pdu);
}

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp,
                      const POSINIT& message) {
    VSCOPE_FUNCTIONF("POSINIT");

    auto ulp_pdu = create_message(UlpMessage_PR_msSUPLPOSINIT, version);
    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    encode_session(ulp_pdu, set, slp);

    auto& pdu_message           = ulp_pdu->message.choice.msSUPLPOSINIT;
    pdu_message.sETCapabilities = encode_setcapabilities(message.sETCapabilities);
    pdu_message.locationId      = encode_locationid(message.locationID);

    print(loglet::Level::Verbose, ulp_pdu);
    return encode_uper(ulp_pdu);
}

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp, const POS& message) {
    VSCOPE_FUNCTIONF("POS");

    auto ulp_pdu = create_message(UlpMessage_PR_msSUPLPOS, version);
    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    encode_session(ulp_pdu, set, slp);

    auto& pdu_message      = ulp_pdu->message.choice.msSUPLPOS;
    pdu_message.posPayLoad = encode_pospayload(message.payloads);

    print(loglet::Level::Verbose, ulp_pdu);
    return encode_uper(ulp_pdu);
}

}  // namespace supl
