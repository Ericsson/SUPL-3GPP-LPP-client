#pragma once
#include <core/core.hpp>

namespace generator {
namespace tokoro {
namespace constant {
CONSTEXPR static double SPEED_OF_LIGHT = 2.99792458e8;

// WGS84 GM — used for Shapiro relativistic delay
CONSTEXPR static double WGS84_EARTH_GRAVITATIONAL_CONSTANT = 3.986004418e14;

// IERS 2010 GM — used for earth solid tides (differs from WGS84 by 3e6 m³/s²)
CONSTEXPR static double GME                         = 3.986004415e14;
CONSTEXPR static double SUN_GRAVITATIONAL_CONSTANT  = 1.327124E+20;
CONSTEXPR static double MOON_GRAVITATIONAL_CONSTANT = 4.902801E+12;

// IS-GPS-200 / Galileo / QZSS ICD value — used for Sagnac correction and phase windup
CONSTEXPR static double EARTH_ANGULAR_VELOCITY = 7.2921151467E-5;

CONSTEXPR static double PI         = 3.1415926535897932;
CONSTEXPR static double DEG2RAD    = (PI / 180.0);
CONSTEXPR static double RAD2DEG    = (180.0 / PI);
CONSTEXPR static double ARCSEC2RAD = (DEG2RAD / 3600.0);

CONSTEXPR static double AU = 149597870691.0;

// Earth semimajor axis (WGS84) (m)
CONSTEXPR static double RE_WGS84 = 6378137.0;
// Earth flattening (WGS84)
CONSTEXPR static double FE_WGS84 = (1.0 / 298.257223563);
// PI constant (WGS84)
CONSTEXPR static double PI_WGS84 = 3.1415926535897932;

}  // namespace constant
}  // namespace tokoro
}  // namespace generator
