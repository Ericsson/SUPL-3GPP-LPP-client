#pragma once

namespace coordinates {

struct NullReferenceFrame {};

struct Ellipsoid {
    double a;
    double f;
    double b;
    double e2;

    static constexpr Ellipsoid from_a_f(double a, double f) {
        return Ellipsoid{a, f, a * (1.0 - f), 2.0 * f - f * f};
    }
};

namespace ellipsoids {
static constexpr Ellipsoid Wgs84 = Ellipsoid::from_a_f(6378137.0, 1.0 / 298.257223563);
static constexpr Ellipsoid Grs80 = Ellipsoid::from_a_f(6378137.0, 1.0 / 298.257222101);
static constexpr Ellipsoid Pz90  = Ellipsoid::from_a_f(6378136.0, 1.0 / 298.25784);
}  // namespace ellipsoids

}  // namespace coordinates
