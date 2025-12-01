#pragma once
#include "ecef.hpp"

namespace coordinates {

template <typename Frame = NullReferenceFrame>
struct Ned {
    Vector3d value;

    double north() const { return value.x(); }
    double east() const { return value.y(); }
    double down() const { return value.z(); }

    double& north() { return value.x(); }
    double& east() { return value.y(); }
    double& down() { return value.z(); }
};

}  // namespace coordinates
