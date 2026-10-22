#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "models/astronomical_arguments.hpp"

namespace generator {
namespace tokoro {

struct EarthSolidTides {
    double displacement;
    Float3 displacement_vector;
    bool   valid;
};

/// Solid Earth tide displacement of a station [m, ECEF], IERS Conventions 2010 (TN 36) section
/// 7.1.1.1: Step 1 (degree 2 with latitude dependent Love numbers, degree 3 for the Moon, l(1) and
/// out-of-phase terms) plus Step 2 (Tables 7.3a and 7.3b). The result contains the permanent tide
/// ("conventional tide free" convention). `gmst` is the Greenwich mean sidereal time [rad] and
/// `args` the fundamental arguments (l, l', F, D, Omega) at the same epoch.
Float3 solid_earth_tide_displacement(Float3 station_ecef, Float3 sun_ecef, Float3 moon_ecef,
                                     double gmst, AstronomicalArguments const& args) NOEXCEPT;

struct SatelliteState;
EarthSolidTides model_earth_solid_tides(ts::Tai const& time, SatelliteState const& satellite,
                                        Float3 ground_position_ecef);

}  // namespace tokoro
}  // namespace generator
