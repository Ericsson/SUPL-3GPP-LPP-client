#pragma once
#include <core/core.hpp>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/glo.hpp>
#include <ephemeris/gps.hpp>
#include <ephemeris/qzs.hpp>
#include <time/tai.hpp>

namespace format {
namespace rtcm {

class Rtcm1019;
class Rtcm1042;
class Rtcm1046;
class Rtcm1020;
class Rtcm1044;

NODISCARD ephemeris::GpsEphemeris to_ephemeris(Rtcm1019 const& rtcm) NOEXCEPT;
NODISCARD ephemeris::BdsEphemeris to_ephemeris(Rtcm1042 const& rtcm) NOEXCEPT;
NODISCARD ephemeris::GalEphemeris to_ephemeris(Rtcm1046 const& rtcm) NOEXCEPT;
NODISCARD ephemeris::QzsEphemeris to_ephemeris(Rtcm1044 const& rtcm) NOEXCEPT;
// `now` resolves the day of tb when the message has no day number (NT/N4)
NODISCARD ephemeris::GloEphemeris to_ephemeris(Rtcm1020 const& rtcm, ts::Tai const& now) NOEXCEPT;

}  // namespace rtcm
}  // namespace format
