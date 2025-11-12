#pragma once
#include <maths/float3.hpp>

namespace generator {
namespace tokoro {

struct ReferenceEllipsoid {
    double semi_major_axis;
    double semi_minor_axis;
    double flattening;
    double eccentricity_sq;
    double eccentricity;

    static ReferenceEllipsoid create(double semi_major_axis, double flattening,
                                     double eccentricity_sq);

    static ReferenceEllipsoid create(double semi_major_axis, double flattening);
};

namespace ellipsoid {
extern ReferenceEllipsoid const WGS84;
extern ReferenceEllipsoid const GRS80;
extern ReferenceEllipsoid const PZ90;
extern ReferenceEllipsoid const PZ90_11;
}  // namespace ellipsoid

}  // namespace tokoro
}  // namespace generator
