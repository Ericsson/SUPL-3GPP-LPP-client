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

}  // namespace coordinates
