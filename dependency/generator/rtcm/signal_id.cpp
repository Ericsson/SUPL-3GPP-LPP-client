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

//
// GPS
//

static std::string GPS_NAMES[24] = {
    "L1 C/A",   "L1C",      "L2C",           "L5",        "L1 P",      "L1 Z-tracking",
    "L2 C/A",   "L2 P",     "L2 Z-tracking", "L2 L2C(M)", "L2 L2C(L)", "L2 L2C(M+L)",
    "L5 I",     "L5 Q",     "L5 I+Q",        "L1 L1C(D)", "L1 L1C(P)", "L1 L1C(D+P)",
    "Reserved", "Reserved", "Reserved",      "Reserved",  "Reserved",  "Reserved",
};

CONSTEXPR static int32_t GPS_LPP_TO_RTCM[24] = {
    2, -1, -1, -1, 3, 4, 8, 9, 10, 15, 16, 17, 22, 23, 24, 30, 31, 32, -1, -1, -1, -1, -1, -1,
};

#define GPS_L1_FREQ 1575.42e3
#define GPS_L2_FREQ 1227.60e3
#define GPS_L5_FREQ 1176.45e3

static double GPS_FREQ[24] = {
    GPS_L1_FREQ, GPS_L1_FREQ, GPS_L2_FREQ, GPS_L5_FREQ, GPS_L1_FREQ, GPS_L1_FREQ,
    GPS_L2_FREQ, GPS_L2_FREQ, GPS_L2_FREQ, GPS_L2_FREQ, GPS_L2_FREQ, GPS_L2_FREQ,
    GPS_L5_FREQ, GPS_L5_FREQ, GPS_L5_FREQ, GPS_L1_FREQ, GPS_L1_FREQ, GPS_L1_FREQ,
    0.0,         0.0,         0.0,         0.0,         0.0,         0.0,
};

//
// GLONASS
//
static std::string GLONASS_NAMES[24] = {
    "G1 C/A",   "G2 C/A",   "G3",       "G1 P",     "G2 P",     "G1a(D)",   "G1a(P)",   "G1a (D+P)",
    "G2a(I)",   "G2a(P)",   "G2a(I+P)", "G3 I",     "G3 Q",     "G3 I+Q",   "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

CONSTEXPR static int32_t GLONASS_LPP_TO_RTCM[24] = {
    2, 8, -1, 3, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

#define GLONASS_G1_FREQ 1602.0e3
#define GLONASS_G2_FREQ 1246.0e3
#define GLONASS_G3_FREQ 1202.025e3

static double GLONASS_FREQ[24] = {
    GLONASS_G1_FREQ,
    GLONASS_G2_FREQ,
    GLONASS_G3_FREQ,
    GLONASS_G1_FREQ,
    GLONASS_G2_FREQ,
    GLONASS_G1_FREQ,
    GLONASS_G1_FREQ,
    GLONASS_G1_FREQ,
    GLONASS_G2_FREQ,
    GLONASS_G2_FREQ,
    GLONASS_G2_FREQ,
    GLONASS_G3_FREQ,
    GLONASS_G3_FREQ,
    GLONASS_G3_FREQ,
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

CONSTEXPR static int32_t GALILEO_LPP_TO_RTCM[24] = {
    -1, -1, -1, -1, -1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 16, 18, 19, 20, 22, 23, 24,
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

//
// BDS
//

static std::string BDS_NAMES[24] = {
    "B1 I",     "B1 Q",     "B1 I+Q",   "B3 I",     "B3 Q",     "B3 I+Q",   "B2 I",     "B2 Q",
    "B2 I+Q",   "B1C(D)",   "B1C(P)",   "B1C(D+P)", "B2a(D)",   "B2a(P)",   "B2a(D+P)", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

CONSTEXPR static int32_t BDS_LPP_TO_RTCM[24] = {
    2, 3, 4, 8, 9, 10, 14, 15, 16, 30, 31, 32, 22, 23, 24, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

#define BEIDOU_B1_FREQ 1561.098e3
#define BEIDOU_B2_FREQ 1207.14e3
#define BEIDOU_B3_FREQ 1268.52e3
#define BEIDOU_B2a_FREQ 1176.45e3
#define BEIDOU_B2b_FREQ 1207.140e3
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
