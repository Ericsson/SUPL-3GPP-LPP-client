#include "wgs84.hpp"
#include "constant.hpp"

#include <math.h>

namespace generator {
namespace tokoro {

Wgs84Position ecef_to_wgs84(Float3 ecef) NOEXCEPT {
    double e2   = constant::FE_WGS84 * (2.0 - constant::FE_WGS84);
    double r2   = std::pow(ecef.x, 2) + std::pow(ecef.y, 2);
    double z    = 0.0;
    double zk   = 0.0;
    double v    = constant::RE_WGS84;
    double sinp = 0.0;

    for (z = ecef.z, zk = 0.0; std::fabs(z - zk) >= 1E-4;) {
        zk   = z;
        sinp = z / std::sqrt(r2 + z * z);
        v    = constant::RE_WGS84 / std::sqrt(1.0 - e2 * sinp * sinp);
        z    = ecef.z + v * e2 * sinp;
    }
    auto latitude  = r2 > 1E-12 ?
                         std::atan(z / std::sqrt(r2)) :
                         (ecef.z > 0.0 ? constant::PI_WGS84 / 2.0 : -constant::PI_WGS84 / 2.0);
    auto longitude = r2 > 1E-12 ? std::atan2(ecef.y, ecef.x) : 0.0;
    auto altitude  = std::sqrt(r2 + z * z) - v;
    Wgs84Position result{};
    result.x  = latitude * constant::RAD2DEG;
    result.y = longitude * constant::RAD2DEG;
    result.z  = altitude;
    return result;
}
}  // namespace tokoro
}  // namespace generator
