#pragma once
#include <core/core.hpp>

namespace generator {
namespace tokoro {

struct GlobalMappingFunction {
    double hydrostatic;
    double wet;
};

// Global Mapping Function (Boehm et al. 2006), derived from the IERS Conventions routine GMF.
//  mjd:                     modified julian date
//  latitude, longitude:     geodetic position [rad], east longitude
//  height:                  height [m]
//  zenith_distance:         [rad]
//  apply_height_correction: Niell (1996) height correction of the hydrostatic mapping function
NODISCARD GlobalMappingFunction global_mapping_function(double mjd, double latitude,
                                                        double longitude, double height,
                                                        double zenith_distance,
                                                        bool   apply_height_correction) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator
