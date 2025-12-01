#include <cmath>
#include <coordinates/ecef_llh.hpp>

namespace coordinates {

Vector3d ecef_to_llh_impl(Vector3d const& ecef, Ellipsoid const& e) {
    auto x = ecef.x();
    auto y = ecef.y();
    auto z = ecef.z();

    auto lon = std::atan2(y, x);
    auto p   = ecef.head<2>().norm();
    auto lat = std::atan2(z, p * (1.0 - e.e2));

    for (int i = 0; i < 10; i++) {
        auto sin_lat = std::sin(lat);
        auto N       = e.a / std::sqrt(1.0 - e.e2 * sin_lat * sin_lat);
        auto lat_new = std::atan2(z + N * e.e2 * sin_lat, p);
        if (std::abs(lat_new - lat) < 1e-12) {
            lat = lat_new;
            break;
        }
        lat = lat_new;
    }

    auto sin_lat = std::sin(lat);
    auto cos_lat = std::cos(lat);
    auto N       = e.a / std::sqrt(1.0 - e.e2 * sin_lat * sin_lat);

    double h;
    if (std::abs(cos_lat) > 1e-10) {
        h = p / cos_lat - N;
    } else {
        h = std::abs(z) - e.b;
    }

    return Vector3d(lat, lon, h);
}

Vector3d llh_to_ecef_impl(Vector3d const& llh, Ellipsoid const& e) {
    auto lat = llh.x();
    auto lon = llh.y();
    auto h   = llh.z();

    auto sin_lat = std::sin(lat);
    auto cos_lat = std::cos(lat);
    auto sin_lon = std::sin(lon);
    auto cos_lon = std::cos(lon);

    auto N = e.a / std::sqrt(1.0 - e.e2 * sin_lat * sin_lat);

    auto x = (N + h) * cos_lat * cos_lon;
    auto y = (N + h) * cos_lat * sin_lon;
    auto z = (N * (1.0 - e.e2) + h) * sin_lat;

    return Vector3d(x, y, z);
}

}  // namespace coordinates
