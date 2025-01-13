#include "phase_windup.hpp"
#include "coordinates/enu.hpp"
#include "models/helper.hpp"
#include "models/sun_moon.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "tokoro/phw"

namespace generator {
namespace tokoro {

static bool compute_satellite_antenna_basis_sun(Float3 satellite_position, Float3 sun_position,
                                                Float3& x, Float3& y, Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("(%+.4f, %+.4f, %+.4f), (%+.4f, %+.4f, %+.4f)", satellite_position.x,
                     satellite_position.y, satellite_position.z, sun_position.x, sun_position.y,
                     sun_position.z);

    // unit vector of satellite antenna (pointing at the center of the Earth)
    auto sz = -1.0 * satellite_position;
    if (!sz.normalize()) return false;

    // unit vector from sun to satellite
    auto e = sun_position - satellite_position;
    if (!e.normalize()) return false;
    VERBOSEF("e:   %+.4f, %+.4f, %+.4f", e.x, e.y, e.z);

    // the satellite antenna basis
    auto sy = cross_product(sz, e);
    if (!sy.normalize()) return false;
    auto sx = cross_product(sy, sz);
    if (!sx.normalize()) return false;

    VERBOSEF("sx:  %+.4f, %+.4f, %+.4f", sx.x, sx.y, sx.z);
    VERBOSEF("sy:  %+.4f, %+.4f, %+.4f", sy.x, sy.y, sy.z);
    VERBOSEF("sz:  %+.4f, %+.4f, %+.4f", sz.x, sz.y, sz.z);

    x = sx;
    y = sy;
    z = sz;
    return true;
}

static bool compute_satellite_antenna_basis_velocity(Float3 ground_position,
                                                     Float3 satellite_position,
                                                     Float3 satellite_velocity, Float3& x,
                                                     Float3& y, Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("(%+.4f, %+.4f, %+.4f), (%+.4f, %+.4f, %+.4f)", satellite_position.x,
                     satellite_position.y, satellite_position.z, satellite_velocity.x,
                     satellite_velocity.y, satellite_velocity.z);

    // unit vector of satellite antenna (pointing at the center of the Earth)
    auto sz = satellite_position;
    if (!sz.normalize()) return false;
    sz = -1.0 * sz;

    // unit vector from ground to satellite
    auto k = ground_position - satellite_position;
    if (!k.normalize()) return false;
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    // unit vector of velocity to satellite position considering Earth rotation
    auto e = satellite_velocity + constant::EARTH_ANGULAR_VELOCITY *
                                      Float3{-satellite_position.x, satellite_position.y, 0.0};
    if (!e.normalize()) return false;
    VERBOSEF("e:   %+.4f, %+.4f, %+.4f", e.x, e.y, e.z);

    // the satellite antenna basis
    auto sy = cross_product(sz, e);
    if (!sy.normalize()) return false;
    auto sx = cross_product(sy, sz);
    if (!sx.normalize()) return false;

    VERBOSEF("sx:  %+.4f, %+.4f, %+.4f", sx.x, sx.y, sx.z);
    VERBOSEF("sy:  %+.4f, %+.4f, %+.4f", sy.x, sy.y, sy.z);
    VERBOSEF("sz:  %+.4f, %+.4f, %+.4f", sz.x, sz.y, sz.z);

    x = sx;
    y = sy;
    z = sz;
    return true;
}

// https://github.com/Azurehappen/Virtual-Network-DGNSS-Project/blob/5d0904aabab5880d807e92460ac540d456819329/VN_DGNSS_Server/rtklib/phase_windup.cpp#L308
static double satellite_yaw_angle(double beta, double mu) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.4f, %+.4f", beta, mu);
    if (fabs(beta) < 1.0e-12 && fabs(mu) < 1.0e-12) {
        return constant::PI;
    }

    return atan2(-tan(beta), sin(mu)) + constant::PI;
}

// https://github.com/Azurehappen/Virtual-Network-DGNSS-Project/blob/5d0904aabab5880d807e92460ac540d456819329/VN_DGNSS_Server/rtklib/phase_windup.cpp#L321
static bool compute_satellite_antenna_basis_yaw(ts::Tai const& emission_time,
                                                Float3 ground_position, Float3 satellite_position,
                                                Float3 satellite_velocity, Float3& x, Float3& y,
                                                Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, (%+.4f, %+.4f, %+.4f), (%+.4f, %+.4f, %+.4f)",
                     emission_time.rtklib_time_string().c_str(), ground_position.x,
                     ground_position.y, ground_position.z, satellite_position.x,
                     satellite_position.y, satellite_position.z);

    auto sm = sun_and_moon_position_ecef(emission_time);

    auto satellite_unit = satellite_position;
    if (!satellite_unit.normalize()) return false;
    VERBOSEF("stu: %+.4f, %+.4f, %+.4f", satellite_unit.x, satellite_unit.y, satellite_unit.z);

    auto sun_unit = sm.sun;
    if (!sun_unit.normalize()) return false;
    VERBOSEF("suu: %+.4f, %+.4f, %+.4f", sun_unit.x, sun_unit.y, sun_unit.z);

    auto velocity =
        satellite_velocity +
        constant::EARTH_ANGULAR_VELOCITY * Float3{-satellite_position.x, satellite_position.y, 0.0};

    // normal of the plane formed by the satellite position (from the earth center) and the velocity
    auto n = cross_product(satellite_position, velocity);
    auto p = cross_product(sm.sun, n);
    if (!n.normalize()) return false;
    if (!p.normalize()) return false;

    VERBOSEF("n:   %+.4f, %+.4f, %+.4f", n.x, n.y, n.z);
    VERBOSEF("p:   %+.4f, %+.4f, %+.4f", p.x, p.y, p.z);

    auto b = constant::PI / 2.0 - acos(dot_product(sun_unit, n));
    VERBOSEF("b:   %+.4f", b);

    auto e = acos(dot_product(satellite_unit, p));
    VERBOSEF("e:   %+.4f", e);

    auto mu = constant::PI / 2.0;
    if (dot_product(satellite_unit, sun_unit) <= 0.0) {
        mu -= e;
    } else {
        mu += e;
    }

    if (mu < -constant::PI) mu += 2.0 * constant::PI;
    if (mu >= constant::PI) mu -= 2.0 * constant::PI;
    VERBOSEF("mu:  %+.4f", mu);

    auto yaw = satellite_yaw_angle(b, mu);
    VERBOSEF("yaw: %+.4f", yaw);

    auto k = cross_product(n, satellite_unit);
    if (!k.normalize()) return false;
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    auto cos_yaw = cos(yaw);
    auto sin_yaw = sin(yaw);
    auto rx      = Float3{
        -sin_yaw * k.x + cos_yaw * n.x,
        -sin_yaw * k.y + cos_yaw * n.y,
        -sin_yaw * k.z + cos_yaw * n.z,
    };
    auto ry = Float3{
        -cos_yaw * k.x - sin_yaw * n.x,
        -cos_yaw * k.y - sin_yaw * n.y,
        -cos_yaw * k.z - sin_yaw * n.z,
    };
    auto rz = cross_product(rx, ry);
    if (!rz.normalize()) return false;

    VERBOSEF("rx:  %+.4f, %+.4f, %+.4f", rx.x, rx.y, rx.z);
    VERBOSEF("ry:  %+.4f, %+.4f, %+.4f", ry.x, ry.y, ry.z);
    VERBOSEF("rz:  %+.4f, %+.4f, %+.4f", rz.x, rz.y, rz.z);

    x = rx;
    y = ry;
    z = rz;
    return true;
}

static bool compute_receiver_antenna_basis(Float3 ground_position_llh, Float3& x, Float3& y,
                                           Float3& z) NOEXCEPT {
    VSCOPE_FUNCTIONF("(%+.4f, %+.4f, %+.4f)", ground_position_llh.x * constant::RAD2DEG,
                     ground_position_llh.y * constant::RAD2DEG, ground_position_llh.z);

    // enu frame of the receiver
    Float3 east, north, up;
    enu_basis_from_llh(ground_position_llh, east, north, up);

    // the receiver antenna basis
    auto rx = east;
    auto ry = north;
    auto rz = cross_product(rx, ry);
    if (!rz.normalize()) return false;

    VERBOSEF("rx:  %+.4f, %+.4f, %+.4f", rx.x, rx.y, rx.z);
    VERBOSEF("ry:  %+.4f, %+.4f, %+.4f", ry.x, ry.y, ry.z);
    VERBOSEF("rz:  %+.4f, %+.4f, %+.4f", rz.x, rz.y, rz.z);

    x = rx;
    y = ry;
    z = rz;
    return true;
}

static double phase_windup_from_basis(SatelliteState const& satellite, Float3 sx, Float3 sy,
                                      Float3 rx, Float3 ry, double prev_phw) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto k = satellite.true_line_of_sight;  // ground_position_ecef - satellite.true_position;
    if (!k.normalize()) {
        WARNF("failed to normalize k");
        return {};
    }
    VERBOSEF("k:   %+.4f, %+.4f, %+.4f", k.x, k.y, k.z);

    auto sd = sx - k * dot_product(k, sx) - cross_product(k, sy);
    auto rd = rx - k * dot_product(k, rx) + cross_product(k, ry);
    VERBOSEF("sd:  %+.4f, %+.4f, %+.4f", sd.x, sd.y, sd.z);
    VERBOSEF("rd:  %+.4f, %+.4f, %+.4f", rd.x, rd.y, rd.z);

    auto zeta = dot_product(cross_product(sd, rd), k);
    VERBOSEF("zeta: %+.4f", zeta);

    auto cosp = dot_product(sd, rd) / (rd.length() * sd.length());
    if (cosp < -1.0) cosp = -1.0;
    if (cosp > 1.0) cosp = 1.0;
    VERBOSEF("cos: %+.4f", cosp);
    auto ph = acos(cosp) / (2.0 * constant::PI);
    VERBOSEF("ph:  %+.4f", ph);
    if (zeta < 0.0) ph = -ph;

    VERBOSEF("phw: %+.4f", prev_phw);
    auto phw = ph + floor(prev_phw - ph + 0.5);
    VERBOSEF("phw: %+.4f (%+.4f)", phw, ph);

    return phw;
}

PhaseWindup model_phase_windup(ts::Tai const& time, SatelliteState const& satellite,
                               UNUSED Float3 ground_position_ecef, Float3 ground_position_llh,
                               PhaseWindup const& previous_windup) {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    auto sm = sun_and_moon_position_ecef(time);

    Float3 sx_sun, sy_sun, sz_sun;
    if (!compute_satellite_antenna_basis_sun(satellite.true_position, sm.sun, sx_sun, sy_sun,
                                             sz_sun)) {
        WARNF("failed to compute satellite antenna basis (sun)");
        return {};
    }

    Float3 sx_velocity, sy_velocity, sz_velocity;
    if (!compute_satellite_antenna_basis_velocity(ground_position_ecef, satellite.true_position,
                                                  satellite.true_velocity, sx_velocity, sy_velocity,
                                                  sz_velocity)) {
        WARNF("failed to compute satellite antenna basis (velocity)");
        return {};
    }

    Float3 sx_angle, sy_angle, sz_angle;
    if (!compute_satellite_antenna_basis_yaw(time, ground_position_ecef, satellite.true_position,
                                             satellite.true_velocity, sx_angle, sy_angle,
                                             sz_angle)) {
        WARNF("failed to compute satellite antenna basis (yaw)");
        return {};
    }

    Float3 rx, ry, rz;
    if (!compute_receiver_antenna_basis(ground_position_llh, rx, ry, rz)) {
        WARNF("failed to compute receiver antenna basis");
        return {};
    }

    auto prev_phw_sun      = 0.0;
    auto prev_phw_velocity = 0.0;
    auto prev_phw_yaw      = 0.0;
    if (previous_windup.valid) {
        prev_phw_sun      = previous_windup.correction_sun;
        prev_phw_velocity = previous_windup.correction_velocity;
        prev_phw_yaw      = previous_windup.correction_angle;
    }

    auto phw_sun = phase_windup_from_basis(satellite, sx_sun, sy_sun, rx, ry, prev_phw_sun);
    auto phw_velocity =
        phase_windup_from_basis(satellite, sx_velocity, sy_velocity, rx, ry, prev_phw_velocity);
    auto phw_yaw = phase_windup_from_basis(satellite, sx_angle, sy_angle, rx, ry, prev_phw_yaw);

    PhaseWindup result{};
    result.correction_sun      = phw_sun;
    result.correction_velocity = phw_velocity;
    result.correction_angle    = phw_yaw;
    result.valid               = true;
    return result;
}

}  // namespace tokoro
}  // namespace generator
