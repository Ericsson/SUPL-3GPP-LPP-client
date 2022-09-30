#include "utility/satellite_id.h"

#include <cassert>

constexpr static long MAX_LPP_SATELLITE_ID = 64;

constexpr static s32 MSM_TO_LPP[65] = {
    -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
};

constexpr static s32 DF009_TO_LPP[65] = {
    -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
};

constexpr static s32 DF038_TO_LPP[65] = {
    -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr static s32 DF068_TO_LPP[65] = {
    -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

constexpr static s32 DF252_TO_LPP[65] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1,
};

constexpr static s32 DF466_TO_LPP[65] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, -1,
};

Optional<SatelliteID> SatelliteID::from_lpp(GNSS_System system, u32 id) {
    if (id >= MAX_LPP_SATELLITE_ID) {
        return {};
    }

    return SatelliteID{system, static_cast<s32>(id)};
}

Optional<SatelliteID> SatelliteID::from_msm(GNSS_System system, u32 id) {
    if (id == 0 || id > MAX_LPP_SATELLITE_ID) {
        return {};
    }

    auto result = -1;
    if (system == GNSS_System::GPS) {
        result = MSM_TO_LPP[id];
    } else if (system == GNSS_System::GLONASS) {
        result = MSM_TO_LPP[id];
    } else if (system == GNSS_System::GALILEO) {
        result = MSM_TO_LPP[id];
    } else if (system == GNSS_System::BDS) {
        result = MSM_TO_LPP[id];
    } else {
        assert(false);
        return {};
    }

    if (result < 0) {
        return {};
    }

    return SatelliteID{system, result};
}

Optional<SatelliteID> SatelliteID::from_df009(GNSS_System system, u32 id) {
    assert(id <= 64);
    assert(system == GNSS_System::GPS);

    auto result = DF009_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SatelliteID{system, result};
}

Optional<SatelliteID> SatelliteID::from_df038(GNSS_System system, u32 id) {
    assert(id <= 64);
    assert(system == GNSS_System::GLONASS);

    auto result = DF038_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SatelliteID{system, result};
}

Optional<SatelliteID> SatelliteID::from_df068(GNSS_System system, u32 id) {
    assert(id <= 64);
    assert(system == GNSS_System::GPS);

    auto result = DF068_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SatelliteID{system, result};
}

Optional<SatelliteID> SatelliteID::from_df252(GNSS_System system, u32 id) {
    assert(id <= 64);
    assert(system == GNSS_System::GALILEO);

    auto result = DF252_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SatelliteID{system, result};
}

Optional<SatelliteID> SatelliteID::from_df466(GNSS_System system, u32 id) {
    assert(id <= 64);
    assert(system == GNSS_System::BDS);

    auto result = DF466_TO_LPP[id];
    if (result < 0) {
        return {};
    }

    return SatelliteID{system, result};
}

long SatelliteID::lpp_id() const {
    assert(mId >= 0 && mId < MAX_LPP_SATELLITE_ID);
    return mId;
}

Optional<long> SatelliteID::as_df009() const {
    assert(mSystem == GNSS_System::GPS);
    return mId + 1;
}

Optional<long> SatelliteID::as_df038() const {
    assert(mSystem == GNSS_System::GLONASS && mId < 24);
    return mId + 1;
}

