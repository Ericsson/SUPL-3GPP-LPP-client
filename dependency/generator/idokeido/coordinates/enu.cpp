#include "enu.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, enu);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, enu)

namespace idokeido {

void enu_basis_from_llh(const Vector3& llh, Vector3& east, Vector3& north, Vector3& up) NOEXCEPT
{
    VSCOPE_FUNCTIONF("%+.6f, %+.6f, %+.2f", llh.x(), llh.y(), llh.z());

    auto lat = llh.x();
    auto lon = llh.y();

    east.x() = -sin(lon);
    east.y() = cos(lon);
    east.z() = 0.0;

    north.x() = -sin(lat) * cos(lon);
    north.y() = -sin(lat) * sin(lon);
    north.z() = cos(lat);

    up.x() = cos(lat) * cos(lon);
    up.y() = cos(lat) * sin(lon);
    up.z() = sin(lat);

    VERBOSEF("east:  (%+.4f, %+.4f, %+.4f)", east.x(), east.y(), east.z());
    VERBOSEF("north: (%+.4f, %+.4f, %+.4f)", north.x(), north.y(), north.z());
    VERBOSEF("up:    (%+.4f, %+.4f, %+.4f)", up.x(), up.y(), up.z());
}

void enu_basis_from_xyz(const Vector3& ecef, Vector3& east, Vector3& north, Vector3& up) NOEXCEPT
{
    VSCOPE_FUNCTIONF("%+.4f, %+.4f, %+.4f", ecef.x(), ecef.y(), ecef.z());

    auto r = ecef.norm();
    auto lat = asin(ecef.z() / r);
    auto lon = atan2(ecef.y(), ecef.x());

    enu_basis_from_llh(Vector3(lat, lon, r), east, north, up);
}

Vector3 ecef_to_enu_at_llh(const Vector3& llh, const Vector3& ecef_vector)
{
    Vector3 east, north, up;
    enu_basis_from_llh(llh, east, north, up);

    return east.dot(ecef_vector) * east +
           north.dot(ecef_vector) * north +
           up.dot(ecef_vector) * up;
}

} // namespace tokoro

