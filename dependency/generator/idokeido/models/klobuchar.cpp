#include "klobuchar.hpp"

#include <loglet/loglet.hpp>
#include <time/gps.hpp>

LOGLET_MODULE2(idokeido, klobuchar);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, klobuchar)

namespace idokeido {

static Scalar sin_sc(Scalar x) {
    return std::sin(x * constant::sc2r);
}
static Scalar cos_sc(Scalar x) {
    return std::cos(x * constant::sc2r);
}

Scalar KlobucharModelParameters::evaluate(ts::Tai const& time, Scalar elevation, Scalar azimuth,
                                          Vector3 const& llh) const NOEXCEPT {
    FUNCTION_SCOPEF("time=%s, elevation=%f, azimuth=%f, llh=(%f, %f, %f)", time, elevation, azimuth,
                    llh.x(), llh.y(), llh.z());
    VERBOSEF("a: %+.14f, %+14f, %+14f, %+14f", a[0], a[1], a[2], a[3]);
    VERBOSEF("b: %+.14f, %+14f, %+14f, %+14f", b[0], b[1], b[2], b[3]);

    auto latitude_sc  = llh.x() * constant::r2sc;
    auto longitude_sc = llh.y() * constant::r2sc;
    auto elevation_sc = elevation * constant::r2sc;

    auto gps_time     = ts::Gps(time);
    auto gps_time_sec = gps_time.timestamp().full_seconds();

    // Calculate the earth-centred angle (elevation in semicircles)
    auto theta = (0.0137 / (elevation_sc + 0.11)) - 0.022;
    VERBOSEF("theta: %+.14f", theta);

    // Compute the latitude of the Ionospheric Pierce Point
    auto ipp_latitude = latitude_sc + theta * std::cos(azimuth);
    VERBOSEF("ipp_latitude: %+.14f", ipp_latitude);
    if (ipp_latitude > 0.416) {
        ipp_latitude = 0.416;
    } else if (ipp_latitude < -0.416) {
        ipp_latitude = -0.416;
    }

    // Compute the longitude of the IPP
    auto ipp_longitude = longitude_sc + theta * std::sin(azimuth) / cos_sc(ipp_latitude);
    VERBOSEF("ipp_longitude: %+.14f", ipp_longitude);

    // Find the geomagnetic latitude of the IPP
    auto geo_latitude = ipp_latitude + 0.064 * cos_sc(ipp_longitude - 1.617);
    VERBOSEF("geo_latitude: %+.14f", geo_latitude);

    // Find the local time at the IPP
    auto ipp_local_time = 43200 * ipp_longitude + gps_time_sec;
    ipp_local_time -= std::floor(ipp_local_time / 86400.0) * 86400.0;
    VERBOSEF("ipp_local_time: %+.14f", ipp_local_time);

    // Compute the amplitude of the ionospheric delay
    auto amplitude = a[0] + geo_latitude * (a[1] + geo_latitude * (a[2] + geo_latitude * a[3]));
    VERBOSEF("amplitude: %+.14f", amplitude);
    if (amplitude < 0.0) {
        amplitude = 0.0;
    }

    // Compute the period of the ionospheric delay
    auto period = b[0] + geo_latitude * (b[1] + geo_latitude * (b[2] + geo_latitude * b[3]));
    VERBOSEF("period: %+.14f", period);
    if (period < 72000.0) {
        period = 72000.0;
    }

    // Compute the phase of the ionospheric delay
    auto phase = 2 * constant::pi * (ipp_local_time - 50400.0) / period;
    VERBOSEF("phase: %+.14f", phase);

    // Compute the slant factor
    auto slant_factor = 1.0 + 16.0 * std::pow(0.53 - elevation_sc, 3.0);
    VERBOSEF("slant factor: %+.14f", slant_factor);

    // Compute the ionospheric delay
    Scalar result = 5.0e-9;
    if (std::fabs(phase) <= 1.57) {
        auto phase_sq = phase * phase;
        result += amplitude * (1.0 + phase_sq * (-0.5 + phase_sq / 24.0));
    }

    auto delay = constant::c * slant_factor * result;
    VERBOSEF("delay: %+.14f", delay);

    return delay;
}

}  // namespace idokeido
