#include "reference_ellipsoid.hpp"
#include <cmath>

namespace generator {
namespace tokoro {

ReferenceEllipsoid ReferenceEllipsoid::create(double semi_major_axis, double flattening,
                                              double eccentricity_sq) {
    return {
        /* semi_major_axis */ semi_major_axis,
        /* semi_minor_axis */ semi_major_axis * (1.0 - flattening),
        /* flattening */ flattening,
        /* eccentricity_sq */ eccentricity_sq,
        /* eccentricity */ std::sqrt(eccentricity_sq),
    };
}

ReferenceEllipsoid ReferenceEllipsoid::create(double semi_major_axis, double flattening) {
    return create(semi_major_axis, flattening, 2.0 * flattening - flattening * flattening);
}

namespace ellipsoid {
ReferenceEllipsoid const WGS84   = ReferenceEllipsoid::create(6378137.0, 1.0 / 298.257223563);
ReferenceEllipsoid const GRS80   = ReferenceEllipsoid::create(6378137.0, 1.0 / 298.257222101);
ReferenceEllipsoid const PZ90    = ReferenceEllipsoid::create(6378136.0, 1.0 / 298.25);
ReferenceEllipsoid const PZ90_11 = ReferenceEllipsoid::create(6378136.0, 1.0 / 298.257839303);
}  // namespace ellipsoid
}  // namespace tokoro
}  // namespace generator
