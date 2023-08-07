#pragma once
#include "types.hpp"
#include "maybe.hpp"

class SatelliteId {
public:
    SatelliteId() = default;

    enum Gnss {
        UNKNOWN,
        GPS,
        GLONASS,
        GALILEO,
        BEIDOU,
    };

    RTCM_NODISCARD static SatelliteId invalid();
    RTCM_NODISCARD static SatelliteId from_lpp(Gnss gnss, long id);

    //
    //
    //

    RTCM_NODISCARD Maybe<long> lpp_id() const;
    RTCM_NODISCARD Maybe<long> as_df009() const;
    RTCM_NODISCARD Maybe<long> as_df038() const;

    inline bool operator==(const SatelliteId& other) const {
        return mGnss == other.mGnss && mLppId == other.mLppId;
    }

    inline bool operator!=(const SatelliteId& other) const { return !(*this == other); }

private:
    explicit SatelliteId(Gnss gnss, int32_t lpp_id) : mGnss(gnss), mLppId(lpp_id) {}

    Gnss mGnss;
    int32_t mLppId;
};
