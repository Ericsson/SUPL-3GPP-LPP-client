#include <cmath>
#include <coordinates/ecef_aer.hpp>
#include <coordinates/ecef_enu.hpp>

namespace coordinates {

Vector3d ecef_to_aer_impl(Vector3d const& ecef, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid) {
    auto enu = ecef_to_enu_impl(ecef, origin_llh, ellipsoid);

    auto range     = enu.norm();
    auto azimuth   = std::atan2(enu.x(), enu.y());
    auto elevation = std::asin(enu.z() / range);

    if (azimuth < 0) azimuth += 2 * M_PI;

    return Vector3d(azimuth, elevation, range);
}

Vector3d aer_to_ecef_impl(Vector3d const& aer, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid) {
    auto az = aer.x();
    auto el = aer.y();
    auto r  = aer.z();

    auto cos_el = std::cos(el);
    auto enu    = Vector3d(r * cos_el * std::sin(az), r * cos_el * std::cos(az), r * std::sin(el));

    return enu_to_ecef_impl(enu, origin_llh, ellipsoid);
}

}  // namespace coordinates
