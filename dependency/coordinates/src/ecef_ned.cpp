#include <coordinates/ecef_enu.hpp>
#include <coordinates/ecef_ned.hpp>

namespace coordinates {

Vector3d ecef_to_ned_impl(Vector3d const& ecef, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid) {
    auto enu = ecef_to_enu_impl(ecef, origin_llh, ellipsoid);
    return Vector3d(enu.y(), enu.x(), -enu.z());
}

Vector3d ned_to_ecef_impl(Vector3d const& ned, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid) {
    auto enu = Vector3d(ned.y(), ned.x(), -ned.z());
    return enu_to_ecef_impl(enu, origin_llh, ellipsoid);
}

}  // namespace coordinates
