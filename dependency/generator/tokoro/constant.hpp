#pragma once
#include <core/core.hpp>

namespace constant {
CONSTEXPR static double SPEED_OF_LIGHT = 2.99792458e8;

CONSTEXPR static double PI      = 3.1415926535897932;
CONSTEXPR static double DEG2RAD = (PI / 180.0);
CONSTEXPR static double RAD2DEG = (180.0 / PI);

// Earth semimajor axis (WGS84) (m)
CONSTEXPR static double RE_WGS84 = 6378137.0;
// Earth flattening (WGS84)
CONSTEXPR static double FE_WGS84 = (1.0 / 298.257223563);
// PI constant (WGS84)
CONSTEXPR static double PI_WGS84   = 3.1415926535897932;

}  // namespace constant
