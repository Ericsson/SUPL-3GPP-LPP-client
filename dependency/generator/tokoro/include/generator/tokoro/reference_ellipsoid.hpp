#pragma once
#include <maths/float3.hpp>

#include <math.h>

namespace generator {
namespace tokoro {

struct ReferenceEllipsoid {
    double semi_major_axis;
    double semi_minor_axis;
    double flattening;
    double eccentricity_sq;
    double eccentricity;

    static CONSTEXPR ReferenceEllipsoid create(double semi_major_axis, double flattening, double eccentricity_sq) {
        return {
            /* semi_major_axis */ semi_major_axis,
            /* semi_minor_axis */ semi_major_axis * (1.0 - flattening),
            /* flattening */ flattening,
            /* eccentricity_sq */ eccentricity_sq,
            /* eccentricity */ std::sqrt(eccentricity_sq),
        };
    }

    static CONSTEXPR ReferenceEllipsoid create(double semi_major_axis, double flattening) {
        return create(semi_major_axis, flattening, 2.0 * flattening - flattening * flattening);
    }
};

extern ReferenceEllipsoid const WGS84;
extern ReferenceEllipsoid const GRS80;
extern ReferenceEllipsoid const PZ90;
extern ReferenceEllipsoid const PZ90_11;

}  // namespace tokoro
}  // namespace generator
