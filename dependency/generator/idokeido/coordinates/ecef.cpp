#include "ecef.hpp"

#include <math.h>

namespace idokeido {

Vector3 ecef_to_llh(Vector3 ecef, ReferenceEllipsoid const& ellipsoid) {
    auto x = ecef.x();
    auto y = ecef.y();
    auto z = ecef.z();

    auto a  = ellipsoid.semi_major_axis;
    auto e2 = ellipsoid.eccentricity_sq;

    auto lon = std::atan2(y, x);

    auto p   = std::sqrt(x * x + y * y);
    auto phi = std::atan2(z, p * (1.0 - e2));

    auto tolerance = 1e-12;
    for (;;) {
        auto sin_phi = std::sin(phi);
        auto N       = a / std::sqrt(1.0 - e2 * sin_phi * sin_phi);
        auto phi_n   = std::atan2(z + N * e2 * sin_phi, p);
        if (std::abs(phi_n - phi) < tolerance) {
            phi = phi_n;

            auto h = p / std::cos(phi) - N;
            return Vector3{phi, lon, h};
        }

        phi = phi_n;
    }
}

Vector3 llh_to_ecef(Vector3 llh, ReferenceEllipsoid const& ellipsoid) {
    auto lat = llh.x();
    auto lon = llh.y();
    auto h   = llh.z();

    auto d = ellipsoid.eccentricity * std::sin(lat);
    auto N = ellipsoid.semi_major_axis / std::sqrt(1.0 - d * d);

    auto x = (N + h) * std::cos(lat) * std::cos(lon);
    auto y = (N + h) * std::cos(lat) * std::sin(lon);
    auto z = (N * (1.0 - ellipsoid.eccentricity_sq) + h) * std::sin(lat);
    return Vector3{x, y, z};
}

}  // namespace idokeido
