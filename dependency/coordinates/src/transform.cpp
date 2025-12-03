// Coordinate frame transformations
// PZ-90 parameters: GLONASS ICD
// WGS84/ITRF parameters: Various GNSS ICDs and IERS publications

#include "coordinates/transform.hpp"
#include "coordinates/ecef_llh.hpp"
#include "coordinates/frames.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(coord, transform);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(coord, transform)

namespace coordinates {

static Vector3d apply_helmert7_impl(Vector3d const& coords, Helmert7Params const& params) {
    VSCOPE_FUNCTION();
    TRACEF("params: tx=%.3f ty=%.3f tz=%.3f rx=%.6f ry=%.6f rz=%.6f s=%.6f", params.tx, params.ty,
           params.tz, params.rx, params.ry, params.rz, params.s);

    constexpr double MAS2RAD  = 4.84813681109536e-9;
    constexpr double MM2M     = 0.001;
    constexpr double PPB2UNIT = 1e-9;

    double rx = params.rx * MAS2RAD;
    double ry = params.ry * MAS2RAD;
    double rz = params.rz * MAS2RAD;
    double s  = 1.0 + params.s * PPB2UNIT;

    Vector3d result;
    result.x() = params.tx * MM2M + s * (coords.x() - rz * coords.y() + ry * coords.z());
    result.y() = params.ty * MM2M + s * (rz * coords.x() + coords.y() - rx * coords.z());
    result.z() = params.tz * MM2M + s * (-ry * coords.x() + rx * coords.y() + coords.z());

    TRACEF("result: x=%.3f y=%.3f z=%.3f", result.x(), result.y(), result.z());

    return result;
}

static Vector3d apply_helmert7_velocity_impl(Vector3d const& position, Vector3d const& velocity,
                                             Helmert7Params const& base,
                                             Helmert7Params const& rate) {
    constexpr double MAS2RAD  = 4.84813681109536e-9;
    constexpr double MM2M     = 0.001;
    constexpr double PPB2UNIT = 1e-9;

    double rx_base = base.rx * MAS2RAD;
    double ry_base = base.ry * MAS2RAD;
    double rz_base = base.rz * MAS2RAD;
    double s_base  = 1.0 + base.s * PPB2UNIT;

    double rx_rate = rate.rx * MAS2RAD;
    double ry_rate = rate.ry * MAS2RAD;
    double rz_rate = rate.rz * MAS2RAD;
    double s_rate  = rate.s * PPB2UNIT;

    Vector3d result;
    result.x() = s_base * (velocity.x() - rz_base * velocity.y() + ry_base * velocity.z()) +
                 rate.tx * MM2M +
                 s_rate * (position.x() - rz_base * position.y() + ry_base * position.z()) -
                 rz_rate * position.y() + ry_rate * position.z();
    result.y() = s_base * (rz_base * velocity.x() + velocity.y() - rx_base * velocity.z()) +
                 rate.ty * MM2M +
                 s_rate * (rz_base * position.x() + position.y() - rx_base * position.z()) +
                 rz_rate * position.x() - rx_rate * position.z();
    result.z() = s_base * (-ry_base * velocity.x() + rx_base * velocity.y() + velocity.z()) +
                 rate.tz * MM2M +
                 s_rate * (-ry_base * position.x() + rx_base * position.y() + position.z()) -
                 ry_rate * position.x() + rx_rate * position.y();

    return result;
}

static Helmert7Params transform_to_epoch(Helmert7Params const& base, Helmert7Params const& rate,
                                         double dt) {
    VSCOPE_FUNCTIONF("dt=%.3f years", dt);
    TRACEF("base: tx=%.3f ty=%.3f tz=%.3f rx=%.6f ry=%.6f rz=%.6f s=%.6f", base.tx, base.ty,
           base.tz, base.rx, base.ry, base.rz, base.s);
    TRACEF("rate: tx=%.3f ty=%.3f tz=%.3f rx=%.6f ry=%.6f rz=%.6f s=%.6f", rate.tx, rate.ty,
           rate.tz, rate.rx, rate.ry, rate.rz, rate.s);

    auto result = Helmert7Params{
        base.tx + rate.tx * dt, base.ty + rate.ty * dt, base.tz + rate.tz * dt,
        base.rx + rate.rx * dt, base.ry + rate.ry * dt, base.rz + rate.rz * dt,
        base.s + rate.s * dt,
    };

    TRACEF("result: tx=%.3f ty=%.3f tz=%.3f rx=%.6f ry=%.6f rz=%.6f s=%.6f", result.tx, result.ty,
           result.tz, result.rx, result.ry, result.rz, result.s);
    return result;
}

Helmert7Params Helmert7Params::inverse() const {
    constexpr double MAS2RAD  = 4.84813681109536e-9;
    constexpr double MM2M     = 0.001;
    constexpr double PPB2UNIT = 1e-9;

    double s = 1.0 + this->s * PPB2UNIT;

    double rx_rad = rx * MAS2RAD;
    double ry_rad = ry * MAS2RAD;
    double rz_rad = rz * MAS2RAD;

    Matrix3d R;
    R << 1.0, -rz_rad, ry_rad, rz_rad, 1.0, -rx_rad, -ry_rad, rx_rad, 1.0;

    Vector3d t(tx * MM2M, ty * MM2M, tz * MM2M);
    Vector3d inv_t = -R.transpose() * t / s;

    return Helmert7Params{inv_t.x() / MM2M, inv_t.y() / MM2M, inv_t.z() / MM2M, -rx, -ry, -rz,
                          -this->s / s};
}

Helmert7Params Helmert7Params::scale_std(double factor) const {
    return Helmert7Params{tx * factor, ty * factor, tz * factor, rx, ry, rz, s * factor};
}

Helmert7Params Helmert7Params::identity() {
    return Helmert7Params{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

Helmert7Params Helmert7Params::at_epoch(Helmert7Params const& base, Helmert7Params const& rate,
                                        double dt) {
    return transform_to_epoch(base, rate, dt);
}

Vector3d Helmert7Params::apply_position(Helmert7Params const& base, Vector3d const& position) {
    return apply_helmert7_impl(position, base);
}

Vector3d Helmert7Params::apply_velocity(Helmert7Params const& base, Helmert7Params const& rate,
                                        Vector3d const& position, Vector3d const& velocity) {
    return apply_helmert7_velocity_impl(position, velocity, base, rate);
}

}  // namespace coordinates
