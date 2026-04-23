#include "constant.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <GNSS-ID.h>
EXTERNAL_WARNINGS_POP

#define X 255

static CONSTEXPR uint8_t GPS_TO_SPARTN[24] = {
    0,  // L1 C/A -> L1C
    X,  // L1C
    X,  // L2C
    X,  // L5
    X,  // L1 P
    X,  // L1 Z-tracking
    X,  // L2 C/A
    X,  // L2 P
    1,  // L2 Z-tracking -> L2W
    X,  // L2 L2C(M)
    2,  // L2 L2C(L) -> L2L
    X,  // L2 L2C(M+L)
    X,  // L5 I
    3,  // L5 Q -> L5Q
    X,  // L5 I+Q
    X,  // L1 L1C(D)
    X,  // L1 L1C(P)
    X,  // L1 L1C(D+P)
    // Reserved
    X,
    X,
    X,
    X,
    X,
    X,
};

static CONSTEXPR uint8_t GPS_MAPPING[32] = {
    X,   // L1 C/A
    X,   // L1C
    X,   // L2C
    X,   // L5
    X,   // L1 P
    X,   // L1 Z-tracking
    X,   // L2 C/A
    X,   // L2 P
    X,   // L2 Z-tracking
    X,   // L2 L2C(M)
    X,   // L2 L2C(L)
    10,  // L2 L2C(M+L) -> L2 L2C(L)
    X,   // L5 I
    X,   // L5 Q
    X,   // L5 I+Q
    X,   // L1 L1C(D)
    X,   // L1 L1C(P)
    X,   // L1 L1C(D+P)
    // Reserved
    X,
    X,
    X,
    X,
    X,
    X,
};

#define GPS_L1 1575.42
#define GPS_L2 1227.60
#define GPS_L5 1176.45
static CONSTEXPR double GPS_FREQ[24] = {
    GPS_L1,  // L1 C/A
    GPS_L1,  // L1C
    GPS_L2,  // L2C
    GPS_L5,  // L5
    GPS_L1,  // L1 P
    GPS_L1,  // L1 Z-tracking
    GPS_L2,  // L2 C/A
    GPS_L2,  // L2 P
    GPS_L2,  // L2 Z-tracking
    GPS_L2,  // L2 L2C(M)
    GPS_L2,  // L2 L2C(L)
    GPS_L2,  // L2 L2C(M+L)
    GPS_L5,  // L5 I
    GPS_L5,  // L5 Q
    GPS_L5,  // L5 I+Q
    GPS_L1,  // L1 L1C(D)
    GPS_L1,  // L1 L1C(P)
    GPS_L1,  // L1 L1C(D+P)
    // Reserved
    0,
    0,
    0,
    0,
    0,
    0,
};

static CONSTEXPR const char* GPS_SIGNAL_NAMES[24] = {
    "L1 C/A",
    "L1C",
    "L2C",
    "L5",
    "L1 P",
    "L1 Z-tracking",
    "L2 C/A",
    "L2 P",
    "L2 Z-tracking",
    "L2 L2C(M)",
    "L2 L2C(L)",
    "L2 L2C(M+L)",
    "L5 I",
    "L5 Q",
    "L5 I+Q",
    "L1 L1C(D)",
    "L1 L1C(P)",
    "L1 L1C(D+P)",
    // Reserved
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

static CONSTEXPR uint8_t GLO_TO_SPARTN[24] = {
    0,  // G1 C/A -> L1C
    1,  // G2 C/A -> L2C
    X,  // G3
    X,  // G1 P
    X,  // G2 P
    X,  // G1a(D)
    X,  // G1a(P)
    X,  // G1a(D+P)
    X,  // G2a(I)
    X,  // G2a(P)
    X,  // G2a(I+P)
    X,  // G3 I
    X,  // G3 Q
    X,  // G3 I+Q
    // Reserved
    X,
    X,
    X,
    X,
    X,
    X,
    X,
    X,
    X,
    X,
};

static CONSTEXPR uint8_t GLO_MAPPING[32] = {
    X,  // G1 C/A
    X,  // G2 C/A
    X,  // G3
    X,  // G1 P
    X,  // G2 P
    X,  // G1a(D)
    X,  // G1a(P)
    X,  // G1a(D+P)
    X,  // G2a(I)
    X,  // G2a(P)
    X,  // G2a(I+P)
    X,  // G3 I
    X,  // G3 Q
    X,  // G3 I+Q
    // Reserved
    X,
    X,
    X,
    X,
    X,
    X,
    X,
    X,
    X,
    X,
};

static CONSTEXPR double GLO_FREQ[24] = {
    1.0,  // G1 C/A
    1.0,  // G2 C/A
    1.0,  // G3
    1.0,  // G1 P
    1.0,  // G2 P
    1.0,  // G1a(D)
    1.0,  // G1a(P)
    1.0,  // G1a(D+P)
    1.0,  // G2a(I)
    1.0,  // G2a(P)
    1.0,  // G2a(I+P)
    1.0,  // G3 I
    1.0,  // G3 Q
    1.0,  // G3 I+Q
    // Reserved
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
};

static CONSTEXPR const char* GLO_SIGNAL_NAMES[24] = {
    "G1 C/A",
    "G2 C/A",
    "G3",
    "G1 P",
    "G2 P",
    "G1a(D)",
    "G1a(P)",
    "G1a(D+P)",
    "G2a(I)",
    "G2a(P)",
    "G2a(I+P)",
    "G3 I",
    "G3 Q",
    "G3 I+Q",
    // Reserved
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

static CONSTEXPR uint8_t GAL_TO_SPARTN[24] = {
    X,  // E1
    X,  // E5A
    X,  // E5B
    X,  // E6
    X,  // E5A+E5B
    0,  // E1 C No data -> L1C
    X,  // E1 A
    X,  // E1 B I/NAV OS/CS/SoL
    X,  // E1 B+C
    X,  // E1 A+B+C
    4,  // E6 C -> L6C
    X,  // E6 A
    X,  // E6 B
    X,  // E6 B+C
    X,  // E6 A+B+C
    X,  // E5B I
    2,  // E5B Q -> L7Q
    X,  // E5B I+Q
    X,  // E5(A+B) I
    X,  // E5(A+B) Q
    3,  // E5(A+B) I+Q -> L8Q
    X,  // E5A I
    1,  // E5A Q -> L5Q
    X,  // E5A I+Q
};

static CONSTEXPR uint8_t GAL_MAPPING[32] = {
    X,  // E1
    X,  // E5A
    X,  // E5B
    X,  // E6
    X,  // E5A+E5B
    X,  // E1 C No data
    X,  // E1 A
    X,  // E1 B I/NAV OS/CS/SoL
    5,  // E1 B+C
    X,  // E1 A+B+C
    X,  // E6 C
    X,  // E6 A
    X,  // E6 B
    X,  // E6 B+C
    X,  // E6 A+B+C
    X,  // E5B I
    X,  // E5B Q
    X,  // E5B I+Q
    X,  // E5(A+B) I
    X,  // E5(A+B) Q
    X,  // E5(A+B) I+Q
    X,  // E5A I
    X,  // E5A Q
    X,  // E5A I+Q
};

#define GAL_E1 1575.420
#define GAL_E6 1278.750
#define GAL_E5 1191.795
#define GAL_E5A 1176.450
#define GAL_E5B 1207.140
static CONSTEXPR double GAL_FREQ[24] = {
    GAL_E1,   // E1
    GAL_E5A,  // E5A
    GAL_E5B,  // E5B
    GAL_E6,   // E6
    0.0,      // E5A+E5B
    GAL_E1,   // E1 C No data
    GAL_E1,   // E1 A
    GAL_E1,   // E1 B I/NAV OS/CS/SoL
    GAL_E1,   // E1 B+C
    GAL_E1,   // E1 A+B+C
    GAL_E6,   // E6 C
    GAL_E6,   // E6 A
    GAL_E6,   // E6 B
    GAL_E6,   // E6 B+C
    GAL_E6,   // E6 A+B+C
    GAL_E5B,  // E5B I
    GAL_E5B,  // E5B Q
    GAL_E5B,  // E5B I+Q
    GAL_E5,   // E5(A+B) I
    GAL_E5,   // E5(A+B) Q
    GAL_E5,   // E5(A+B) I+Q
    GAL_E5A,  // E5A I
    GAL_E5A,  // E5A Q
    GAL_E5A,  // E5A I+Q
};

static CONSTEXPR const char* GAL_SIGNAL_NAMES[24] = {
    "E1",        "E5A",        "E5B",         "E6",       "E5A+E5B", "E1 C No data",
    "E1 A",      "E1 B I/NAV", "E1 B+C",      "E1 A+B+C", "E6 C",    "E6 A",
    "E6 B",      "E6 B+C",     "E6 A+B+C",    "E5B I",    "E5B Q",   "E5B I+Q",
    "E5(A+B) I", "E5(A+B) Q",  "E5(A+B) I+Q", "E5A I",    "E5A Q",   "E5A I+Q",
};

static CONSTEXPR uint8_t BDS_TO_SPARTN[24] = {
    0,  //  0 2I B1 I
    X,  //  1 2Q B1 Q
    X,  //  2 2X B1 I+Q
    3,  //  3 6I B3 I
    X,  //  4 6Q B3 Q
    X,  //  5 6X B3 I+Q
    2,  //  6 2I B2 I -> L7I
    X,  //  7 2Q B2 Q
    X,  //  8 2X B2 I+Q
    X,  //  9 1D B1C(D)
    4,  // 10 1C B1C(P) -> L1P
    X,  // 11 1X B1C(D+P)
    X,  // 12 5D B2a(D)
    1,  // 13 5P B2a(P)
    X,  // 14 5X B2a(D+P)
    X,  // 15 Reserved
    X,  // 16 Reserved
    X,  // 17 Reserved
    X,  // 18 Reserved
    X,  // 19 Reserved
    X,  // 20 Reserved
    X,  // 21 Reserved
    X,  // 22 Reserved
    X,  // 23 Reserved
};

static CONSTEXPR uint8_t BDS_MAPPING[24] = {
    X,  //  0 2I B1 I
    X,  //  1 2Q B1 Q
    X,  //  2 2X B1 I+Q
    X,  //  3 6I B3 I
    X,  //  4 6Q B3 Q
    X,  //  5 6X B3 I+Q
    X,  //  6 2I B2 I
    X,  //  7 2Q B2 Q
    X,  //  8 2X B2 I+Q
    X,  //  9 1D B1C(D)
    X,  // 10 1C B1C(P)
    X,  // 11 1X B1C(D+P)
    X,  // 12 5D B2a(D)
    X,  // 13 5P B2a(P)
    X,  // 14 5X B2a(D+P)
    X,  // 15 Reserved
    X,  // 16 Reserved
    X,  // 17 Reserved
    X,  // 18 Reserved
    X,  // 19 Reserved
    X,  // 20 Reserved
    X,  // 21 Reserved
    X,  // 22 Reserved
    X,  // 23 Reserved
};

#define BDS_B1 1561.098
#define BDS_B1B3 1575.42
#define BDS_B2A 1176.45
#define BDS_B2b 1207.14
#define BDS_B2AB 1191.795
#define BDS_B3 1268.52
static CONSTEXPR double BDS_FREQ[24] = {
    BDS_B1,    //  0 2I B1 I
    BDS_B1,    //  1 2Q B1 Q
    BDS_B1,    //  2 2X B1 I+Q
    BDS_B3,    //  3 6I B3 I
    BDS_B3,    //  4 6Q B3 Q
    BDS_B3,    //  5 6X B3 I+Q
    BDS_B2b,   //  6 2I B2 I
    BDS_B2b,   //  7 2Q B2 Q
    BDS_B2b,   //  8 2X B2 I+Q
    BDS_B1B3,  //  9 1D B1C(D)
    BDS_B1B3,  // 10 1C B1C(P)
    BDS_B1B3,  // 11 1X B1C(D+P)
    BDS_B2A,   // 12 5D B2a(D)
    BDS_B2A,   // 13 5P B2a(P)
    BDS_B2AB,  // 14 5X B2a(D+P)
    0.0,       // 15 Reserved
    0.0,       // 16 Reserved
    0.0,       // 17 Reserved
    0.0,       // 18 Reserved
    0.0,       // 19 Reserved
    0.0,       // 20 Reserved
    0.0,       // 21 Reserved
    0.0,       // 22 Reserved
    0.0,       // 23 Reserved
};

static CONSTEXPR const char* BDS_SIGNAL_NAMES[24] = {
    "B1 I",     "B1 Q",     "B1 I+Q",   "B3 I",     "B3 Q",     "B3 I+Q",   "B2 I",     "B2 Q",
    "B2 I+Q",   "B1C(D)",   "B1C(P)",   "B1C(D+P)", "B2a(D)",   "B2a(P)",   "B2a(D+P)", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

static CONSTEXPR const char* GPS_RINEX_SUFFIX[24] = {
    "1C",  // L1 C/A
    "1X",  // L1C (D+P)
    "2X",  // L2C
    "5X",  // L5
    "1P",  // L1 P
    "1W",  // L1 Z-tracking
    "2C",  // L2 C/A
    "2P",  // L2 P
    "2W",  // L2 Z-tracking
    "2S",  // L2C(M)
    "2L",  // L2C(L)
    "2X",  // L2C(M+L)
    "5I",  // L5 I
    "5Q",  // L5 Q
    "5X",  // L5 I+Q
    "1S",  // L1C(D)
    "1L",  // L1C(P)
    "1X",  // L1C(D+P)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};
static CONSTEXPR const char* GPS_RINEX_NAME[24] = {
    "GPS L1 C/A", "GPS L1C(D+P)", "GPS L2C",
    "GPS L5 I+Q", "GPS L1 P",     "GPS L1 Z-tracking",
    "GPS L2 C/A", "GPS L2 P",     "GPS L2 Z-tracking",
    "GPS L2C(M)", "GPS L2C(L)",   "GPS L2C(M+L)",
    "GPS L5 I",   "GPS L5 Q",     "GPS L5 I+Q",
    "GPS L1C(D)", "GPS L1C(P)",   "GPS L1C(D+P)",
    nullptr,      nullptr,        nullptr,
    nullptr,      nullptr,        nullptr,
};

static CONSTEXPR const char* GLO_RINEX_SUFFIX[24] = {
    "1C",  // G1 C/A
    "2C",  // G2 C/A
    "3X",  // G3
    "1P",  // G1 P
    "2P",  // G2 P
    "4A",  // G1a(D)
    "4B",  // G1a(P)
    "4X",  // G1a(D+P)
    "6A",  // G2a(I)
    "6B",  // G2a(P)
    "6X",  // G2a(I+P)
    "3I",  // G3 I
    "3Q",  // G3 Q
    "3X",  // G3 I+Q
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};
static CONSTEXPR const char* GLO_RINEX_NAME[24] = {
    "GLO G1 C/A", "GLO G2 C/A",   "GLO G3 I+Q", "GLO G1 P",   "GLO G2 P",     "GLO G1a(D)",
    "GLO G1a(P)", "GLO G1a(D+P)", "GLO G2a(I)", "GLO G2a(P)", "GLO G2a(I+P)", "GLO G3 I",
    "GLO G3 Q",   "GLO G3 I+Q",   nullptr,      nullptr,      nullptr,        nullptr,
    nullptr,      nullptr,        nullptr,      nullptr,      nullptr,        nullptr,
};

static CONSTEXPR const char* GAL_RINEX_SUFFIX[24] = {
    "1Z",  // E1
    "5X",  // E5A
    "7X",  // E5B
    "6Z",  // E6
    "8X",  // E5A+E5B
    "1C",  // E1 C no data
    "1A",  // E1 A
    "1B",  // E1 B I/NAV
    "1X",  // E1 B+C
    "1Z",  // E1 A+B+C
    "6C",  // E6 C
    "6A",  // E6 A
    "6B",  // E6 B
    "6X",  // E6 B+C
    "6Z",  // E6 A+B+C
    "7I",  // E5B I
    "7Q",  // E5B Q
    "7X",  // E5B I+Q
    "8I",  // E5(A+B) I
    "8Q",  // E5(A+B) Q
    "8X",  // E5(A+B) I+Q
    "5I",  // E5A I
    "5Q",  // E5A Q
    "5X",  // E5A I+Q
};
static CONSTEXPR const char* GAL_RINEX_NAME[24] = {
    "GAL E1 A+B+C",      "GAL E5a I+Q",      "GAL E5b I+Q",       "GAL E6 A+B+C",
    "GAL E5(a+b) I+Q",   "GAL E1 C no data", "GAL E1 A PRS",      "GAL E1 B I/NAV",
    "GAL E1 B+C",        "GAL E1 A+B+C",     "GAL E6 C no data",  "GAL E6 A PRS",
    "GAL E6 B C/NAV",    "GAL E6 B+C",       "GAL E6 A+B+C",      "GAL E5b I I/NAV",
    "GAL E5b Q no data", "GAL E5b I+Q",      "GAL E5(a+b) I",     "GAL E5(a+b) Q",
    "GAL E5(a+b) I+Q",   "GAL E5a I F/NAV",  "GAL E5a Q no data", "GAL E5a I+Q",
};

static CONSTEXPR const char* BDS_RINEX_SUFFIX[24] = {
    "2I",  //  0 B1-2 I
    "2Q",  //  1 B1-2 Q
    "2X",  //  2 B1-2 I+Q
    "6I",  //  3 B3 I
    "6Q",  //  4 B3 Q
    "6X",  //  5 B3 I+Q
    "7I",  //  6 B2b I (BDS-2)
    "7Q",  //  7 B2b Q
    "7X",  //  8 B2b I+Q
    "1D",  //  9 B1C(D)
    "1P",  // 10 B1C(P)
    "1X",  // 11 B1C(D+P)
    "5D",  // 12 B2a(D)
    "5P",  // 13 B2a(P)
    "5X",  // 14 B2a(D+P)
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};
static CONSTEXPR const char* BDS_RINEX_NAME[24] = {
    "BDS B1-2 I",
    "BDS B1-2 Q",
    "BDS B1-2 I+Q",
    "BDS B3 I",
    "BDS B3 Q",
    "BDS B3 I+Q",
    "BDS B2b I (BDS-2)",
    "BDS B2b Q",
    "BDS B2b I+Q",
    "BDS B1C Data",
    "BDS B1C Pilot",
    "BDS B1C Data+Pilot",
    "BDS B2a Data",
    "BDS B2a Pilot",
    "BDS B2a Data+Pilot",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

char const* SystemMapping::gnss_name() const {
    if (gnss_id == GNSS_ID__gnss_id_gps) return "GPS";
    if (gnss_id == GNSS_ID__gnss_id_glonass) return "GLO";
    if (gnss_id == GNSS_ID__gnss_id_galileo) return "GAL";
    if (gnss_id == GNSS_ID__gnss_id_bds) return "BDS";
    if (gnss_id == GNSS_ID__gnss_id_qzss) return "QZSS";
    return "Unknown";
}

char const* SystemMapping::signal_name(long signal_id) const {
    if (signal_id < signal_count) return signal_names[signal_id];
    return "Unknown";
}

char const* SystemMapping::signal_rinex_suffix(long signal_id) const {
    if (signal_id < signal_count && rinex_suffixes[signal_id]) return rinex_suffixes[signal_id];
    return nullptr;
}

char const* SystemMapping::signal_rinex_name(long signal_id) const {
    if (signal_id < signal_count && rinex_names[signal_id]) return rinex_names[signal_id];
    return nullptr;
}

SystemMapping const gGpsSm = {
    GNSS_ID__gnss_id_gps,
    24,
    GPS_TO_SPARTN,
    GPS_MAPPING,
    GPS_FREQ,
    GPS_SIGNAL_NAMES,
    GPS_RINEX_SUFFIX,
    GPS_RINEX_NAME,
};

SystemMapping const gGloSm = {
    GNSS_ID__gnss_id_glonass,
    24,
    GLO_TO_SPARTN,
    GLO_MAPPING,
    GLO_FREQ,
    GLO_SIGNAL_NAMES,
    GLO_RINEX_SUFFIX,
    GLO_RINEX_NAME,
};

SystemMapping const gGalSm = {
    GNSS_ID__gnss_id_galileo,
    24,
    GAL_TO_SPARTN,
    GAL_MAPPING,
    GAL_FREQ,
    GAL_SIGNAL_NAMES,
    GAL_RINEX_SUFFIX,
    GAL_RINEX_NAME,
};

SystemMapping const gBdsSm = {
    GNSS_ID__gnss_id_bds,
    24,
    BDS_TO_SPARTN,
    BDS_MAPPING,
    BDS_FREQ,
    BDS_SIGNAL_NAMES,
    BDS_RINEX_SUFFIX,
    BDS_RINEX_NAME,
};

static CONSTEXPR const char* GPS_PHASE_BIAS_TYPES[4] = {
    "L1C",
    "L2W",
    "L2L",
    "L5Q",
};

static CONSTEXPR const char* GLO_PHASE_BIAS_TYPES[2] = {
    "L1C",
    "L2C",
};

static CONSTEXPR const char* GAL_PHASE_BIAS_TYPES[5] = {
    "L1C", "L5Q", "L7Q", "L8Q", "L6C",
};

static CONSTEXPR const char* BDS_PHASE_BIAS_TYPES[5] = {
    "L2I", "L5P", "L7I", "L6I", "L1P",
};

static CONSTEXPR const char* GPS_CODE_BIAS_TYPES[4] = {
    "C1C",
    "C2W",
    "C2L",
    "C5Q",
};

static CONSTEXPR const char* GLO_CODE_BIAS_TYPES[2] = {
    "C1C",
    "C2C",
};

static CONSTEXPR const char* GAL_CODE_BIAS_TYPES[5] = {
    "C1C", "C5Q", "C7Q", "C8Q", "C6C",
};

static CONSTEXPR const char* BDS_CODE_BIAS_TYPES[5] = {
    "C2I", "C5P", "C7I", "C6I", "C1P",
};

char const* bias_type_name(long gnss_id, bool is_phase, uint8_t type) {
    if (gnss_id == GNSS_ID__gnss_id_gps) {
        if (is_phase) {
            if (type < 4) return GPS_PHASE_BIAS_TYPES[type];
        } else {
            if (type < 4) return GPS_CODE_BIAS_TYPES[type];
        }
    } else if (gnss_id == GNSS_ID__gnss_id_glonass) {
        if (is_phase) {
            if (type < 2) return GLO_PHASE_BIAS_TYPES[type];
        } else {
            if (type < 2) return GLO_CODE_BIAS_TYPES[type];
        }
    } else if (gnss_id == GNSS_ID__gnss_id_galileo) {
        if (is_phase) {
            if (type < 5) return GAL_PHASE_BIAS_TYPES[type];
        } else {
            if (type < 5) return GAL_CODE_BIAS_TYPES[type];
        }
    } else if (gnss_id == GNSS_ID__gnss_id_bds) {
        if (is_phase) {
            if (type < 5) return BDS_PHASE_BIAS_TYPES[type];
        } else {
            if (type < 5) return BDS_CODE_BIAS_TYPES[type];
        }
    }

    return "Unknown";
}

#undef X
