#pragma once
#include <Eigen/Dense>

namespace coordinates {

using Eigen::Matrix3d;
using Eigen::Vector3d;

struct Helmert7Params {
    double tx;  // millimeters
    double ty;  // millimeters
    double tz;  // millimeters
    double rx;  // milli-arc-seconds
    double ry;  // milli-arc-seconds
    double rz;  // milli-arc-seconds
    double s;   // parts per billion

    Helmert7Params fast_inverse() const { return Helmert7Params{-tx, -ty, -tz, -rx, -ry, -rz, -s}; }

    Helmert7Params inverse() const;
    Helmert7Params scale_std(double factor) const;

    static Helmert7Params identity();
    static Helmert7Params at_epoch(Helmert7Params const& base, Helmert7Params const& rate,
                                   double dt);
    static Vector3d       apply_position(Helmert7Params const& base, Vector3d const& position);
    static Vector3d       apply_velocity(Helmert7Params const& base, Helmert7Params const& rate,
                                         Vector3d const& position, Vector3d const& velocity);

    Vector3d apply_position(Vector3d const& position) const {
        return Helmert7Params::apply_position(*this, position);
    }
};

struct TimeDependentHelmertParams {
    Helmert7Params base;
    Helmert7Params rate;
    double         reference_epoch;
};

}  // namespace coordinates
