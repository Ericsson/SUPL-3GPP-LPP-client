#pragma once
#include "idokeido.hpp"

namespace idokeido {

struct LookAngles {
    Scalar azimuth;
    Scalar elevation;
    Scalar nadir;
};

bool compute_look_angles(Vector3 const& ground_ecef, Vector3 const& enu,
                         Vector3 const& satellite_ecef, LookAngles& look_angles) NOEXCEPT;

}  // namespace idokeido
