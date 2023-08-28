#include "satellite_id.hpp"
#include <cinttypes>

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

Maybe<long> SatelliteId::as_msm() const {
    if (mLppId >= 0 && mLppId < LPP_SATELLITE_ID_LIMIT) {
        return mLppId + 1;
    } else {
        return Maybe<long>();
    }
}

SatelliteId::Gnss SatelliteId::gnss() const {
    return mGnss;
}

std::string SatelliteId::to_string() const {
    char buffer[32];
    switch (mGnss) {
    case Gnss::GPS: snprintf(buffer, sizeof(buffer), "G%02" PRId32 "", mLppId); break;
    case Gnss::GLONASS: snprintf(buffer, sizeof(buffer), "R%02" PRId32 "", mLppId); break;
    case Gnss::GALILEO: snprintf(buffer, sizeof(buffer), "E%02" PRId32 "", mLppId); break;
    case Gnss::BEIDOU: snprintf(buffer, sizeof(buffer), "C%02" PRId32 "", mLppId); break;
    default: snprintf(buffer, sizeof(buffer), "U%02" PRId32 "", mLppId); break;
    }
    return std::string(buffer);
}