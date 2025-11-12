#pragma once
#include "idokeido.hpp"

#include <cmath>

namespace idokeido {

struct ReferenceEllipsoid {
    Scalar semi_major_axis;
    Scalar semi_minor_axis;
    Scalar flattening;
    Scalar eccentricity_sq;
    Scalar eccentricity;

    static CONSTEXPR_CXX20 ReferenceEllipsoid create(Scalar semi_major_axis, Scalar flattening,
                                                     Scalar eccentricity_sq) {
        return {
            /* semi_major_axis */ semi_major_axis,
            /* semi_minor_axis */ semi_major_axis * (1.0 - flattening),
            /* flattening */ flattening,
            /* eccentricity_sq */ eccentricity_sq,
            /* eccentricity */ std::sqrt(eccentricity_sq),
        };
    }

    static CONSTEXPR_CXX20 ReferenceEllipsoid create(Scalar semi_major_axis, Scalar flattening) {
        return create(semi_major_axis, flattening, 2.0 * flattening - flattening * flattening);
    }
};

namespace ellipsoid {
extern ReferenceEllipsoid const WGS84;
extern ReferenceEllipsoid const GRS80;
extern ReferenceEllipsoid const PZ90;
extern ReferenceEllipsoid const PZ90_11;
}  // namespace ellipsoid

}  // namespace idokeido
