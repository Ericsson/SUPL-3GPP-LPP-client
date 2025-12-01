#pragma once
#include <Eigen/Dense>
#include "ellipsoid.hpp"

namespace coordinates {

using Eigen::Vector3d;

template <typename Frame = NullReferenceFrame>
struct Ecef {
    Vector3d value;

    double x() const { return value.x(); }
    double y() const { return value.y(); }
    double z() const { return value.z(); }

    double& x() { return value.x(); }
    double& y() { return value.y(); }
    double& z() { return value.z(); }
};

template <typename Frame>
Ecef<Frame> operator+(Ecef<Frame> const& a, Ecef<Frame> const& b) {
    return Ecef<Frame>{a.value + b.value};
}

template <typename Frame>
Ecef<Frame> operator-(Ecef<Frame> const& a, Ecef<Frame> const& b) {
    return Ecef<Frame>{a.value - b.value};
}

template <typename Frame>
Ecef<Frame> operator*(double scalar, Ecef<Frame> const& v) {
    return Ecef<Frame>{scalar * v.value};
}

template <typename Frame>
double distance(Ecef<Frame> const& a, Ecef<Frame> const& b) {
    return (a.value - b.value).norm();
}

template <typename Frame>
double dot(Ecef<Frame> const& a, Ecef<Frame> const& b) {
    return a.value.dot(b.value);
}

template <typename Frame>
Ecef<Frame> cross(Ecef<Frame> const& a, Ecef<Frame> const& b) {
    return Ecef<Frame>{a.value.cross(b.value)};
}

}  // namespace coordinates
