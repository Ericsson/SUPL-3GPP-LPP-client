#pragma once
#include "ecef.hpp"

namespace coordinates {

template <typename Frame = NullReferenceFrame>
struct Eci {
    Vector3d value;

    double x() const { return value.x(); }
    double y() const { return value.y(); }
    double z() const { return value.z(); }

    double& x() { return value.x(); }
    double& y() { return value.y(); }
    double& z() { return value.z(); }
};

}  // namespace coordinates
