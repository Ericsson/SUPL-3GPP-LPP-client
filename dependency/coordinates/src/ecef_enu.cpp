#include <Eigen/Dense>
#include <cmath>
#include <coordinates/ecef_enu.hpp>
#include <coordinates/ecef_llh.hpp>

namespace coordinates {

Eigen::Matrix3d enu_rotation_matrix(double lat, double lon) {
    auto sin_lat = std::sin(lat);
    auto cos_lat = std::cos(lat);
    auto sin_lon = std::sin(lon);
    auto cos_lon = std::cos(lon);

    Eigen::Matrix3d R;
    R << -sin_lon, cos_lon, 0.0, -sin_lat * cos_lon, -sin_lat * sin_lon, cos_lat, cos_lat * cos_lon,
        cos_lat * sin_lon, sin_lat;

    return R;
}

Vector3d ecef_to_enu_impl(Vector3d const& ecef, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid) {
    auto origin_ecef = llh_to_ecef_impl(origin_llh, ellipsoid);
    auto delta_ecef  = ecef - origin_ecef;
    return enu_rotation_matrix(origin_llh.x(), origin_llh.y()) * delta_ecef;
}

Vector3d enu_to_ecef_impl(Vector3d const& enu, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid) {
    auto origin_ecef = llh_to_ecef_impl(origin_llh, ellipsoid);
    return origin_ecef + enu_rotation_matrix(origin_llh.x(), origin_llh.y()).transpose() * enu;
}

}  // namespace coordinates
