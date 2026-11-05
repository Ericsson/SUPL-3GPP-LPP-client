#pragma once
#include <core/core.hpp>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>

namespace format {
namespace rtcm {

class Rtcm1019;
class Rtcm1042;
class Rtcm1046;

NODISCARD ephemeris::GpsEphemeris to_ephemeris(Rtcm1019 const& rtcm) NOEXCEPT;
NODISCARD ephemeris::BdsEphemeris to_ephemeris(Rtcm1042 const& rtcm) NOEXCEPT;
NODISCARD ephemeris::GalEphemeris to_ephemeris(Rtcm1046 const& rtcm) NOEXCEPT;

}  // namespace rtcm
}  // namespace format
