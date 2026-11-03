#pragma once
#include <core/core.hpp>

namespace generator {
namespace tokoro {

struct ViennaMapping {
    double hydrostatic;
    double wet;
};

// Herring continued fraction form of a mapping function with the coefficients (a, b, c)
NODISCARD double herring_mapping(double elevation, double a, double b, double c) NOEXCEPT;

// VMF1 mapping function (Boehm et al. 2006) with the hydrostatic and wet coefficients `ah` and `aw`
// (e.g. from a GPT2 grid file).
//  mjd:                     modified julian date
//  latitude:                geodetic latitude [rad]
//  height:                  ellipsoidal height [m]
//  elevation:               [rad]
//  apply_height_correction: Niell (1996) height correction of the hydrostatic mapping function
NODISCARD ViennaMapping vmf1_mapping(double ah, double aw, double mjd, double latitude,
                                     double height, double elevation,
                                     bool apply_height_correction) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator
