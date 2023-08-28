#pragma once
#include <string>
#include "maybe.hpp"
#include "types.hpp"

class SignalId {
public:
    SignalId() = default;

    enum Gnss {
        UNKNOWN,
        GPS,
        GLONASS,
        GALILEO,
        BEIDOU,
    };

    RTCM_NODISCARD static SignalId from_lpp(Gnss gnss, long id);

    //
    //
    //

    RTCM_NODISCARD Maybe<long> as_msm() const;
    RTCM_NODISCARD long        lpp_id() const;
    RTCM_NODISCARD std::string to_string() const;

    inline bool operator==(const SignalId& other) const {
        return mGnss == other.mGnss && mLppId == other.mLppId;
    }

    inline bool operator!=(const SignalId& other) const { return !(*this == other); }

private:
    explicit SignalId(Gnss gnss, int32_t lpp_id) : mGnss(gnss), mLppId(lpp_id) {}

    Gnss    mGnss  = Gnss::UNKNOWN;
    int32_t mLppId = -1;
};