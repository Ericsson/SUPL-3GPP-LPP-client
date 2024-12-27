#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct EarthSolidTides {
    double displacement;
    Float3 displacement_vector;
    bool   valid;
};

struct SatelliteState;
EarthSolidTides model_earth_solid_tides(ts::Tai const& time, SatelliteState const& satellite,
                                        Float3 ground_position_ecef, Float3 ground_position_llh);

}  // namespace tokoro
}  // namespace generator
