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

    double e() const { return value.x(); }
    double n() const { return value.y(); }
    double u() const { return value.z(); }

    double& e() { return value.x(); }
    double& n() { return value.y(); }
    double& u() { return value.z(); }

    Enu<NullReferenceFrame> to_any() const { return Enu<NullReferenceFrame>{value}; }

    static Enu from_any(Enu<NullReferenceFrame> const& other) { return Enu{other.value}; }
};

}  // namespace coordinates
