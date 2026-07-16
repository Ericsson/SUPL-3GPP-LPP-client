#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct PhaseWindup {
    double correction_sun;       // cycles
    double correction_velocity;  // cycles
    double correction_angle;     // cycles
    bool   valid;

    // Satellite antenna (yaw-steering) body basis in ECEF, for PCO projection.
    Float3 sat_x;
    Float3 sat_y;
    Float3 sat_z;
};

struct SatelliteState;
PhaseWindup model_phase_windup(ts::Tai const& time, SatelliteState const& satellite,
                               Float3 ground_position, Float3 ground_position_llh,
                               PhaseWindup const& previous_windup);

}  // namespace tokoro
}  // namespace generator
