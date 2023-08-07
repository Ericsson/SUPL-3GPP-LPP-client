#include "satellite_id.hpp"

RTCM_CONSTEXPR static long LPP_SATELLITE_ID_LIMIT = 64;

SatelliteId SatelliteId::invalid() {
    return SatelliteId();
}

SatelliteId SatelliteId::from_lpp(Gnss gnss, long id) {
    if (id >= LPP_SATELLITE_ID_LIMIT) {
        return invalid();
    } else {
        return SatelliteId(gnss, static_cast<int32_t>(id));
    }
}

Maybe<long> SatelliteId::lpp_id() const {
    if (mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId;
    } else {
        return Maybe<long>();
    }
}

Maybe<long> SatelliteId::as_df009() const {
    if (mGnss == Gnss::GPS && mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId + 1;
    } else {
        return Maybe<long>();
    }
}

Maybe<long> SatelliteId::as_df038() const {
    if (mGnss == Gnss::GLONASS && mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId + 1;
    } else {
        return Maybe<long>();
    }
}
