#include "constant.hpp"
#include "coordinate.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, enu);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, enu)

namespace generator {
namespace tokoro {

void enu_basis_from_llh(Float3 llh, Float3& east, Float3& north, Float3& up) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.6f, %+.6f, %+.2f", llh.x, llh.y, llh.z);

    auto lat = llh.x;
    auto lon = llh.y;

    auto cos_lat = cos(lat);
    auto sin_lat = sin(lat);
    auto cos_lon = cos(lon);
    auto sin_lon = sin(lon);

    east.x = -sin_lon;
    east.y = cos_lon;
    east.z = 0.0;

    north.x = -sin_lat * cos_lon;
    north.y = -sin_lat * sin_lon;
    north.z = cos_lat;

    up.x = cos_lat * cos_lon;
    up.y = cos_lat * sin_lon;
    up.z = sin_lat;

    VERBOSEF("east:  (%+.4f, %+.4f, %+.4f)", east.x, east.y, east.z);
    VERBOSEF("north: (%+.4f, %+.4f, %+.4f)", north.x, north.y, north.z);
    VERBOSEF("up:    (%+.4f, %+.4f, %+.4f)", up.x, up.y, up.z);
}

void enu_basis_from_xyz(Float3 ecef, Float3& east, Float3& north, Float3& up) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.4f, %+.4f, %+.4f", ecef.x, ecef.y, ecef.z);

    // Calculate geodetic latitude and longitude assuming a spherical Earth. If you need better
    // accuracy, use ecef_to_llh() instead first with the appropriate ellipsoid and then convert the
    // result to ENU basis with enu_basis_from_llh().
    auto r   = ecef.length();
    auto lat = asin(ecef.z / r);
    auto lon = atan2(ecef.y, ecef.x);
    enu_basis_from_llh(Float3{lat, lon, r}, east, north, up);
}

Float3 ecef_to_enu_at_llh(Float3 llh, Float3 ecef_vector) {
    Float3 east, north, up;
    enu_basis_from_llh(llh, east, north, up);
    return Float3{
        dot_product(east, ecef_vector),
        dot_product(north, ecef_vector),
        dot_product(up, ecef_vector),
    };
}

}  // namespace tokoro
}  // namespace generator
