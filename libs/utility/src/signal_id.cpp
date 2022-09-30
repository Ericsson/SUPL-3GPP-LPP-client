#include "utility/signal_id.h"

#include <cassert>

//
// GPS
//

static std::string GPS_NAMES[24] = {
    "L1 C/A",   "L1C",      "L2C",           "L5",        "L1 P",      "L1 Z-tracking",
    "L2 C/A",   "L2 P",     "L2 Z-tracking", "L2 L2C(M)", "L2 L2C(L)", "L2 L2C(M+L)",
    "L5 I",     "L5 Q",     "L5 I+Q",        "L1 L1C(D)", "L1 L1C(P)", "L1 L1C(D+P)",
    "Reserved", "Reserved", "Reserved",      "Reserved",  "Reserved",  "Reserved",
};

constexpr static s32 GPS_RTCM_MSM_TO_LPP[33] = {
    -1, -1, 0,  4,  5,  -1, -1, -1, 6,  7,  8,  -1, -1, -1, -1, 9,  10,
    11, -1, -1, -1, -1, 12, 13, 14, -1, -1, -1, -1, -1, 15, 16, 17,
};

constexpr static s32 GPS_RTCM_DF380_TO_LPP[32] = {
    0,  4,  5,  -1, -1, 6,  -1 /* TODO(ewasjon): */,
    9,  10, 11, 7,  8,  -1, -1,
    12, 13, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1,
};

constexpr static s32 GPS_LPP_TO_RTCM[24] = {
    2, -1, -1, -1, 3, 4, 8, 9, 10, 15, 16, 17, 22, 23, 24, 30, 31, 32, -1, -1, -1, -1, -1, -1,
};

//
// GLONASS
//
static std::string GLONASS_NAMES[24] = {
    "G1 C/A",   "G2 C/A",   "G3",       "G1 P",     "G2 P",     "G1a(D)",   "G1a(P)",   "G1a (D+P)",
    "G2a(I)",   "G2a(P)",   "G2a(I+P)", "G3 I",     "G3 Q",     "G3 I+Q",   "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

constexpr static s32 GLONASS_RTCM_MSM_TO_LPP[33] = {
    -1, -1, 0,  3,  -1, -1, -1, -1, 1,  4,  -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr static s32 GLONASS_LPP_TO_RTCM[24] = {
    2, 8, -1, 3, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
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

constexpr static s32 GALILEO_RTCM_MSM_TO_LPP[33] = {
    -1, -1, 5,  6,  7,  8,  9,  -1, 10, 11, 12, 13, 14, -1, 15, 16, 17,
    -1, 18, 19, 20, -1, 21, 22, 23, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr static s32 GALILEO_RTCM_DF382_TO_LPP[32] = {
    6,  7,  5,  -1, -1, 21, 22, -1, 15, 16, -1, -1 /*E5I*/, -1 /*E5Q*/, -1, 11, 12,
    10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,         -1,         -1, -1, -1,
};

constexpr static s32 GALILEO_LPP_TO_RTCM[24] = {
    -1, -1, -1, -1, -1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 16, 18, 19, 20, 22, 23, 24,
};

//
// BDS
//

static std::string BDS_NAMES[24] = {
    "B1 I",     "B1 Q",     "B1 I+Q",   "B3 I",     "B3 Q",     "B3 I+Q",   "B2 I",     "B2 Q",
    "B2 I+Q",   "B1C(D)",   "B1C(P)",   "B1C(D+P)", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
};

constexpr static s32 BDS_RTCM_MSM_TO_LPP[33] = {
    -1, -1, 0,  1,  2,  -1, -1, -1, 3,  4,  5,  -1, -1, -1, 6,  7,  8,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr static s32 BDS_RTCM_DF467_TO_LPP[32] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr static s32 BDS_LPP_TO_RTCM[24] = {
    2, 3, 4, 8, 9, 10, 14, 15, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

Optional<SignalID> SignalID::from_rtcm_msm(GNSS_System system, u32 id) {
    if (id == 0 || id >= 33) {
        return {};
    }

    auto result = -1;
    if (system == GNSS_System::GPS) {
        result = GPS_RTCM_MSM_TO_LPP[id];
    } else if (system == GNSS_System::GLONASS) {
        result = GLONASS_RTCM_MSM_TO_LPP[id];
    } else if (system == GNSS_System::GALILEO) {
        result = GALILEO_RTCM_MSM_TO_LPP[id];
    } else if (system == GNSS_System::BDS) {
        result = BDS_RTCM_MSM_TO_LPP[id];
    } else {
        assert(false);
        return {};
    }

    if (result < 0) {
        return {};
    }

    return SignalID{system, result};
}

Optional<SignalID> SignalID::from_rtcm_df380(GNSS_System system, u32 id) {
    assert(id <= 32);
    assert(system == GNSS_System::GPS);

    auto result = GPS_RTCM_DF380_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SignalID{system, result};
}

Optional<SignalID> SignalID::from_rtcm_df382(GNSS_System system, u32 id) {
    assert(id <= 32);
    assert(system == GNSS_System::GALILEO);

    auto result = GALILEO_RTCM_DF382_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SignalID{system, result};
}

Optional<SignalID> SignalID::from_rtcm_df467(GNSS_System system, u32 id) {
    assert(id <= 32);
    assert(system == GNSS_System::BDS);

    auto result = BDS_RTCM_DF467_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SignalID{system, result};
}

Optional<SignalID> SignalID::from_lpp(GNSS_System system, u32 id) {
    if (id >= 24) {
        return {};
    }

    if (system == GNSS_System::GPS) {
        return SignalID{system, static_cast<s32>(id)};
    } else if (system == GNSS_System::GLONASS) {
        return SignalID{system, static_cast<s32>(id)};
    } else if (system == GNSS_System::GALILEO) {
        return SignalID{system, static_cast<s32>(id)};
    } else if (system == GNSS_System::BDS) {
        return SignalID{system, static_cast<s32>(id)};
    } else {
        assert(false);
        return {};
    }
}

const std::string SignalID::name() const {
    auto id = lpp_id();

    if (system == GNSS_System::GPS) {
        return GPS_NAMES[id];
    } else if (system == GNSS_System::GLONASS) {
        return GLONASS_NAMES[id];
    } else if (system == GNSS_System::GALILEO) {
        return GALILEO_NAMES[id];
    } else if (system == GNSS_System::BDS) {
        return BDS_NAMES[id];
    } else {
        assert(false);
        return "Invalid GNSS";
    }
}

Optional<long> SignalID::rtcm_msm_id() const {
    if (id < 0 || id >= 24) {
        return {};
    }

    auto result = -1;
    if (system == GNSS_System::GPS) {
        result = GPS_LPP_TO_RTCM[id];
    } else if (system == GNSS_System::GLONASS) {
        result = GLONASS_LPP_TO_RTCM[id];
    } else if (system == GNSS_System::GALILEO) {
        result = GALILEO_LPP_TO_RTCM[id];
    } else if (system == GNSS_System::BDS) {
        result = BDS_LPP_TO_RTCM[id];
    } else {
        assert(false);
        return {};
    }

    if (result < 0) {
        return {};
    }

    return result;
}

long SignalID::lpp_id() const {
    assert(id >= 0 && id < 24);
    return id;
}