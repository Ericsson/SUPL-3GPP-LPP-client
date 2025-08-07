#include "gal.hpp"

#include <cmath>

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(eph, gal);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(eph, gal)

namespace ephemeris {

CONSTEXPR static double CONSTANT_MU              = 3.986004418e14;
CONSTEXPR static double CONSTANT_OMEGA_EARTH_DOT = 7.2921151467e-5;
CONSTEXPR static double CONSTANT_C               = 2.99792458e8;

bool GalEphemeris::is_valid(ts::Gst const& time) const NOEXCEPT {
    if ((time.week() % 4096) != (week_number % 4096)) {
        VERBOSEF("week number mismatch (expected: %u, actual: %u)", week_number,
                 time.week() % 4096);
        return false;
    }

    auto toe_tow  = static_cast<uint32_t>(toe);
    auto toe_frac = toe - toe_tow;

    auto fit_interval = 4 * 3600;
    auto toe_ts       = ts::Gst::from_week_tow(time.week(), toe_tow, toe_frac).timestamp();
    auto current_ts   = time.timestamp();

    auto difference = (current_ts - toe_ts).full_seconds();
    if (difference < -fit_interval || difference > fit_interval) {
        return false;
    }

    return true;
}

double GalEphemeris::calculate_elapsed_time(ts::Gst const& time, double reference) const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %g", ts::Utc(time).rtklib_time_string().c_str(), reference);

    auto reference_tow  = static_cast<uint32_t>(reference);
    auto reference_frac = reference - reference_tow;

    auto reference_ts =
        ts::Gst::from_week_tow(time.week(), reference_tow, reference_frac).timestamp();
    auto current_ts = time.timestamp();
    VERBOSEF("reference_ts: %f", reference_ts.full_seconds());
    VERBOSEF("current_ts: %f", current_ts.full_seconds());
    auto difference = (current_ts - reference_ts).full_seconds();

    VERBOSEF("difference: %f", difference);

    // this should be the total time difference in seconds, but to be safe we'll
    // we should handle the week crossover
    while (difference < -302400) {
        difference += 604800;
    }
    while (difference > 302400) {
        difference -= 604800;
    }

    return difference;
}

double GalEphemeris::calculate_elapsed_time_toe(ts::Gst const& time) const NOEXCEPT {
    return calculate_elapsed_time(time, toe);
}

double GalEphemeris::calculate_elapsed_time_toc(ts::Gst const& time) const NOEXCEPT {
    return calculate_elapsed_time(time, toc);
}

double GalEphemeris::calculate_eccentric_anomaly(double t_k) const NOEXCEPT {
    VSCOPE_FUNCTIONF("%f", t_k);

    // calculate the mean anomaly
    auto n0  = std::sqrt(CONSTANT_MU / std::pow(a, 3));
    auto n   = n0 + delta_n;
    auto m_k = m0 + n * t_k;
    VERBOSEF("a: %f", a);
    VERBOSEF("n0: %f", n0);
    VERBOSEF("delta_n: %f", delta_n);
    VERBOSEF("n: %f", n);
    VERBOSEF("m0: %f", m0);
    VERBOSEF("m_k: %f", m_k);

    // calculate the eccentric anomaly
    auto e_k = m_k;
    for (auto i = 0; i < 30; i++) {
        auto e_k_sin = e * std::sin(e_k);
        auto e_k_cos = e * std::cos(e_k);

        auto new_e_k   = e_k + (m_k - e_k + e_k_sin) / (1 - e_k_cos);
        auto delta_e_k = std::abs(new_e_k - e_k);
        VERBOSEF("%2i: e_k: %f, new_e_k: %f, delta_e_k: %f", i, e_k, new_e_k, delta_e_k);
        if (delta_e_k < 1e-12) {
            break;
        }

        e_k = new_e_k;
    }

    return e_k;
}

double GalEphemeris::calculate_eccentric_anomaly_rate(double e_k) const NOEXCEPT {
    auto n0 = std::sqrt(CONSTANT_MU / (a * a * a));
    auto n  = n0 + delta_n;
    return n / (1 - e * std::cos(e_k));
}

double GalEphemeris::calculate_clock_bias(ts::Gst const& time) const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", ts::Utc(time).rtklib_time_string().c_str());

    // elapsed time since the clock data reference time
    auto t_k = calculate_elapsed_time_toc(time);
    VERBOSEF("t_k: %+.14f", t_k);

    VERBOSEF("af0: %+.14f", af0);
    VERBOSEF("af1: %+.14f", af1);
    VERBOSEF("af2: %+.14f", af2);

    // satellite clock bias
    auto t_b = t_k;
    VERBOSEF("t_b: %+.14f", t_b);
    for (auto i = 0; i < 2; i++) {
        t_b = t_k - (af0 + af1 * t_b + af2 * t_b * t_b);
        VERBOSEF("t_b: %02d %+.14f", i, t_b);
    }

    VERBOSEF("t_b: -- %+.14f", t_b);

    auto delta_t_sv = af0 + af1 * t_b + af2 * t_b * t_b;
    VERBOSEF("delta_t_sv: %+.14f", delta_t_sv);

    auto clock_bias = delta_t_sv;
    VERBOSEF("clock_bias: %+.14f", clock_bias);
    return clock_bias;
}

double GalEphemeris::calculate_relativistic_correction(Float3 const& position,
                                                       Float3 const& velocity) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto r_v = dot_product(position, velocity);
    VERBOSEF("r_v: %+.14f", r_v);

    auto t_r = -2.0 * r_v / (CONSTANT_C * CONSTANT_C);
    VERBOSEF("t_r: %+.14f (%+.14fm)", t_r, t_r * CONSTANT_C);
    return t_r;
}

double GalEphemeris::calculate_relativistic_correction_idc(double e_k) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto e_sin   = std::sin(e_k);
    auto delta_t = -2.0 * e_sin * e * std::sqrt(a * CONSTANT_MU) / (CONSTANT_C * CONSTANT_C);
    VERBOSEF("delta_t: %+.14f", delta_t);

    return delta_t;
}

EphemerisResult GalEphemeris::compute(ts::Gst const& time) const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", ts::Utc(time).rtklib_time_string().c_str());

    // calculate the elapsed time since the ephemeris reference epoch
    auto t_k = calculate_elapsed_time_toe(time);
    VERBOSEF("t_k: %f", t_k);

    // calculate the eccentric anomaly
    auto e_k     = calculate_eccentric_anomaly(t_k);
    auto dot_e_k = calculate_eccentric_anomaly_rate(e_k);
    auto e_sin   = std::sin(e_k);
    auto e_cos   = std::cos(e_k);
    VERBOSEF("e_k: %f", e_k);
    VERBOSEF("dot_e_k: %f", dot_e_k);
    VERBOSEF("e_sin: %f", e_sin);
    VERBOSEF("e_cos: %f", e_cos);

    // calculate the true anomaly (this is not the equation referenced in the IS-GPS-200, but it is
    // equivalent, and is more stable around +-pi)
    // see: https://duncaneddy.github.io/rastro/user_guide/orbits/anomalies/
    auto sqrt_1_e2 = std::sqrt(1.0 - (e * e));
    auto v_k       = std::atan2(sqrt_1_e2 * e_sin, e_cos - e);
    auto dot_v_k   = dot_e_k * sqrt_1_e2 / (1.0 - e * e_cos);
    VERBOSEF("v_k: %f", v_k);
    VERBOSEF("dot_v_k: %f", dot_v_k);

    // calculate the argument of latitude
    auto phi_k = v_k + omega;
    VERBOSEF("phi_k: %f", phi_k);

    // calculate the second harmonic perturbations
    auto phi_k_sin = std::sin(2.0 * phi_k);
    auto phi_k_cos = std::cos(2.0 * phi_k);
    auto delta_u_k = cus * phi_k_sin + cuc * phi_k_cos;
    auto delta_r_k = crs * phi_k_sin + crc * phi_k_cos;
    auto delta_i_k = cis * phi_k_sin + cic * phi_k_cos;
    VERBOSEF("delta_u_k: %f", delta_u_k);
    VERBOSEF("delta_r_k: %f", delta_r_k);
    VERBOSEF("delta_i_k: %f", delta_i_k);

    // calculate the corrected argument of latitude
    auto u_k     = phi_k + delta_u_k;
    auto dot_u_k = dot_v_k + 2.0 * dot_v_k * (cus * phi_k_cos - cuc * phi_k_sin);
    VERBOSEF("u_k: %f", u_k);
    VERBOSEF("dot_u_k: %f", dot_u_k);

    // calculate the corrected radius
    auto r_k     = a * (1.0 - e * e_cos) + delta_r_k;
    auto dot_r_k = e * a * dot_e_k * e_sin + 2.0 * dot_v_k * (crs * phi_k_cos - crc * phi_k_sin);
    VERBOSEF("r_k: %f", r_k);
    VERBOSEF("dot_r_k: %f", dot_r_k);

    // calculate the corrected inclination
    auto i_k     = i0 + delta_i_k + idot * t_k;
    auto dot_i_k = idot + 2.0 * dot_v_k * (cis * phi_k_cos - cic * phi_k_sin);
    VERBOSEF("i_k: %f", i_k);
    VERBOSEF("dot_i_k: %f", dot_i_k);

    // calculate the position in the orbital plane
    auto x_k_prime = r_k * std::cos(u_k);
    auto y_k_prime = r_k * std::sin(u_k);
    VERBOSEF("x_k_prime: %f", x_k_prime);
    VERBOSEF("y_k_prime: %f", y_k_prime);

    // calculate the velocity in the orbital plane
    auto dot_x_k_prime = dot_r_k * std::cos(u_k) - r_k * dot_u_k * std::sin(u_k);
    auto dot_y_k_prime = dot_r_k * std::sin(u_k) + r_k * dot_u_k * std::cos(u_k);
    VERBOSEF("dot_x_k_prime: %f", dot_x_k_prime);
    VERBOSEF("dot_y_k_prime: %f", dot_y_k_prime);

    // calculate corrected longitude of ascending node
    auto omega_k =
        omega0 + (omega_dot - CONSTANT_OMEGA_EARTH_DOT) * t_k - CONSTANT_OMEGA_EARTH_DOT * toe;
    auto dot_omega_k = omega_dot - CONSTANT_OMEGA_EARTH_DOT;
    VERBOSEF("omega_k: %f", omega_k);
    VERBOSEF("dot_omega_k: %f", dot_omega_k);

    // calculate the position in the ECEF frame
    auto omega_k_sin = std::sin(omega_k);
    auto omega_k_cos = std::cos(omega_k);
    auto i_k_sin     = std::sin(i_k);
    auto i_k_cos     = std::cos(i_k);

    auto x_k = x_k_prime * omega_k_cos - y_k_prime * omega_k_sin * i_k_cos;
    auto y_k = x_k_prime * omega_k_sin + y_k_prime * omega_k_cos * i_k_cos;
    auto z_k = y_k_prime * i_k_sin;
    VERBOSEF("x_k: %f", x_k);
    VERBOSEF("y_k: %f", y_k);
    VERBOSEF("z_k: %f", z_k);

    // calculate the velocity in the ECEF frame
    auto dot_x_k =
        -x_k_prime * dot_omega_k * omega_k_sin + dot_x_k_prime * omega_k_cos -
        dot_y_k_prime * omega_k_sin * i_k_cos -
        y_k_prime * (dot_omega_k * omega_k_cos * i_k_cos - dot_i_k * omega_k_sin * i_k_sin);
    auto dot_y_k =
        x_k_prime * dot_omega_k * omega_k_cos + dot_x_k_prime * omega_k_sin +
        dot_y_k_prime * omega_k_cos * i_k_cos -
        y_k_prime * (dot_omega_k * omega_k_sin * i_k_cos + dot_i_k * omega_k_cos * i_k_sin);
    auto dot_z_k = dot_y_k_prime * i_k_sin + y_k_prime * dot_i_k * i_k_cos;
    VERBOSEF("dot_x_k: %f", dot_x_k);
    VERBOSEF("dot_y_k: %f", dot_y_k);
    VERBOSEF("dot_z_k: %f", dot_z_k);

    // calculate the clock bias
    auto clock = calculate_clock_bias(time);

    EphemerisResult result{};
    result.position = Float3{x_k, y_k, z_k};
    result.velocity = Float3{dot_x_k, dot_y_k, dot_z_k};
    result.clock    = clock;

    // relativistic correction
    auto rc_brdc  = calculate_relativistic_correction_idc(e_k);
    auto rc_dotrv = calculate_relativistic_correction(result.position, result.velocity);
    result.relativistic_correction_brdc  = rc_brdc;
    result.relativistic_correction_dotrv = rc_dotrv;
    return result;
}

}  // namespace ephemeris
