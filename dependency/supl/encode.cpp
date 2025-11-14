#include "encode.hpp"
#include "print.hpp"
#include "supl.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <ApplicationID.h>
#include <FQDN.h>
#include <MCC-MNC-Digit.h>
#include <MCC.h>
#include <MNC.h>
#include <OCTET_STRING.h>
#include <PosProtocolVersion3GPP.h>
#include <PosProtocolVersion3GPP2.h>
#include <SETId.h>
#include <ServCellNR.h>
#include <SessionID.h>
#include <SetSessionID.h>
#include <SlpSessionID.h>
#include <ULP-PDU.h>
#include <UTCTime.h>
#include <Ver2-PosProtocol-extension.h>
#include <Ver2-SUPL-START-extension.h>
#include <per_encoder.h>
EXTERNAL_WARNINGS_POP

#include <asn.1/bit_string.hpp>
#include <asn.1/helper.hpp>

LOGLET_MODULE2(supl, encode);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(supl, encode)

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
    VSCOPE_FUNCTION();

    auto result = uper_encode_to_length(&asn_DEF_ULP_PDU, nullptr, pdu);
    if (result.encoded < 0) {
        VERBOSEF("uper_encode_to_length failed: %ld", result.encoded);
        return {};
    }

    auto length  = static_cast<size_t>((result.encoded + 7) / 8);
    auto buffer  = new uint8_t[length];
    auto message = EncodedMessage{buffer, length};

    pdu->length = static_cast<long>(length);

    result = uper_encode_to_buffer(&asn_DEF_ULP_PDU, nullptr, pdu, buffer, length);
    if (result.encoded < 0) {
        VERBOSEF("uper_encode_to_buffer failed: %ld", result.encoded);
        return {};
    }

    auto new_length = static_cast<size_t>((result.encoded + 7) / 8);
    if (length != new_length) {
        VERBOSEF("length mismatch: %zu != %zu", length, new_length);
        return {};
    }

    return message;
}

//
//
//

static ULP_PDU* create_message(UlpMessage_PR present, Version version) {
    VSCOPE_FUNCTION();
    auto message = reinterpret_cast<ULP_PDU*>(calloc(1, sizeof(ULP_PDU)));
    ASSERT(message, "out of memory");
    message->length          = 0;
    message->version.maj     = version.major;
    message->version.min     = version.minor;
    message->version.servind = version.servind;

    message->message.present = present;
    return message;
}

static OCTET_STRING binary_encoded_octet(size_t max_length, uint64_t from) {
    VSCOPE_FUNCTION();
    OCTET_STRING octet{};
    octet.size = max_length;
    octet.buf  = reinterpret_cast<uint8_t*>(calloc(1, octet.size));
    ASSERT(octet.buf, "out of memory");
    memset(octet.buf, 0xFF, octet.size);

    size_t length = 0;
    auto   data   = from;
    while (data > 0) {
        data /= 10;
        length++;
    }

    auto offset = 2 * max_length - length;
    auto index  = offset;
    data        = from;
    while (data > 0 && index < 2 * max_length) {
        auto value = static_cast<uint8_t>((data % 10) & 0xF);
        data /= 10;

        auto byte_index = max_length - 1 - (index / 2);
        if (index % 2 == 1) {
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
    octet.buf  = reinterpret_cast<uint8_t*>(calloc(1, octet.size));
    ASSERT(octet.buf, "out of memory");
    memcpy(octet.buf, data, size);
    return octet;
}

static IPAddress encode_ipaddress(Identity identity) {
    VSCOPE_FUNCTION();
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
    VSCOPE_FUNCTION();
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

    case Identity::Type::UNKNOWN:
    case Identity::Type::FQDN: break;
    }
    UNREACHABLE();
}

static SLPAddress encode_slp_address(Identity identity) {
    VSCOPE_FUNCTION();
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
        result.present     = SLPAddress_PR_fQDN;
        result.choice.fQDN = octet_string_from(
            reinterpret_cast<uint8_t const*>(identity.data.fqdn.data()), identity.data.fqdn.size());
        return result;
    }

    case Identity::Type::UNKNOWN:
    case Identity::Type::MSISDN:
    case Identity::Type::IMSI: break;
    }

    UNREACHABLE();
}

static MCC* encode_mcc(uint64_t mcc_value) {
    ASSERT(mcc_value <= 999, "mcc_value must be <= 999");

    char tmp[8];
    sprintf(tmp, "%03" PRIu64, mcc_value);

    auto mcc = helper::asn1_allocate<MCC>();
    for (size_t i = 0; i < strlen(tmp); i++) {
        auto d = helper::asn1_allocate<MCC_MNC_Digit_t>();
        *d     = tmp[i] - '0';
        ASN_SEQUENCE_ADD(mcc, d);
    }

    return mcc;
}

static MNC encode_mnc(uint64_t mnc_value) {
    ASSERT(mnc_value <= 999, "mnc_value must be <= 999");

    char tmp[8];
    sprintf(tmp, "%02" PRIu64, mnc_value);

    MNC mnc{};
    for (size_t i = 0; i < strlen(tmp); i++) {
        auto d = helper::asn1_allocate<MCC_MNC_Digit_t>();
        *d     = tmp[i] - '0';
        ASN_SEQUENCE_ADD(&mnc, d);
    }

    return mnc;
}

static CellGlobalIdEUTRA_t encode_cell_global_id_eutra(uint64_t mcc, uint64_t mnc, uint64_t ci) {
    VSCOPE_FUNCTION();
    CellIdentity_t cell_identity{};
    helper::BitStringBuilder{}.integer(0, 28, ci).into_bit_string(28, &cell_identity);

    CellGlobalIdEUTRA_t result{};
    result.plmn_Identity.mcc = encode_mcc(mcc);
    result.plmn_Identity.mnc = encode_mnc(mnc);
    result.cellIdentity      = cell_identity;
    return result;
}

static PhysCellId_t encode_phys_cell_id(uint64_t id) {
    VSCOPE_FUNCTION();
    return static_cast<long>(id);
}

static TrackingAreaCode_t encode_tracking_area_code(uint64_t tac) {
    VSCOPE_FUNCTION();
    TrackingAreaCode_t tracking_area_code{};
    helper::BitStringBuilder{}.integer(0, 16, tac).into_bit_string(16, &tracking_area_code);
    return tracking_area_code;
}

static CellGlobalIdNR_t encode_cell_global_id_nr(uint64_t mcc, uint64_t mnc, uint64_t ci) {
    VSCOPE_FUNCTION();
    CellIdentityNR_t cell_identity{};
    helper::BitStringBuilder{}.integer(0, 36, ci).into_bit_string(36, &cell_identity);

    CellGlobalIdNR_t result{};
    result.plmn_Identity.mcc = encode_mcc(mcc);
    result.plmn_Identity.mnc = encode_mnc(mnc);
    result.cellIdentityNR    = cell_identity;
    return result;
}

static PhysCellIdNR_t encode_phys_cell_id_nr(uint64_t id) {
    return static_cast<long>(id);
}

static TrackingAreaCodeNR_t encode_tracking_area_code_nr(uint64_t tac) {
    VSCOPE_FUNCTION();
    TrackingAreaCodeNR_t tracking_area_code{};
    helper::BitStringBuilder{}.integer(0, 24, tac).into_bit_string(24, &tracking_area_code);
    return tracking_area_code;
}

static CellInfo encode_cellinfo(Cell cell) {
    VSCOPE_FUNCTION();
    CellInfo result{};
    memset(&result, 0, sizeof(CellInfo));

    if (cell.type == Cell::Type::UNKNOWN) {
        result.present = CellInfo_PR_NOTHING;
        return result;
    }

    if (cell.type == Cell::Type::GSM) {
        result.present               = CellInfo_PR_gsmCell;
        result.choice.gsmCell.refMCC = static_cast<long>(cell.data.gsm.mcc);
        result.choice.gsmCell.refMNC = static_cast<long>(cell.data.gsm.mnc);
        result.choice.gsmCell.refLAC = static_cast<long>(cell.data.gsm.lac);
        result.choice.gsmCell.refCI  = static_cast<long>(cell.data.gsm.ci);
        result.choice.gsmCell.tA     = NULL;
        result.choice.gsmCell.nMR    = NULL;
        return result;
    } else if (cell.type == Cell::Type::LTE) {
        result.present                                = CellInfo_PR_ver2_CellInfo_extension;
        result.choice.ver2_CellInfo_extension.present = Ver2_CellInfo_extension_PR_lteCell;

        auto& lte_cell = result.choice.ver2_CellInfo_extension.choice.lteCell;
        lte_cell.cellGlobalIdEUTRA =
            encode_cell_global_id_eutra(cell.data.lte.mcc, cell.data.lte.mnc, cell.data.lte.ci);
        lte_cell.physCellId       = encode_phys_cell_id(cell.data.lte.phys_id);
        lte_cell.trackingAreaCode = encode_tracking_area_code(cell.data.lte.tac);
        return result;
    } else if (cell.type == Cell::Type::NR) {
        result.present                                = CellInfo_PR_ver2_CellInfo_extension;
        result.choice.ver2_CellInfo_extension.present = Ver2_CellInfo_extension_PR_nrCell;

        auto& nr_cell      = result.choice.ver2_CellInfo_extension.choice.nrCell;
        auto& nr_cell_list = nr_cell.servingCellInformation.list;

        auto serv_cell = reinterpret_cast<ServCellNR*>(calloc(1, sizeof(ServCellNR)));
        ASSERT(serv_cell, "out of memory");
        serv_cell->physCellId = encode_phys_cell_id_nr(cell.data.nr.phys_id);
        serv_cell->arfcn_NR   = 0;
        serv_cell->cellGlobalId =
            encode_cell_global_id_nr(cell.data.nr.mcc, cell.data.nr.mnc, cell.data.nr.ci);
        serv_cell->trackingAreaCode = encode_tracking_area_code_nr(cell.data.nr.tac);

        asn_sequence_add(&nr_cell_list, serv_cell);
        return result;
    }

    UNREACHABLE();
}

static void encode_session(ULP_PDU* pdu, Session::SET& set, Session::SLP& slp) {
    VSCOPE_FUNCTION();
    if (set.is_active) {
        auto set_session_id = reinterpret_cast<SetSessionID*>(calloc(1, sizeof(SetSessionID)));
        ASSERT(set_session_id, "out of memory");
        set_session_id->sessionId   = set.id;
        set_session_id->setId       = encode_setid(set.identity);
        pdu->sessionID.setSessionID = set_session_id;
    }

    if (slp.is_active) {
        auto slp_session_id = reinterpret_cast<SlpSessionID*>(calloc(1, sizeof(SlpSessionID)));
        ASSERT(slp_session_id, "out of memory");
        slp_session_id->sessionID   = octet_string_from(slp.id, 4);
        slp_session_id->slpId       = encode_slp_address(slp.identity);
        pdu->sessionID.slpSessionID = slp_session_id;
    }
}

static ::SETCapabilities encode_setcapabilities(supl::SETCapabilities const& set_capabilities) {
    VSCOPE_FUNCTION();
    ::PosProtocol pos_protocol{};
    pos_protocol.ver2_PosProtocol_extension = helper::asn1_allocate<Ver2_PosProtocol_extension>();

    auto& rrc = set_capabilities.pos_protocol.rrc;
    if (rrc.enabled) {
        auto version                   = helper::asn1_allocate<PosProtocolVersion3GPP>();
        version->majorVersionField     = rrc.major_version_field;
        version->technicalVersionField = rrc.technical_version_field;
        version->editorialVersionField = rrc.editorial_version_field;

        pos_protocol.rrc                                               = true;
        pos_protocol.ver2_PosProtocol_extension->posProtocolVersionRRC = version;
    }

    auto& rrlp = set_capabilities.pos_protocol.rrlp;
    if (rrlp.enabled) {
        auto version                   = helper::asn1_allocate<PosProtocolVersion3GPP>();
        version->majorVersionField     = rrlp.major_version_field;
        version->technicalVersionField = rrlp.technical_version_field;
        version->editorialVersionField = rrlp.editorial_version_field;

        pos_protocol.rrlp                                               = true;
        pos_protocol.ver2_PosProtocol_extension->posProtocolVersionRRLP = version;
    }

    auto& lpp = set_capabilities.pos_protocol.lpp;
    if (lpp.enabled) {
        auto version                   = helper::asn1_allocate<PosProtocolVersion3GPP>();
        version->majorVersionField     = lpp.major_version_field;
        version->technicalVersionField = lpp.technical_version_field;
        version->editorialVersionField = lpp.editorial_version_field;

        pos_protocol.ver2_PosProtocol_extension->lpp                   = true;
        pos_protocol.ver2_PosProtocol_extension->posProtocolVersionLPP = version;
    }

    ::SETCapabilities result{};
    result.posTechnology.agpsSETassisted = set_capabilities.pos_technology.agps_set_assisted;
    result.posTechnology.agpsSETBased    = set_capabilities.pos_technology.agps_set_based;
    result.posTechnology.autonomousGPS   = set_capabilities.pos_technology.autonomous_gps;
    result.posTechnology.aFLT            = set_capabilities.pos_technology.aflt;
    result.posTechnology.eCID            = set_capabilities.pos_technology.ecid;
    result.posTechnology.eOTD            = set_capabilities.pos_technology.eotd;
    result.posTechnology.oTDOA           = set_capabilities.pos_technology.otdoa;
    result.prefMethod                    = static_cast<PrefMethod_t>(set_capabilities.pref_method);
    result.posProtocol                   = pos_protocol;
    return result;
}

static ::LocationId encode_locationid(supl::LocationID const& location_id) {
    VSCOPE_FUNCTION();
    ::LocationId result{};
    result.cellInfo = encode_cellinfo(location_id.cell);
    result.status   = Status_current;
    return result;
}

static ::PosPayLoad encode_pospayload(std::vector<supl::Payload> const& payloads) {
    VSCOPE_FUNCTION();
    auto lpppayload =
        helper::asn1_allocate<Ver2_PosPayLoad_extension::Ver2_PosPayLoad_extension__lPPPayload>();
    asn_sequence_empty(&lpppayload->list);
    for (auto& payload : payloads) {
        auto os   = helper::asn1_allocate<OCTET_STRING>();
        auto data = reinterpret_cast<uint8_t*>(malloc(payload.data.size()));
        memcpy(data, payload.data.data(), payload.data.size());
        os->buf  = data;
        os->size = payload.data.size();
        asn_sequence_add(&lpppayload->list, os);
    }

    ::PosPayLoad pos_payload{};
    pos_payload.present = PosPayLoad_PR_ver2_PosPayLoad_extension;
    pos_payload.choice.ver2_PosPayLoad_extension.lPPPayload = lpppayload;
    return pos_payload;
}

static ::ApplicationID* encode_applicationid(ApplicationID const& app_id) {
    VSCOPE_FUNCTION();
    auto application_id = helper::asn1_allocate<::ApplicationID>();

    char app_provider[24 + 1];
    char app_name[32 + 1];
    char app_version[8 + 1];

    if (app_id.provider.size() < 1)
        snprintf(app_provider, sizeof(app_provider), "%s", "unknown");
    else
        snprintf(app_provider, sizeof(app_provider), "%s", app_id.provider.c_str());

    if (app_id.name.size() < 1)
        snprintf(app_name, sizeof(app_name), "%s", "unknown");
    else
        snprintf(app_name, sizeof(app_name), "%s", app_id.name.c_str());

    if (app_id.version.size() < 1)
        snprintf(app_version, sizeof(app_version), "%s", "unknown");
    else
        snprintf(app_version, sizeof(app_version), "%s", app_id.version.c_str());

    OCTET_STRING_fromString(&application_id->appProvider, app_provider);
    OCTET_STRING_fromString(&application_id->appName, app_name);

    auto app_version_ptr = helper::asn1_allocate<IA5String_t>();
    OCTET_STRING_fromBuf(app_version_ptr, app_version, static_cast<int>(strlen(app_version)));
    application_id->appVersion = app_version_ptr;
    return application_id;
}

static ::Ver2_SUPL_START_extension* encode_start_extension(const START& message) {
    VSCOPE_FUNCTION();
    auto application_id = encode_applicationid(message.application_id);

    if (!application_id) {
        helper::asn1_free(application_id);
        return nullptr;
    }

    auto extension           = helper::asn1_allocate<::Ver2_SUPL_START_extension>();
    extension->applicationID = application_id;
    return extension;
}

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp, const START& message) {
    FUNCTION_SCOPEN("START");

    auto ulp_pdu = create_message(UlpMessage_PR_msSUPLSTART, version);
    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    encode_session(ulp_pdu, set, slp);

    auto& pdu_message                     = ulp_pdu->message.choice.msSUPLSTART;
    pdu_message.sETCapabilities           = encode_setcapabilities(message.set_capabilities);
    pdu_message.locationId                = encode_locationid(message.location_id);
    pdu_message.ver2_SUPL_START_extension = encode_start_extension(message);

    print(loglet::Level::Trace, ulp_pdu);
    return encode_uper(ulp_pdu);
}

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp,
                      const POSINIT& message) {
    FUNCTION_SCOPEN("POSINIT");

    auto ulp_pdu = create_message(UlpMessage_PR_msSUPLPOSINIT, version);
    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    encode_session(ulp_pdu, set, slp);

    auto& pdu_message           = ulp_pdu->message.choice.msSUPLPOSINIT;
    pdu_message.sETCapabilities = encode_setcapabilities(message.set_capabilities);
    pdu_message.locationId      = encode_locationid(message.location_id);

    if (message.payloads.size() > 0) {
        auto suplpos        = helper::asn1_allocate<SUPLPOS>();
        suplpos->posPayLoad = encode_pospayload(message.payloads);
        pdu_message.sUPLPOS = suplpos;
    }

    print(loglet::Level::Trace, ulp_pdu);
    return encode_uper(ulp_pdu);
}

EncodedMessage encode(Version version, Session::SET& set, Session::SLP& slp, const POS& message) {
    FUNCTION_SCOPEN("POS");

    auto ulp_pdu = create_message(UlpMessage_PR_msSUPLPOS, version);
    SUPL_DEFER {
        ASN_STRUCT_FREE(asn_DEF_ULP_PDU, ulp_pdu);
    };

    encode_session(ulp_pdu, set, slp);

    auto& pdu_message      = ulp_pdu->message.choice.msSUPLPOS;
    pdu_message.posPayLoad = encode_pospayload(message.payloads);

    print(loglet::Level::Trace, ulp_pdu);
    return encode_uper(ulp_pdu);
}

}  // namespace supl
