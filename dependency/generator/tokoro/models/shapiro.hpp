#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>

namespace generator {
namespace tokoro {

struct Shapiro {
    double correction;  // meters
    bool   valid;
};

struct SatelliteState;
Shapiro model_shapiro(SatelliteState const& satellite, Float3 ground_position);

}  // namespace tokoro
}  // namespace generator
