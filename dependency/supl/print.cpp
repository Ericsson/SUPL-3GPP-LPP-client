#include "print.hpp"

#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <KeyIdentity.h>
#include <MAC.h>
#include <MCC.h>
#include <MNC.h>
#include <SetSessionID.h>
#include <SlpSessionID.h>
#include <ULP-PDU.h>
#include <Version.h>
#pragma GCC diagnostic pop

#define LOGLET_CURRENT_MODULE "supl/print"

namespace supl {

class Printer {
public:
    Printer() {
        mIndent        = 0;
        mPreviousField = false;
    }

    char const* c_str() const { return mBuffer.c_str(); }

    void push(char ch) {
        mBuffer.push_back(ch);
        mIndent += 1;
        newline();
        mPreviousField = false;
    }

    void pop(char ch) {
        mIndent -= 1;
        newline();
        mBuffer.push_back(ch);
        mPreviousField = true;
    }

    void newline() {
        mBuffer.push_back('\n');
        for (auto i = 0; i < mIndent; i++) {
            mBuffer.push_back(' ');
            mBuffer.push_back(' ');
        }
    }

    void append(char const* data) { mBuffer += std::string{data}; }

    void vappendf(char const* fmt, va_list args) {
        vsnprintf(mTempBuffer, sizeof(mTempBuffer), fmt, args);
        append(mTempBuffer);
    }

    void appendf(char const* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vappendf(fmt, args);
        va_end(args);
    }

    void value(long value) { appendf("%ld", value); }
    void value(char const* value) { appendf("\"%s\"", value); }

    void value(::ULP_PDU const& value);
    void value(::Version const& value);
    void value(::SessionID const& value);
    void value(::SetSessionID const& value);
    void value(::SETId const& value);
    void value(::OCTET_STRING const& value);
    void value(::IPAddress const& value);
    void value(::SlpSessionID const& value);
    void value(::SLPAddress const& value);
    void value(::UlpMessage const& value);
    void value(::SUPLINIT const& value);
    void value(::PosMethod const& value);
    void value(::Notification const& value);
    void value(::QoP const& value);
    void value(::SLPMode const& value);
    void value(::BIT_STRING_s const& value);
    void value(::Ver2_SUPL_INIT_extension const& value);
    void value(::SUPLSTART const& value);
    void value(::SETCapabilities const& value);
    void value(::LocationId const& value);
    void value(::Ver2_SUPL_START_extension const& value);

    void value(::PosTechnology const& x);
    void value(::Ver2_PosTechnology_extension const& x);
    void value(::PrefMethod const& x);
    void value(::PosProtocol const& x);
    void value(::Ver2_PosProtocol_extension const& x);
    void value(::CellInfo const& x);
    void value(::GsmCellInformation const& x);
    void value(::NMR const& x);
    void value(::WcdmaCellInformation const& x);
    void value(::CdmaCellInformation const& x);

    void value(::Ver2_CellInfo_extension const& x);
    void value(::LteCellInformation const& x);
    void value(::CellGlobalIdEUTRA const& x);
    void value(::PLMN_Identity const& x);
    void value(::MCC const& x);
    void value(::MNC const& x);
    void value(::MeasResultListEUTRA const& x);
    void value(::Ver2_SETCapabilities_extension const& x);

    template <typename T>
    void field(char const* field_name, T const& field_value) {
        if (mPreviousField) {
            mBuffer.push_back(',');
            newline();
        }

        appendf("\"%s\": ", field_name);
        value(field_value);
        mPreviousField = true;
    }

    template <typename T>
    void field_opt(char const* field_name, T const* field_value) {
        if (field_value) {
            field(field_name, *field_value);
        }
    }

private:
    std::string mBuffer;
    char        mTempBuffer[4096];
    int         mIndent;
    bool        mPreviousField;
};

void Printer::value(::ULP_PDU const& x) {
    push('{');
    field("length", x.length);
    field("version", x.version);
    field("session-id", x.sessionID);
    field("message", x.message);
    pop('}');
}

void Printer::value(::Version const& x) {
    push('{');
    field("maj", x.maj);
    field("min", x.min);
    field("servind", x.servind);
    pop('}');
}

void Printer::value(::SessionID const& x) {
    push('{');
    field_opt("set-session-id", x.setSessionID);
    field_opt("slp-session-id", x.slpSessionID);
    pop('}');
}

void Printer::value(::SetSessionID const& x) {
    push('{');
    field("session-id", x.sessionId);
    field("set-id", x.setId);
    pop('}');
}

void Printer::value(::SETId const& x) {
    push('{');
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x.present)) {
    case SETId_PR_NOTHING: field("type", "nothing"); break;
    case SETId_PR_msisdn: field("msisdn", x.choice.msisdn); break;
    case SETId_PR_imsi: field("imsi", x.choice.imsi); break;
    case SETId_PR_iPAddress: field("ip-address", x.choice.iPAddress); break;
    case SETId_PR_mdn:
    case SETId_PR_min:
    case SETId_PR_nai:
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(::OCTET_STRING const& x) {
    append("OCTET STRING \"");
    for (size_t i = 0; i < x.size; i++) {
        appendf("%02X", x.buf[i]);
    }
    append("\"");
}

void Printer::value(::IPAddress const& x) {
    push('{');
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x.present)) {
    case IPAddress_PR_NOTHING: field("type", "nothing"); break;
    case IPAddress_PR_ipv4Address: field("ipv4-address", x.choice.ipv4Address); break;
    case IPAddress_PR_ipv6Address: field("ipv6-address", x.choice.ipv6Address); break;
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(::SlpSessionID const& x) {
    push('{');
    field("session-id", x.sessionID);
    field("slp-id", x.slpId);
    pop('}');
}

void Printer::value(::SLPAddress const& x) {
    push('{');
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x.present)) {
    case SLPAddress_PR_NOTHING: field("type", "nothing"); break;
    case SLPAddress_PR_iPAddress: field("ip-address", x.choice.iPAddress); break;
    case SLPAddress_PR_fQDN: field("type", "unsupported"); break;
    }
    pop('}');
}

void Printer::value(::UlpMessage const& x) {
    push('{');
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x.present)) {
    case UlpMessage_PR_NOTHING: field("type", "nothing"); break;
    case UlpMessage_PR_msSUPLINIT: field("ms-suplinit", x.choice.msSUPLINIT); break;
    case UlpMessage_PR_msSUPLSTART: field("ms-suplstart", x.choice.msSUPLSTART); break;
    case UlpMessage_PR_msSUPLPOS: field("type", "ms-suplpos"); break;
    case UlpMessage_PR_msSUPLPOSINIT: field("type", "ms-suplposinit"); break;
    case UlpMessage_PR_msSUPLEND: field("type", "ms-suplend"); break;
    case UlpMessage_PR_msSUPLRESPONSE: field("type", "ms-suplresponse"); break;
    case UlpMessage_PR_msSUPLAUTHREQ:
    case UlpMessage_PR_msSUPLAUTHRESP:
    case UlpMessage_PR_msSUPLTRIGGEREDSTART:
    case UlpMessage_PR_msSUPLTRIGGEREDRESPONSE:
    case UlpMessage_PR_msSUPLTRIGGEREDSTOP:
    case UlpMessage_PR_msSUPLNOTIFY:
    case UlpMessage_PR_msSUPLNOTIFYRESPONSE:
    case UlpMessage_PR_msSUPLSETINIT:
    case UlpMessage_PR_msSUPLREPORT:
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(::SUPLINIT const& x) {
    push('{');
    field("pos-method", x.posMethod);
    field_opt("notification", x.notification);
    field_opt("slp-address", x.sLPAddress);
    field_opt("qop", x.qoP);
    field("slp-mode", x.sLPMode);
    field_opt("mac", x.mAC);
    field_opt("key-identity", x.keyIdentity);
    field_opt("ver2-supl-init-extension", x.ver2_SUPL_INIT_extension);
    pop('}');
}

void Printer::value(::PosMethod const& x) {
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x)) {
    case PosMethod_agpsSETassisted: value("agpsSETassisted"); break;
    case PosMethod_agpsSETbased: value("agpsSETbased"); break;
    case PosMethod_agpsSETassistedpref: value("agpsSETassistedpref"); break;
    case PosMethod_agpsSETbasedpref: value("agpsSETbasedpref"); break;
    case PosMethod_autonomousGPS: value("autonomousGPS"); break;
    case PosMethod_aFLT: value("aFLT"); break;
    case PosMethod_eCID: value("eCID"); break;
    case PosMethod_eOTD: value("eOTD"); break;
    case PosMethod_oTDOA: value("oTDOA"); break;
    case PosMethod_noPosition: value("noPosition"); break;
    case PosMethod_ver2_historicalDataRetrieval: value("ver2_historicalDataRetrieval"); break;
    case PosMethod_ver2_agnssSETassisted: value("ver2_agnssSETassisted"); break;
    case PosMethod_ver2_agnssSETbased: value("ver2_agnssSETbased"); break;
    case PosMethod_ver2_agnssSETassistedpref: value("ver2_agnssSETassistedpref"); break;
    case PosMethod_ver2_agnssSETbasedpref: value("ver2_agnssSETbasedpref"); break;
    case PosMethod_ver2_autonomousGNSS: value("ver2_autonomousGNSS"); break;
    case PosMethod_ver2_sessioninfoquery: value("ver2_sessioninfoquery"); break;
    default: appendf("unsupported (%d)", x);
    }
}

void Printer::value(::Notification const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::QoP const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::SLPMode const& x) {
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x)) {
    case SLPMode_proxy: value("proxy"); break;
    case SLPMode_nonProxy: value("nonProxy"); break;
    default: appendf("unsupported (%d)", x);
    }
}

void Printer::value(::BIT_STRING_s const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::Ver2_SUPL_INIT_extension const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::SUPLSTART const& x) {
    push('{');
    field("set-capabilities", x.sETCapabilities);
    field("location-id", x.locationId);
    field_opt("qop", x.qoP);
    field_opt("ver2-supl-start-extension", x.ver2_SUPL_START_extension);
    pop('}');
}

void Printer::value(::SETCapabilities const& x) {
    push('{');
    field("pos-technology", x.posTechnology);
    field("pref-method", x.prefMethod);
    field("pos-protocol", x.posProtocol);
    field_opt("ver2-set-capabilities-extension", x.ver2_SETCapabilities_extension);
    pop('}');
}

void Printer::value(::PosTechnology const& x) {
    push('{');
    field("agps-set-assisted", x.agpsSETassisted);
    field("agps-set-based", x.agpsSETBased);
    field("autonomous-gps", x.autonomousGPS);
    field("a-flt", x.aFLT);
    field("e-cid", x.eCID);
    field("e-otd", x.eOTD);
    field("o-tdoa", x.oTDOA);
    field_opt("ver2-pos-technology-extension", x.ver2_PosTechnology_extension);
    pop('}');
}

void Printer::value(::Ver2_PosTechnology_extension const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::PrefMethod const& x) {
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x)) {
    case PrefMethod_agpsSETassistedPreferred: value("agpsSETassisted"); break;
    case PrefMethod_agpsSETBasedPreferred: value("agpsSETBased"); break;
    case PrefMethod_noPreference: value("noPreference"); break;
    default: appendf("unsupported (%d)", x);
    }
}

void Printer::value(::PosProtocol const& x) {
    push('{');
    field("tia801", x.tia801);
    field("rrlp", x.rrlp);
    field("rrc", x.rrc);
    field_opt("ver2-pos-protocol-extension", x.ver2_PosProtocol_extension);
    pop('}');
}

void Printer::value(::Ver2_PosProtocol_extension const&) {
    push('{');

    pop('}');
}

void Printer::value(::LocationId const& x) {
    push('{');
    field("cell-info", x.cellInfo);
    field("status", x.status);
    pop('}');
}

void Printer::value(::CellInfo const& x) {
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x.present)) {
    case CellInfo_PR_gsmCell: value(x.choice.gsmCell); break;
    case CellInfo_PR_wcdmaCell: value(x.choice.wcdmaCell); break;
    case CellInfo_PR_cdmaCell: value(x.choice.cdmaCell); break;
    case CellInfo_PR_ver2_CellInfo_extension: value(x.choice.ver2_CellInfo_extension); break;
    default: field("type", "unsupported");
    }
}

void Printer::value(::GsmCellInformation const& x) {
    push('{');
    field("ref-mcc", x.refMCC);
    field("ref-mnc", x.refMNC);
    field("ref-lac", x.refLAC);
    field("ref-ci", x.refCI);
    field_opt("n-mr", x.nMR);
    field_opt("t-a", x.tA);
    pop('}');
}

void Printer::value(::NMR const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::WcdmaCellInformation const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::CdmaCellInformation const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::Ver2_CellInfo_extension const& x) {
    // NOTE(ewasjon): switch on the integer value of the enum to be explicit that the actual enum
    // might not contain all variants
    switch (static_cast<long>(x.present)) {
    case Ver2_CellInfo_extension_PR_lteCell: value(x.choice.lteCell); break;
    default: field("type", "unsupported");
    }
}

void Printer::value(::LteCellInformation const& x) {
    push('{');
    field("cell-global-id-eutra", x.cellGlobalIdEUTRA);
    field("phys-cell-id", x.physCellId);
    field("tracking-area-code", x.trackingAreaCode);
    field_opt("rsrp-result", x.rsrpResult);
    field_opt("rsrq-result", x.rsrqResult);
    field_opt("t-a", x.tA);
    field_opt("meas-result-list-eutra", x.measResultListEUTRA);
    pop('}');
}

void Printer::value(::CellGlobalIdEUTRA const& x) {
    push('{');
    field("plmn-identity", x.plmn_Identity);
    field("cell-identity", x.cellIdentity);
    pop('}');
}

void Printer::value(::PLMN_Identity const& x) {
    push('{');
    field_opt("mcc", x.mcc);
    field("mnc", x.mnc);
    pop('}');
}

void Printer::value(::MCC const& x) {
    push('[');
    for (auto i = 0; i < x.list.count; i++) {
        if (x.list.array[i]) {
            value(*x.list.array[i]);
        }
    }
    pop(']');
}

void Printer::value(::MNC const& x) {
    push('[');
    for (auto i = 0; i < x.list.count; i++) {
        if (x.list.array[i]) {
            value(*x.list.array[i]);
        }
    }
    pop(']');
}

void Printer::value(::MeasResultListEUTRA const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::Ver2_SETCapabilities_extension const&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(::Ver2_SUPL_START_extension const&) {
    appendf("{ /* unsupported */ }");
}

void print(loglet::Level level, ::ULP_PDU* ulp_pdu) {
    Printer printer{};
    printer.value(*ulp_pdu);

    loglet::logf(LOGLET_CURRENT_MODULE, level, "\n%s", printer.c_str());
}

}  // namespace supl
