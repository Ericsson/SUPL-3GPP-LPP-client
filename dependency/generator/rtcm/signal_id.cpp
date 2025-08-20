#include "signal_id.hpp"
#include <cassert>

SignalId const SignalId::GPS_L1_CA         = SignalId::from_lpp(SignalId::GPS, 0);
SignalId const SignalId::GPS_L1C           = SignalId::from_lpp(SignalId::GPS, 1);
SignalId const SignalId::GPS_L2C           = SignalId::from_lpp(SignalId::GPS, 2);
SignalId const SignalId::GPS_L5            = SignalId::from_lpp(SignalId::GPS, 3);
SignalId const SignalId::GPS_L1_P          = SignalId::from_lpp(SignalId::GPS, 4);
SignalId const SignalId::GPS_L1_Z_TRACKING = SignalId::from_lpp(SignalId::GPS, 5);
SignalId const SignalId::GPS_L2_C_A        = SignalId::from_lpp(SignalId::GPS, 6);
SignalId const SignalId::GPS_L2_P          = SignalId::from_lpp(SignalId::GPS, 7);
SignalId const SignalId::GPS_L2_Z_TRACKING = SignalId::from_lpp(SignalId::GPS, 8);
SignalId const SignalId::GPS_L2_L2C_M      = SignalId::from_lpp(SignalId::GPS, 9);
SignalId const SignalId::GPS_L2_L2C_L      = SignalId::from_lpp(SignalId::GPS, 10);
SignalId const SignalId::GPS_L2_L2C_M_L    = SignalId::from_lpp(SignalId::GPS, 11);
SignalId const SignalId::GPS_L5_I          = SignalId::from_lpp(SignalId::GPS, 12);
SignalId const SignalId::GPS_L5_Q          = SignalId::from_lpp(SignalId::GPS, 13);
SignalId const SignalId::GPS_L5_I_Q        = SignalId::from_lpp(SignalId::GPS, 14);
SignalId const SignalId::GPS_L1_L1C_D      = SignalId::from_lpp(SignalId::GPS, 15);
SignalId const SignalId::GPS_L1_L1C_P      = SignalId::from_lpp(SignalId::GPS, 16);
SignalId const SignalId::GPS_L1_L1C_D_P    = SignalId::from_lpp(SignalId::GPS, 17);

SignalId const SignalId::GLONASS_G1_CA   = SignalId::from_lpp(SignalId::GLONASS, 0);
SignalId const SignalId::GLONASS_G2_CA   = SignalId::from_lpp(SignalId::GLONASS, 1);
SignalId const SignalId::GLONASS_G3      = SignalId::from_lpp(SignalId::GLONASS, 2);
SignalId const SignalId::GLONASS_G1_P    = SignalId::from_lpp(SignalId::GLONASS, 3);
SignalId const SignalId::GLONASS_G2_P    = SignalId::from_lpp(SignalId::GLONASS, 4);
SignalId const SignalId::GLONASS_G1A_D   = SignalId::from_lpp(SignalId::GLONASS, 5);
SignalId const SignalId::GLONASS_G1A_P   = SignalId::from_lpp(SignalId::GLONASS, 6);
SignalId const SignalId::GLONASS_G1A_D_P = SignalId::from_lpp(SignalId::GLONASS, 7);
SignalId const SignalId::GLONASS_G2A_I   = SignalId::from_lpp(SignalId::GLONASS, 8);
SignalId const SignalId::GLONASS_G2A_P   = SignalId::from_lpp(SignalId::GLONASS, 9);
SignalId const SignalId::GLONASS_G2A_I_P = SignalId::from_lpp(SignalId::GLONASS, 10);
SignalId const SignalId::GLONASS_G3_I    = SignalId::from_lpp(SignalId::GLONASS, 11);
SignalId const SignalId::GLONASS_G3_Q    = SignalId::from_lpp(SignalId::GLONASS, 12);
SignalId const SignalId::GLONASS_G3_I_Q  = SignalId::from_lpp(SignalId::GLONASS, 13);

SignalId const SignalId::GALILEO_E1                   = SignalId::from_lpp(SignalId::GALILEO, 0);
SignalId const SignalId::GALILEO_E5A                  = SignalId::from_lpp(SignalId::GALILEO, 1);
SignalId const SignalId::GALILEO_E5B                  = SignalId::from_lpp(SignalId::GALILEO, 2);
SignalId const SignalId::GALILEO_E6                   = SignalId::from_lpp(SignalId::GALILEO, 3);
SignalId const SignalId::GALILEO_E5A_E5B              = SignalId::from_lpp(SignalId::GALILEO, 4);
SignalId const SignalId::GALILEO_E1_C_NO_DATA         = SignalId::from_lpp(SignalId::GALILEO, 5);
SignalId const SignalId::GALILEO_E1_A                 = SignalId::from_lpp(SignalId::GALILEO, 6);
SignalId const SignalId::GALILEO_E1_B_I_NAV_OS_CS_SOL = SignalId::from_lpp(SignalId::GALILEO, 7);
SignalId const SignalId::GALILEO_E1_B_C               = SignalId::from_lpp(SignalId::GALILEO, 8);
SignalId const SignalId::GALILEO_E1_A_B_C             = SignalId::from_lpp(SignalId::GALILEO, 9);
SignalId const SignalId::GALILEO_E6_C                 = SignalId::from_lpp(SignalId::GALILEO, 10);
SignalId const SignalId::GALILEO_E6_A                 = SignalId::from_lpp(SignalId::GALILEO, 11);
SignalId const SignalId::GALILEO_E6_B                 = SignalId::from_lpp(SignalId::GALILEO, 12);
SignalId const SignalId::GALILEO_E6_B_C               = SignalId::from_lpp(SignalId::GALILEO, 13);
SignalId const SignalId::GALILEO_E6_A_B_C             = SignalId::from_lpp(SignalId::GALILEO, 14);
SignalId const SignalId::GALILEO_E5B_I                = SignalId::from_lpp(SignalId::GALILEO, 15);
SignalId const SignalId::GALILEO_E5B_Q                = SignalId::from_lpp(SignalId::GALILEO, 16);
SignalId const SignalId::GALILEO_E5B_I_Q              = SignalId::from_lpp(SignalId::GALILEO, 17);
SignalId const SignalId::GALILEO_E5_A_B_I             = SignalId::from_lpp(SignalId::GALILEO, 18);
SignalId const SignalId::GALILEO_E5_A_B_Q             = SignalId::from_lpp(SignalId::GALILEO, 19);
SignalId const SignalId::GALILEO_E5_A_B_I_Q           = SignalId::from_lpp(SignalId::GALILEO, 20);
SignalId const SignalId::GALILEO_E5A_I                = SignalId::from_lpp(SignalId::GALILEO, 21);
SignalId const SignalId::GALILEO_E5A_Q                = SignalId::from_lpp(SignalId::GALILEO, 22);
SignalId const SignalId::GALILEO_E5A_I_Q              = SignalId::from_lpp(SignalId::GALILEO, 23);

SignalId const SignalId::BEIDOU_B1_I    = SignalId::from_lpp(SignalId::BEIDOU, 0);
SignalId const SignalId::BEIDOU_B1_Q    = SignalId::from_lpp(SignalId::BEIDOU, 1);
SignalId const SignalId::BEIDOU_B1_I_Q  = SignalId::from_lpp(SignalId::BEIDOU, 2);
SignalId const SignalId::BEIDOU_B3_I    = SignalId::from_lpp(SignalId::BEIDOU, 3);
SignalId const SignalId::BEIDOU_B3_Q    = SignalId::from_lpp(SignalId::BEIDOU, 4);
SignalId const SignalId::BEIDOU_B3_I_Q  = SignalId::from_lpp(SignalId::BEIDOU, 5);
SignalId const SignalId::BEIDOU_B2_I    = SignalId::from_lpp(SignalId::BEIDOU, 6);
SignalId const SignalId::BEIDOU_B2_Q    = SignalId::from_lpp(SignalId::BEIDOU, 7);
SignalId const SignalId::BEIDOU_B2_I_Q  = SignalId::from_lpp(SignalId::BEIDOU, 8);
SignalId const SignalId::BEIDOU_B1C_D   = SignalId::from_lpp(SignalId::BEIDOU, 9);
SignalId const SignalId::BEIDOU_B1C_P   = SignalId::from_lpp(SignalId::BEIDOU, 10);
SignalId const SignalId::BEIDOU_B1C_D_P = SignalId::from_lpp(SignalId::BEIDOU, 11);
SignalId const SignalId::BEIDOU_B2A_D   = SignalId::from_lpp(SignalId::BEIDOU, 12);
SignalId const SignalId::BEIDOU_B2A_P   = SignalId::from_lpp(SignalId::BEIDOU, 13);
SignalId const SignalId::BEIDOU_B2A_D_P = SignalId::from_lpp(SignalId::BEIDOU, 14);

//
// GPS
//

static std::string GPS_NAMES[24] = {
    "L1 C/A",   "L1C",      "L2C",           "L5",        "L1 P",      "L1 Z-tracking",
    "L2 C/A",   "L2 P",     "L2 Z-tracking", "L2 L2C(M)", "L2 L2C(L)", "L2 L2C(M+L)",
    "L5 I",     "L5 Q",     "L5 I+Q",        "L1 L1C(D)", "L1 L1C(P)", "L1 L1C(D+P)",
    "Reserved", "Reserved", "Reserved",      "Reserved",  "Reserved",  "Reserved",
};

static std::string GPS_RINEX_NAMES[24] = {
    "1C",  // L1 C/A
    "??",  // L1C
    "??",  // L2C
    "??",  // L5
    "1P",  // L1 P
    "1W",  // L1 Z-tracking
    "2C",  // L2 C/A
    "2P",  // L2 P
    "2W",  // L2 Z-tracking
    "2S",  // L2 L2C(M)
    "2L",  // L2 L2C(L)
    "2X",  // L2 L2C(M+L)
    "5I",  // L5 I
    "5Q",  // L5 Q
    "5X",  // L5 I+Q
    "1S",  // L1 L1C(D)
    "1L",  // L1 L1C(P)
    "1X",  // L1 L1C(D+P)
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
};

CONSTEXPR static int32_t GPS_LPP_TO_RTCM[24] = {
    2,   // L1 C/A
    -1,  // L1C
    -1,  // L2C
    -1,  // L5
    3,   // L1 P
    4,   // L1 Z-tracking
    8,   // L2 C/A
    9,   // L2 P
    10,  // L2 Z-tracking
    15,  // L2 L2C(M)
    16,  // L2 L2C(L)
    17,  // L2 L2C(M+L)
    22,  // L5 I
    23,  // L5 Q
    24,  // L5 I+Q
    30,  // L1 L1C(D)
    31,  // L1 L1C(P)
    32,  // L1 L1C(D+P)
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
};

#define GPS_L1_FREQ 1575.42e3
#define GPS_L2_FREQ 1227.60e3
#define GPS_L5_FREQ 1176.45e3

static double GPS_FREQ[24] = {
    GPS_L1_FREQ,  // L1 C/A
    GPS_L1_FREQ,  // L1C
    GPS_L2_FREQ,  // L2C
    GPS_L5_FREQ,  // L5
    GPS_L1_FREQ,  // L1 P
    GPS_L1_FREQ,  // L1 Z-tracking
    GPS_L2_FREQ,  // L2 C/A
    GPS_L2_FREQ,  // L2 P
    GPS_L2_FREQ,  // L2 Z-tracking
    GPS_L2_FREQ,  // L2 L2C(M)
    GPS_L2_FREQ,  // L2 L2C(L)
    GPS_L2_FREQ,  // L2 L2C(M+L)
    GPS_L5_FREQ,  // L5 I
    GPS_L5_FREQ,  // L5 Q
    GPS_L5_FREQ,  // L5 I+Q
    GPS_L1_FREQ,  // L1 L1C(D)
    GPS_L1_FREQ,  // L1 L1C(P)
    GPS_L1_FREQ,  // L1 L1C(D+P)
    0.0,          // Reserved
    0.0,          // Reserved
    0.0,          // Reserved
    0.0,          // Reserved
    0.0,          // Reserved
    0.0,          // Reserved
};

static FrequencyType GPS_FREQ_TYPE[24] = {
    FrequencyType::L1,       // L1 C/A
    FrequencyType::L1,       // L1C
    FrequencyType::L2,       // L2C
    FrequencyType::L5,       // L5
    FrequencyType::L1,       // L1 P
    FrequencyType::L1,       // L1 Z-tracking
    FrequencyType::L2,       // L2 C/A
    FrequencyType::L2,       // L2 P
    FrequencyType::L2,       // L2 Z-tracking
    FrequencyType::L2,       // L2 L2C(M)
    FrequencyType::L2,       // L2 L2C(L)
    FrequencyType::L2,       // L2 L2C(M+L)
    FrequencyType::L5,       // L5 I
    FrequencyType::L5,       // L5 Q
    FrequencyType::L5,       // L5 I+Q
    FrequencyType::L1,       // L1 L1C(D)
    FrequencyType::L1,       // L1 L1C(P)
    FrequencyType::L1,       // L1 L1C(D+P)
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
};

//
// GLONASS
//
static std::string GLONASS_NAMES[24] = {
    "G1 C/A",   "G2 C/A",   "G3",       "G1 P",     "G2 P",     "G1a(D)",   "G1a(P)",   "G1a (D+P)",
    "G2a(I)",   "G2a(P)",   "G2a(I+P)", "G3 I",     "G3 Q",     "G3 I+Q",   "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

static std::string GLONASS_RINEX_NAMES[24] = {
    "1C",  // G1 C/A
    "2C",  // G2 C/A
    "??",  // G3
    "1P",  // G1 P
    "2P",  // G2 P
    "4A",  // G1a(D)
    "4B",  // G1a(P)
    "4X",  // G1a (D+P)
    "6A",  // G2a(I)
    "6B",  // G2a(P)
    "6X",  // G2a(I+P)
    "3I",  // G3 I
    "3Q",  // G3 Q
    "3X",  // G3 I+Q
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
};

CONSTEXPR static int32_t GLONASS_LPP_TO_RTCM[24] = {
    2,   // G1 C/A
    8,   // G2 C/A
    -1,  // G3
    3,   // G1 P
    9,   // G2 P
    -1,  // G1a(D)
    -1,  // G1a(P)
    -1,  // G1a (D+P)
    -1,  // G2a(I)
    -1,  // G2a(P)
    -1,  // G2a(I+P)
    -1,  // G3 I
    -1,  // G3 Q
    -1,  // G3 I+Q
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
};

#define GLONASS_G1_FREQ 1602.0e3
#define GLONASS_G2_FREQ 1246.0e3
#define GLONASS_G3_FREQ 1202.025e3

static double GLONASS_FREQ[24] = {
    GLONASS_G1_FREQ,  // G1 C/A
    GLONASS_G2_FREQ,  // G2 C/A
    GLONASS_G3_FREQ,  // G3
    GLONASS_G1_FREQ,  // G1 P
    GLONASS_G2_FREQ,  // G2 P
    GLONASS_G1_FREQ,  // G1a(D)
    GLONASS_G1_FREQ,  // G1a(P)
    GLONASS_G1_FREQ,  // G1a (D+P)
    GLONASS_G2_FREQ,  // G2a(I)
    GLONASS_G2_FREQ,  // G2a(P)
    GLONASS_G2_FREQ,  // G2a(I+P)
    GLONASS_G3_FREQ,  // G3 I
    GLONASS_G3_FREQ,  // G3 Q
    GLONASS_G3_FREQ,  // G3 I+Q
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
};

static FrequencyType GLONASS_FREQ_TYPE[24] = {
    FrequencyType::G1,       // G1 C/A
    FrequencyType::G2,       // G2 C/A
    FrequencyType::G3,       // G3
    FrequencyType::G1,       // G1 P
    FrequencyType::G2,       // G2 P
    FrequencyType::G1,       // G1a(D)
    FrequencyType::G1,       // G1a(P)
    FrequencyType::G1,       // G1a (D+P)
    FrequencyType::G2,       // G2a(I)
    FrequencyType::G2,       // G2a(P)
    FrequencyType::G2,       // G2a(I+P)
    FrequencyType::G3,       // G3 I
    FrequencyType::G3,       // G3 Q
    FrequencyType::G3,       // G3 I+Q
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
};

//
// Galileo
//
static std::string GALILEO_NAMES[24] = {
    "E1",          "E5A",          "E5B",       "E6",
    "E5A + E5B",   "E1 C No data", "E1 A",      "E1 B I/NAV OS/CS/SoL",
    "E1 B+C",      "E1 A+B+C",     "E6 C",      "E6 A",
    "E6 B",        "E6 B+C",       "E6 A+B+C",  "E5B I",
    "E5B Q",       "E5B I+Q",      "E5(A+B) I", "E5(A+B) Q",
    "E5(A+B) I+Q", "E5A I",        "E5A Q",     "E5A I+Q",
};

static std::string GALILEO_RINEX_NAMES[24] = {
    "??",  // E1
    "??",  // E5A
    "??",  // E5B
    "??",  // E6
    "??",  // E5A + E5B
    "1C",  // E1 C No data
    "1A",  // E1 A
    "1B",  // E1 B I/NAV OS/CS/SoL
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

CONSTEXPR static int32_t GALILEO_LPP_TO_RTCM[24] = {
    -1,  // E1
    -1,  // E5A
    -1,  // E5B
    -1,  // E6
    -1,  // E5A + E5B
    2,   // E1 C No data
    3,   // E1 A
    4,   // E1 B I/NAV OS/CS/SoL
    5,   // E1 B+C
    6,   // E1 A+B+C
    8,   // E6 C
    9,   // E6 A
    10,  // E6 B
    11,  // E6 B+C
    12,  // E6 A+B+C
    14,  // E5B I
    15,  // E5B Q
    16,  // E5B I+Q
    18,  // E5(A+B) I
    19,  // E5(A+B) Q
    20,  // E5(A+B) I+Q
    22,  // E5A I
    23,  // E5A Q
    24,  // E5A I+Q
};

#define GALILEO_E1_FREQ 1575.42e3
#define GALILEO_E5A_FREQ 1176.45e3
#define GALILEO_E5B_FREQ 1207.14e3
#define GALILEO_E6_FREQ 1278.75e3
#define GALILEO_E5A_E5B_FREQ 1191.795e3

static double GALILEO_FREQ[24] = {
    GALILEO_E1_FREQ,       // E1
    GALILEO_E5A_FREQ,      // E5A
    GALILEO_E5B_FREQ,      // E5B
    GALILEO_E6_FREQ,       // E6
    GALILEO_E5A_E5B_FREQ,  // E5A + E5B
    GALILEO_E1_FREQ,       // E1 C No data
    GALILEO_E1_FREQ,       // E1 A
    GALILEO_E1_FREQ,       // E1 B I/NAV OS/CS/SoL
    GALILEO_E1_FREQ,       // E1 B+C
    GALILEO_E1_FREQ,       // E1 A+B+C
    GALILEO_E6_FREQ,       // E6 C
    GALILEO_E6_FREQ,       // E6 A
    GALILEO_E6_FREQ,       // E6 B
    GALILEO_E6_FREQ,       // E6 B+C
    GALILEO_E6_FREQ,       // E6 A+B+C
    GALILEO_E5B_FREQ,      // E5B I
    GALILEO_E5B_FREQ,      // E5B Q
    GALILEO_E5B_FREQ,      // E5B I+Q
    GALILEO_E5A_E5B_FREQ,  // E5(A+B) I
    GALILEO_E5A_E5B_FREQ,  // E5(A+B) Q
    GALILEO_E5A_E5B_FREQ,  // E5(A+B) I+Q
    GALILEO_E5A_FREQ,      // E5A I
    GALILEO_E5A_FREQ,      // E5A Q
    GALILEO_E5A_FREQ       // E5A I+Q
};

static FrequencyType GALILEO_FREQ_TYPE[24] = {
    FrequencyType::E1,   // E1
    FrequencyType::E5a,  // E5A
    FrequencyType::E5b,  // E5B
    FrequencyType::E6,   // E6
    FrequencyType::E5,   // E5A + E5B
    FrequencyType::E1,   // E1 C No data
    FrequencyType::E1,   // E1 A
    FrequencyType::E1,   // E1 B I/NAV OS/CS/SoL
    FrequencyType::E1,   // E1 B+C
    FrequencyType::E1,   // E1 A+B+C
    FrequencyType::E6,   // E6 C
    FrequencyType::E6,   // E6 A
    FrequencyType::E6,   // E6 B
    FrequencyType::E6,   // E6 B+C
    FrequencyType::E6,   // E6 A+B+C
    FrequencyType::E5b,  // E5B I
    FrequencyType::E5b,  // E5B Q
    FrequencyType::E5b,  // E5B I+Q
    FrequencyType::E5,   // E5(A+B) I
    FrequencyType::E5,   // E5(A+B) Q
    FrequencyType::E5,   // E5(A+B) I+Q
    FrequencyType::E5a,  // E5A I
    FrequencyType::E5a,  // E5A Q
    FrequencyType::E5a   // E5A I+Q
};

//
// BDS
//

static std::string BDS_NAMES[24] = {
    "B1 I",     "B1 Q",     "B1 I+Q",   "B3 I",     "B3 Q",     "B3 I+Q",   "B2 I",     "B2 Q",
    "B2 I+Q",   "B1C(D)",   "B1C(P)",   "B1C(D+P)", "B2a(D)",   "B2a(P)",   "B2a(D+P)", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

static std::string BDS_RINEX_NAMES[24] = {
    "2I",  // B1 I
    "2Q",  // B1 Q
    "2X",  // B1 I+Q
    "6I",  // B3 I
    "6Q",  // B3 Q
    "6X",  // B3 I+Q
    "7I",  // B2 I
    "7Q",  // B2 Q
    "7X",  // B2 I+Q
    "1D",  // B1C(D)
    "1P",  // B1C(P)
    "1X",  // B1C(D+P)
    "5D",  // B2a(D)
    "5P",  // B2a(P)
    "5X",  // B2a(D+P)
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
    "??",  // Reserved
};

CONSTEXPR static int32_t BDS_LPP_TO_RTCM[24] = {
    2,   // B1 I
    3,   // B1 Q
    4,   // B1 I+Q
    8,   // B3 I
    9,   // B3 Q
    10,  // B3 I+Q
    14,  // B2 I
    15,  // B2 Q
    16,  // B2 I+Q
    30,  // B1C(D)
    31,  // B1C(P)
    32,  // B1C(D+P)
    22,  // B2a(D)
    23,  // B2a(P)
    24,  // B2a(D+P)
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
    -1,  // Reserved
};

#define BEIDOU_B1_FREQ 1561.098e3
#define BEIDOU_B2_FREQ 1207.14e3
#define BEIDOU_B3_FREQ 1268.52e3
#define BEIDOU_B2a_FREQ 1176.45e3
#define BEIDOU_B1C_FREQ 1575.42e3

static double BDS_FREQ[24] = {
    BEIDOU_B1_FREQ,   // B1 I
    BEIDOU_B1_FREQ,   // B1 Q
    BEIDOU_B1_FREQ,   // B1 I+Q
    BEIDOU_B3_FREQ,   // B3 I
    BEIDOU_B3_FREQ,   // B3 Q
    BEIDOU_B3_FREQ,   // B3 I+Q
    BEIDOU_B2_FREQ,   // B2 I
    BEIDOU_B2_FREQ,   // B2 Q
    BEIDOU_B2_FREQ,   // B2 I+Q
    BEIDOU_B1C_FREQ,  // B1C(D)
    BEIDOU_B1C_FREQ,  // B1C(P)
    BEIDOU_B1C_FREQ,  // B1C(D+P)
    BEIDOU_B2a_FREQ,  // B2a(D)
    BEIDOU_B2a_FREQ,  // B2a(P)
    BEIDOU_B2a_FREQ,  // B2a(D+P)
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0,              // Reserved
    0.0               // Reserved
};

static FrequencyType BDS_FREQ_TYPE[24] = {
    FrequencyType::B1,       // B1 I
    FrequencyType::B1,       // B1 Q
    FrequencyType::B1,       // B1 I+Q
    FrequencyType::B3,       // B3 I
    FrequencyType::B3,       // B3 Q
    FrequencyType::B3,       // B3 I+Q
    FrequencyType::B2,       // B2 I
    FrequencyType::B2,       // B2 Q
    FrequencyType::B2,       // B2 I+Q
    FrequencyType::B1,       // B1C(D)
    FrequencyType::B1,       // B1C(P)
    FrequencyType::B1,       // B1C(D+P)
    FrequencyType::B2a,      // B2a(D)
    FrequencyType::B2a,      // B2a(P)
    FrequencyType::B2a,      // B2a(D+P)
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN,  // Reserved
    FrequencyType::UNKNOWN   // Reserved
};

SignalId SignalId::from_lpp(Gnss gnss, long id) {
    if (id >= 24) {
        return {};
    }

    if (gnss == Gnss::GPS) {
        return SignalId(gnss, static_cast<int32_t>(id));
    } else if (gnss == Gnss::GLONASS) {
        return SignalId(gnss, static_cast<int32_t>(id));
    } else if (gnss == Gnss::GALILEO) {
        return SignalId(gnss, static_cast<int32_t>(id));
    } else if (gnss == Gnss::BEIDOU) {
        return SignalId(gnss, static_cast<int32_t>(id));
    } else {
        CORE_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        return {};
#endif
    }
}

std::string SignalId::to_string() const {
    auto id = lpp_id();

    if (mGnss == Gnss::GPS) {
        return GPS_NAMES[id];
    } else if (mGnss == Gnss::GLONASS) {
        return GLONASS_NAMES[id];
    } else if (mGnss == Gnss::GALILEO) {
        return GALILEO_NAMES[id];
    } else if (mGnss == Gnss::BEIDOU) {
        return BDS_NAMES[id];
    } else {
        CORE_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        return "Invalid GNSS";
#endif
    }
}

std::string SignalId::to_rinex() const {
    auto id = lpp_id();

    if (mGnss == Gnss::GPS) {
        return GPS_RINEX_NAMES[id];
    } else if (mGnss == Gnss::GLONASS) {
        return GLONASS_RINEX_NAMES[id];
    } else if (mGnss == Gnss::GALILEO) {
        return GALILEO_RINEX_NAMES[id];
    } else if (mGnss == Gnss::BEIDOU) {
        return BDS_RINEX_NAMES[id];
    } else {
        CORE_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        return "???";
#endif
    }
}

char const* SignalId::name() const {
    auto id = lpp_id();

    if (mGnss == Gnss::GPS) {
        return GPS_NAMES[id].c_str();
    } else if (mGnss == Gnss::GLONASS) {
        return GLONASS_NAMES[id].c_str();
    } else if (mGnss == Gnss::GALILEO) {
        return GALILEO_NAMES[id].c_str();
    } else if (mGnss == Gnss::BEIDOU) {
        return BDS_NAMES[id].c_str();
    } else {
        CORE_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        return "Invalid GNSS";
#endif
    }
}

double SignalId::frequency() const {
    auto id = lpp_id();

    if (mGnss == Gnss::GPS) {
        return GPS_FREQ[id];
    } else if (mGnss == Gnss::GLONASS) {
        return GLONASS_FREQ[id];
    } else if (mGnss == Gnss::GALILEO) {
        return GALILEO_FREQ[id];
    } else if (mGnss == Gnss::BEIDOU) {
        return BDS_FREQ[id];
    } else {
        CORE_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        return 0.0;
#endif
    }
}

FrequencyType SignalId::frequency_type() const {
    auto id = lpp_id();

    if (mGnss == Gnss::GPS) {
        return GPS_FREQ_TYPE[id];
    } else if (mGnss == Gnss::GLONASS) {
        return GLONASS_FREQ_TYPE[id];
    } else if (mGnss == Gnss::GALILEO) {
        return GALILEO_FREQ_TYPE[id];
    } else if (mGnss == Gnss::BEIDOU) {
        return BDS_FREQ_TYPE[id];
    } else {
        CORE_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
        return FrequencyType::UNKNOWN;
#endif
    }
}

double SignalId::wavelength() const {
    return 299792458.0 / frequency();
}

Maybe<long> SignalId::as_msm() const {
    if (mLppId < 0 || mLppId >= 24) {
        return Maybe<long>();
    }

    auto result = -1;
    if (mGnss == Gnss::GPS) {
        result = GPS_LPP_TO_RTCM[mLppId];
    } else if (mGnss == Gnss::GLONASS) {
        result = GLONASS_LPP_TO_RTCM[mLppId];
    } else if (mGnss == Gnss::GALILEO) {
        result = GALILEO_LPP_TO_RTCM[mLppId];
    } else if (mGnss == Gnss::BEIDOU) {
        result = BDS_LPP_TO_RTCM[mLppId];
    } else {
        assert(false);
        return Maybe<long>();
    }

    if (result < 0) {
        return Maybe<long>();
    }

    return result;
}

long SignalId::lpp_id() const {
    assert(mLppId >= 0 && mLppId < 24);
    return mLppId;
}

std::string SatelliteSignalId::to_string() const {
    return mSatellite.to_string() + " " + mSignal.to_string();
}

long SignalId::absolute_id() const {
    if (mLppId >= SIGNAL_ABS_COUNT) {
        return -1;
    }

    return mLppId;
}
