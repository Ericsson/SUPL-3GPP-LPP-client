#pragma once
#include <generator/rtcm/maybe.hpp>

#include <functional>
#include <string>

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

    NODISCARD static SatelliteId invalid();
    NODISCARD static SatelliteId from_lpp(Gnss gnss, long id);
    NODISCARD static SatelliteId from_gps_prn(uint8_t prn);
    NODISCARD static SatelliteId from_gal_prn(uint8_t prn);
    NODISCARD static SatelliteId from_bds_prn(uint8_t prn);
    NODISCARD static SatelliteId from_string(std::string const& str);

    //
    //
    //

    NODISCARD Maybe<long> lpp_id() const;
    NODISCARD Maybe<long> as_df009() const;
    NODISCARD Maybe<long> as_df038() const;
    NODISCARD Maybe<long> as_msm() const;

    NODISCARD bool is_valid() const { return mGnss != Gnss::UNKNOWN && mLppId != -1; }
    NODISCARD bool is_gps() const { return mGnss == Gnss::GPS; }
    NODISCARD bool is_galileo() const { return mGnss == Gnss::GALILEO; }
    NODISCARD bool is_beidou() const { return mGnss == Gnss::BEIDOU; }
    NODISCARD bool is_glonass() const { return mGnss == Gnss::GLONASS; }

    NODISCARD Gnss gnss() const;
    NODISCARD std::string to_string() const;
    NODISCARD const char* name() const;

    inline bool operator==(SatelliteId const& other) const {
        return mGnss == other.mGnss && mLppId == other.mLppId;
    }

    inline bool operator!=(SatelliteId const& other) const { return !(*this == other); }

    inline bool operator<(SatelliteId const& other) const {
        if (mGnss < other.mGnss) return true;
        if (mGnss > other.mGnss) return false;
        return mLppId < other.mLppId;
    }

    inline bool operator>(SatelliteId const& other) const {
        if (mGnss > other.mGnss) return true;
        if (mGnss < other.mGnss) return false;
        return mLppId > other.mLppId;
    }

private:
    explicit SatelliteId(Gnss gnss, int32_t lpp_id) : mGnss(gnss), mLppId(lpp_id) {}

    Gnss    mGnss  = Gnss::UNKNOWN;
    int32_t mLppId = -1;
};

namespace std {
template <>
struct hash<SatelliteId> {
    std::size_t operator()(SatelliteId const& k) const NOEXCEPT {
        auto hash_gnss         = std::hash<int>()(static_cast<int>(k.gnss()));
        auto hash_lpp_id       = std::hash<long>()(k.lpp_id().value);
        auto hash_lpp_id_maybe = std::hash<bool>()(k.lpp_id().valid);
        return hash_gnss ^ hash_lpp_id ^ hash_lpp_id_maybe;
    }
};
}  // namespace std
