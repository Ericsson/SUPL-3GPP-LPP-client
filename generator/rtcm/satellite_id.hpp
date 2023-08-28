#pragma once
#include <functional>
#include <string>
#include "maybe.hpp"
#include "types.hpp"

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
    RTCM_NODISCARD Maybe<long> as_msm() const;

    RTCM_NODISCARD Gnss gnss() const;
    RTCM_NODISCARD std::string to_string() const;

    inline bool operator==(const SatelliteId& other) const {
        return mGnss == other.mGnss && mLppId == other.mLppId;
    }

    inline bool operator!=(const SatelliteId& other) const { return !(*this == other); }

private:
    explicit SatelliteId(Gnss gnss, int32_t lpp_id) : mGnss(gnss), mLppId(lpp_id) {}

    Gnss    mGnss  = Gnss::UNKNOWN;
    int32_t mLppId = -1;
};

namespace std {
template <>
struct hash<SatelliteId> {
    std::size_t operator()(const SatelliteId& k) const RTCM_NOEXCEPT {
        return ((std::hash<int>()(static_cast<int>(k.gnss())) ^ (std::hash<int>()(k.lpp_id().value) << 1)) >> 1);
    }
};
}  // namespace std
