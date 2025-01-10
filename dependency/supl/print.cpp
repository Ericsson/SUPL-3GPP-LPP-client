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
#include <ULP-PDU.h>
#include <Version.h>
#include <SetSessionID.h>
#include <SlpSessionID.h>
#include <MNC.h>
#include <MCC.h>
#pragma GCC diagnostic pop

#define LOGLET_CURRENT_MODULE "supl/print"

namespace supl {

class Printer {
public:
    Printer() {
        mIndent        = 0;
        mPreviousField = false;
    }

    const char* c_str() const { return mBuffer.c_str(); }

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

    void append(const char* data) { mBuffer += std::string{data}; }

    void vappendf(const char* fmt, va_list args) {
        vsnprintf(mTempBuffer, sizeof(mTempBuffer), fmt, args);
        append(mTempBuffer);
    }

    void appendf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vappendf(fmt, args);
        va_end(args);
    }

    void value(long value) { appendf("%ld", value); }
    void value(const char* value) { appendf("\"%s\"", value); }

    void value(const ::ULP_PDU& value);
    void value(const ::Version& value);
    void value(const ::SessionID& value);
    void value(const ::SetSessionID& value);
    void value(const ::SETId& value);
    void value(const ::OCTET_STRING& value);
    void value(const ::IPAddress& value);
    void value(const ::SlpSessionID& value);
    void value(const ::SLPAddress& value);
    void value(const ::UlpMessage& value);
    void value(const ::SUPLINIT& value);
    void value(const ::PosMethod& value);
    void value(const ::Notification& value);
    void value(const ::QoP& value);
    void value(const ::SLPMode& value);
    void value(const ::BIT_STRING_s& value);
    void value(const ::Ver2_SUPL_INIT_extension& value);
    void value(const ::SUPLSTART& value);
    void value(const ::SETCapabilities& value);
    void value(const ::LocationId& value);
    void value(const ::Ver2_SUPL_START_extension& value);

    void value(const ::PosTechnology& x);
    void value(const ::Ver2_PosTechnology_extension& x);
    void value(const ::PrefMethod& x);
    void value(const ::PosProtocol& x);
    void value(const ::Ver2_PosProtocol_extension& x);
    void value(const ::CellInfo& x);
    void value(const ::GsmCellInformation& x);
    void value(const ::NMR& x);
    void value(const ::WcdmaCellInformation& x);
    void value(const ::CdmaCellInformation& x);

    void value(const ::Ver2_CellInfo_extension& x);
    void value(const ::LteCellInformation& x);
    void value(const ::CellGlobalIdEUTRA& x);
    void value(const ::PLMN_Identity& x);
    void value(const ::MCC& x);
    void value(const ::MNC& x);
    void value(const ::MeasResultListEUTRA& x);
    void value(const ::Ver2_SETCapabilities_extension& x);

    template <typename T>
    void field(const char* field_name, const T& field_value) {
        if (mPreviousField) {
            mBuffer.push_back(',');
            newline();
        }

        appendf("\"%s\": ", field_name);
        value(field_value);
        mPreviousField = true;
    }

    template <typename T>
    void field_opt(const char* field_name, const T* field_value) {
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

void Printer::value(const ::ULP_PDU& x) {
    push('{');
    field("length", x.length);
    field("version", x.version);
    field("session-id", x.sessionID);
    field("message", x.message);
    pop('}');
}

void Printer::value(const ::Version& x) {
    push('{');
    field("maj", x.maj);
    field("min", x.min);
    field("servind", x.servind);
    pop('}');
}

void Printer::value(const ::SessionID& x) {
    push('{');
    field_opt("set-session-id", x.setSessionID);
    field_opt("slp-session-id", x.slpSessionID);
    pop('}');
}

void Printer::value(const ::SetSessionID& x) {
    push('{');
    field("session-id", x.sessionId);
    field("set-id", x.setId);
    pop('}');
}

void Printer::value(const ::SETId& x) {
    push('{');
    switch (x.present) {
    case SETId_PR_msisdn: field("msisdn", x.choice.msisdn); break;
    case SETId_PR_imsi: field("imsi", x.choice.imsi); break;
    case SETId_PR_iPAddress: field("ip-address", x.choice.iPAddress); break;
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(const ::OCTET_STRING& x) {
    append("OCTET STRING \"");
    for (size_t i = 0; i < x.size; i++) {
        appendf("%02X", x.buf[i]);
    }
    append("\"");
}

void Printer::value(const ::IPAddress& x) {
    push('{');
    switch (x.present) {
    case IPAddress_PR_ipv4Address: field("ipv4-address", x.choice.ipv4Address); break;
    case IPAddress_PR_ipv6Address: field("ipv6-address", x.choice.ipv6Address); break;
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(const ::SlpSessionID& x) {
    push('{');
    field("session-id", x.sessionID);
    field("slp-id", x.slpId);
    pop('}');
}

void Printer::value(const ::SLPAddress& x) {
    push('{');
    switch (x.present) {
    case SLPAddress_PR_iPAddress: field("ip-address", x.choice.iPAddress); break;
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(const ::UlpMessage& x) {
    push('{');
    switch (x.present) {
    case UlpMessage_PR_msSUPLINIT: field("ms-suplinit", x.choice.msSUPLINIT); break;
    case UlpMessage_PR_msSUPLSTART: field("ms-suplstart", x.choice.msSUPLSTART); break;
    case UlpMessage_PR_msSUPLPOS: field("type", "ms-suplpos"); break;
    case UlpMessage_PR_msSUPLPOSINIT: field("type", "ms-suplposinit"); break;
    case UlpMessage_PR_msSUPLEND: field("type", "ms-suplend"); break;
    case UlpMessage_PR_msSUPLRESPONSE: field("type", "ms-suplresponse"); break;
    default: field("type", "unsupported");
    }
    pop('}');
}

void Printer::value(const ::SUPLINIT& x) {
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

void Printer::value(const ::PosMethod& x) {
    switch (x) {
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

void Printer::value(const ::Notification&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::QoP&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::SLPMode& x) {
    switch (x) {
    case SLPMode_proxy: value("proxy"); break;
    case SLPMode_nonProxy: value("nonProxy"); break;
    default: appendf("unsupported (%d)", x);
    }
}

void Printer::value(const ::BIT_STRING_s&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::Ver2_SUPL_INIT_extension&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::SUPLSTART& x) {
    push('{');
    field("set-capabilities", x.sETCapabilities);
    field("location-id", x.locationId);
    field_opt("qop", x.qoP);
    field_opt("ver2-supl-start-extension", x.ver2_SUPL_START_extension);
    pop('}');
}

void Printer::value(const ::SETCapabilities& x) {
    push('{');
    field("pos-technology", x.posTechnology);
    field("pref-method", x.prefMethod);
    field("pos-protocol", x.posProtocol);
    field_opt("ver2-set-capabilities-extension", x.ver2_SETCapabilities_extension);
    pop('}');
}

void Printer::value(const ::PosTechnology& x) {
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

void Printer::value(const ::Ver2_PosTechnology_extension&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::PrefMethod& x) {
    switch (x) {
    case PrefMethod_agpsSETassistedPreferred: value("agpsSETassisted"); break;
    case PrefMethod_agpsSETBasedPreferred: value("agpsSETBased"); break;
    case PrefMethod_noPreference: value("noPreference"); break;
    default: appendf("unsupported (%d)", x);
    }
}

void Printer::value(const ::PosProtocol& x) {
    push('{');
    field("tia801", x.tia801);
    field("rrlp", x.rrlp);
    field("rrc", x.rrc);
    field_opt("ver2-pos-protocol-extension", x.ver2_PosProtocol_extension);
    pop('}');
}

void Printer::value(const ::Ver2_PosProtocol_extension&) {
    push('{');

    pop('}');
}

void Printer::value(const ::LocationId& x) {
    push('{');
    field("cell-info", x.cellInfo);
    field("status", x.status);
    pop('}');
}

void Printer::value(const ::CellInfo& x) {
    switch (x.present) {
    case CellInfo_PR_gsmCell: value(x.choice.gsmCell); break;
    case CellInfo_PR_wcdmaCell: value(x.choice.wcdmaCell); break;
    case CellInfo_PR_cdmaCell: value(x.choice.cdmaCell); break;
    case CellInfo_PR_ver2_CellInfo_extension: value(x.choice.ver2_CellInfo_extension); break;
    default: field("type", "unsupported");
    }
}

void Printer::value(const ::GsmCellInformation& x) {
    push('{');
    field("ref-mcc", x.refMCC);
    field("ref-mnc", x.refMNC);
    field("ref-lac", x.refLAC);
    field("ref-ci", x.refCI);
    field_opt("n-mr", x.nMR);
    field_opt("t-a", x.tA);
    pop('}');
}

void Printer::value(const ::NMR&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::WcdmaCellInformation&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::CdmaCellInformation&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::Ver2_CellInfo_extension& x) {
    switch (x.present) {
    case Ver2_CellInfo_extension_PR_lteCell: value(x.choice.lteCell); break;
    default: field("type", "unsupported");
    }
}

void Printer::value(const ::LteCellInformation& x) {
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

void Printer::value(const ::CellGlobalIdEUTRA& x) {
    push('{');
    field("plmn-identity", x.plmn_Identity);
    field("cell-identity", x.cellIdentity);
    pop('}');
}

void Printer::value(const ::PLMN_Identity& x) {
    push('{');
    field_opt("mcc", x.mcc);
    field("mnc", x.mnc);
    pop('}');
}

void Printer::value(const ::MCC& x) {
    push('[');
    for (auto i = 0; i < x.list.count; i++) {
        if (x.list.array[i]) {
            value(*x.list.array[i]);
        }
    }
    pop(']');
}

void Printer::value(const ::MNC& x) {
    push('[');
    for (auto i = 0; i < x.list.count; i++) {
        if (x.list.array[i]) {
            value(*x.list.array[i]);
        }
    }
    pop(']');
}

void Printer::value(const ::MeasResultListEUTRA&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::Ver2_SETCapabilities_extension&) {
    appendf("{ /* unsupported */ }");
}

void Printer::value(const ::Ver2_SUPL_START_extension&) {
    appendf("{ /* unsupported */ }");
}

void print(loglet::Level level, ::ULP_PDU* ulp_pdu) {
    Printer printer{};
    printer.value(*ulp_pdu);

    loglet::logf(LOGLET_CURRENT_MODULE, level, "\n%s", printer.c_str());
}

}  // namespace supl
