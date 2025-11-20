#include "glo.hpp"

#include <cmath>

#include <loglet/loglet.hpp>
#include <maths/float3.hpp>
#include <time/utc.hpp>

LOGLET_MODULE_FORWARD_REF(eph);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(eph)

namespace ephemeris {

// PZ-90 constants
CONSTEXPR static double GLO_MU      = 3.9860044e14;  // m³/s²
CONSTEXPR static double GLO_J2      = 1.0826257e-3;  // second zonal harmonic
CONSTEXPR static double GLO_AE      = 6378136.0;     // Earth equatorial radius (m)
CONSTEXPR static double GLO_OMEGA_E = 7.292115e-5;   // Earth rotation rate (rad/s)
CONSTEXPR static double GLO_C       = 2.99792458e8;  // speed of light (m/s)

// Integration parameters
CONSTEXPR static double GLO_STEP_SIZE = 60.0;
CONSTEXPR static int    GLO_MAX_ITER  = 100;

bool GloEphemeris::is_valid(ts::Glo const& time) const NOEXCEPT {
    if (health != 0) return false;
    auto dt = time.timestamp().difference(reference_time.timestamp()).full_seconds();
    return std::abs(dt) <= 1800.0;
}

// compute acceleration from position and velocity (ECEF frame)
static Float3 compute_acceleration(Float3 const& pos, Float3 const& vel, Float3 const& acc_eph) {
    auto r     = pos * 1000.0;  // convert km to m
    auto r_mag = r.length();
    auto r2    = r_mag * r_mag;
    auto r3    = r2 * r_mag;

    if (r2 <= 0.0) return Float3{0.0, 0.0, 0.0};

    // J2 perturbation coefficient
    auto a = 1.5 * GLO_J2 * GLO_MU * GLO_AE * GLO_AE / r2 / r3;
    auto b = 5.0 * r.z * r.z / r2;
    auto c = -GLO_MU / r3 - a * (1.0 - b);

    // acceleration with Earth rotation effects
    auto   omg2 = GLO_OMEGA_E * GLO_OMEGA_E;
    Float3 a_total;
    a_total.x = (c + omg2) * r.x + 2.0 * GLO_OMEGA_E * vel.y * 1000.0 + acc_eph.x * 1000.0;
    a_total.y = (c + omg2) * r.y - 2.0 * GLO_OMEGA_E * vel.x * 1000.0 + acc_eph.y * 1000.0;
    a_total.z = (c - 2.0 * a) * r.z + acc_eph.z * 1000.0;

    return a_total / 1000.0;  // convert back to km/s²
}

// Runge-Kutta 4th order integration step
static void rk4_step(Float3& pos, Float3& vel, double dt, Float3 const& acc_eph) {
    auto k1_pos = vel;
    auto k1_vel = compute_acceleration(pos, vel, acc_eph);

    auto pos2   = pos + k1_pos * (dt / 2.0);
    auto vel2   = vel + k1_vel * (dt / 2.0);
    auto k2_pos = vel2;
    auto k2_vel = compute_acceleration(pos2, vel2, acc_eph);

    auto pos3   = pos + k2_pos * (dt / 2.0);
    auto vel3   = vel + k2_vel * (dt / 2.0);
    auto k3_pos = vel3;
    auto k3_vel = compute_acceleration(pos3, vel3, acc_eph);

    auto pos4   = pos + k3_pos * dt;
    auto vel4   = vel + k3_vel * dt;
    auto k4_pos = vel4;
    auto k4_vel = compute_acceleration(pos4, vel4, acc_eph);

    pos = pos + (k1_pos + k2_pos * 2.0 + k3_pos * 2.0 + k4_pos) * (dt / 6.0);
    vel = vel + (k1_vel + k2_vel * 2.0 + k3_vel * 2.0 + k4_vel) * (dt / 6.0);
}

EphemerisResult GloEphemeris::compute(ts::Glo const& time) const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", ts::Utc(time).rtklib_time_string().c_str());

    // integrate from reference time to target time
    VERBOSEF("reference time: %s", ts::Utc(reference_time).rtklib_time_string().c_str());
    auto dt = time.timestamp().difference(reference_time.timestamp()).full_seconds();
    VERBOSEF("dt: %f", dt);

    // initial state (km, km/s)
    Float3 pos = position;
    Float3 vel = velocity;
    VERBOSEF("initial pos: (%f, %f, %f)", pos.x, pos.y, pos.z);
    VERBOSEF("initial vel: (%f, %f, %f)", vel.x, vel.y, vel.z);

    // integrate using Runge-Kutta 4th order
    auto t    = dt;
    auto step = (t < 0.0) ? -GLO_STEP_SIZE : GLO_STEP_SIZE;

    for (auto i = 0; std::abs(t) > 1e-9 && i < GLO_MAX_ITER; i++) {
        if (std::abs(t) < GLO_STEP_SIZE) step = t;
        rk4_step(pos, vel, step, acceleration);
        t -= step;
    }

    VERBOSEF("final pos: (%f, %f, %f)", pos.x, pos.y, pos.z);
    VERBOSEF("final vel: (%f, %f, %f)", vel.x, vel.y, vel.z);

    auto pos_m = pos * 1000.0;  // convert to meter
    auto vel_m = vel * 1000.0;  // convert to m/s

    auto dt_r    = calculate_relativistic_correction(pos_m, vel_m);
    auto dt_full = calculate_clock_bias(dt);

    // NOTE(ewasjon): glonass clock bias includes the relativistic correction, however, our system
    // has it split into two separate values. we don't currently "use" the relativistic correction
    // except adding it to the clock bias later. to be compatibile we will estimate the
    // relativistic correction and subtract it from clock bias and then add it back later - no
    // actually change will happen.
    auto dt_free = dt_full - dt_r;

    EphemerisResult result{};
    result.position                     = pos_m;
    result.velocity                     = vel_m;
    result.clock                        = dt_free;
    result.relativistic_correction_brdc = dt_r;

    VERBOSEF("clock: %+.14f", result.clock);
    VERBOSEF("relativistic: %+.14f", result.relativistic_correction_brdc);

    return result;
}

double GloEphemeris::calculate_clock_bias(double dt) const NOEXCEPT {
    FUNCTION_SCOPE();

    // satellite clock bias: dt = -tau_n + gamma_n * (t - t_b)
    auto clock_bias = -tau_n + gamma_n * dt;
    VERBOSEF("tau_n: %+.14f", tau_n);
    VERBOSEF("gamma_n: %+.14f", gamma_n);
    VERBOSEF("dt: %f", dt);
    VERBOSEF("clock_bias: %+.14f", clock_bias);

    return clock_bias;
}

double GloEphemeris::calculate_relativistic_correction(Float3 const& pos,
                                                       Float3 const& vel) const NOEXCEPT {
    FUNCTION_SCOPE();

    // relativistic correction: dt_r = -2 * (r · v) / c^2
    auto r_v = dot_product(pos, vel);
    auto t_r = -2.0 * r_v / (GLO_C * GLO_C);
    VERBOSEF("r_v: %+.14f", r_v);
    VERBOSEF("t_r: %+.14f (%+.14fm)", t_r, t_r * GLO_C);

    return t_r;
}

}  // namespace ephemeris
