#pragma once
#include "ecef.hpp"

namespace coordinates {

template <typename Frame = NullReferenceFrame>
struct Enu {
    Vector3d value;

    double east() const { return value.x(); }
    double north() const { return value.y(); }
    double up() const { return value.z(); }

    double& east() { return value.x(); }
    double& north() { return value.y(); }
    double& up() { return value.z(); }
};

}  // namespace coordinates
