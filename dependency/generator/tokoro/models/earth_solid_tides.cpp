#include "earth_solid_tides.hpp"
#include "constant.hpp"
#include "coordinates/eci.hpp"
#include "coordinates/enu.hpp"
#include "models/astronomical_arguments.hpp"
#include "models/sun_moon.hpp"
#include "satellite.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, est);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, est)

namespace generator {
namespace tokoro {

namespace {

// IERS Conventions 2010 (TN 36), Table 7.3a: corrections [mm] for the frequency dependence of the
// Love and Shida numbers in the diurnal band. Arguments are multipliers of l, l', F, D and Omega.
struct DiurnalTerm {
    int    l, lp, f, d, om;
    double r_ip, r_op, t_ip, t_op;
};

static DiurnalTerm const DIURNAL_TERMS[] = {
    {1, 0, 2, 0, 2, -0.08, 0.00, -0.01, 0.01},    // Q1
    {0, 0, 2, 0, 1, -0.10, 0.00, 0.00, 0.00},     //
    {0, 0, 2, 0, 2, -0.51, 0.00, -0.02, 0.03},    // O1
    {1, 0, 0, 0, 0, 0.06, 0.00, 0.00, 0.00},      // No1
    {0, 1, 2, -2, 2, -0.06, 0.00, 0.00, 0.00},    // pi1
    {0, 0, 2, -2, 2, -1.23, -0.07, 0.06, 0.01},   // P1
    {0, 0, 0, 0, -1, -0.22, 0.01, 0.01, 0.00},    //
    {0, 0, 0, 0, 0, 12.00, -0.78, -0.67, -0.03},  // K1
    {0, 0, 0, 0, 1, 1.73, -0.12, -0.10, 0.00},    //
    {0, -1, 0, 0, 0, -0.50, -0.01, 0.03, 0.00},   // psi1
    {0, 0, -2, 2, -2, -0.11, 0.01, 0.01, 0.00},   // phi1
};

// IERS Conventions 2010 (TN 36), Table 7.3b: zonal (long period) band, same layout [mm].
static DiurnalTerm const ZONAL_TERMS[] = {
    {0, 0, 0, 0, 1, 0.47, 0.16, 0.23, 0.07},
    {0, 0, -2, 2, -2, -0.20, -0.11, -0.12, -0.05},  // Ssa
    {-1, 0, 0, 0, 0, -0.11, -0.09, -0.08, -0.04},   // Mm
    {0, 0, -2, 0, -2, -0.13, -0.15, -0.11, -0.07},  // Mf
    {0, 0, -2, 0, -1, -0.05, -0.06, -0.05, -0.03},
};

struct Body {
    double distance;
    double k;        // GM_j * Re^4 / (GM_earth * R_j^3)
    double sin_lat;  // body-fixed geocentric latitude
    double cos_lat;
    double longitude;  // body-fixed east longitude
    Float3 unit;
};

static bool make_body(Float3 const& position, double gm, Body& body) {
    body.distance = position.length();
    if (body.distance <= 0.0) return false;
    body.unit = position / body.distance;
    body.k    = (gm / constant::GME) * std::pow(constant::RE_WGS84, 4) /
             (body.distance * body.distance * body.distance);
    body.sin_lat   = position.z / body.distance;
    body.cos_lat   = std::sqrt(position.x * position.x + position.y * position.y) / body.distance;
    body.longitude = std::atan2(position.y, position.x);
    return true;
}

}  // namespace

Float3 solid_earth_tide_displacement(Float3 station_ecef, Float3 sun_ecef, Float3 moon_ecef,
                                     double gmst, AstronomicalArguments const& args) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto radius = station_ecef.length();
    if (radius <= 0.0) {
        VERBOSEF("station at the geocenter");
        return Float3{0.0, 0.0, 0.0};
    }

    // Geocentric station frame, as used by the IERS formulae (n is perpendicular to r).
    auto sin_phi = station_ecef.z / radius;
    auto cos_phi =
        std::sqrt(station_ecef.x * station_ecef.x + station_ecef.y * station_ecef.y) / radius;
    auto lambda   = std::atan2(station_ecef.y, station_ecef.x);
    auto sin_lam  = std::sin(lambda);
    auto cos_lam  = std::cos(lambda);
    auto sin_2phi = 2.0 * sin_phi * cos_phi;
    auto cos_2phi = cos_phi * cos_phi - sin_phi * sin_phi;

    Float3 r_hat{cos_phi * cos_lam, cos_phi * sin_lam, sin_phi};
    Float3 n_hat{-sin_phi * cos_lam, -sin_phi * sin_lam, cos_phi};
    Float3 e_hat{-sin_lam, cos_lam, 0.0};
    VERBOSEF("station: (%+.4f, %+.4f, %+.4f)", station_ecef.x, station_ecef.y, station_ecef.z);
    VERBOSEF("phi (geocentric): %+.14f, lambda: %+.14f", std::atan2(sin_phi, cos_phi), lambda);
    VERBOSEF("sin_2phi: %+.14f, cos_2phi: %+.14f", sin_2phi, cos_2phi);
    VERBOSEF("sun:  (%+.4f, %+.4f, %+.4f)", sun_ecef.x, sun_ecef.y, sun_ecef.z);
    VERBOSEF("moon: (%+.4f, %+.4f, %+.4f)", moon_ecef.x, moon_ecef.y, moon_ecef.z);
    VERBOSEF("gmst: %+.14f", gmst);

    Body sun{};
    Body moon{};
    if (!make_body(sun_ecef, constant::SUN_GRAVITATIONAL_CONSTANT, sun) ||
        !make_body(moon_ecef, constant::MOON_GRAVITATIONAL_CONSTANT, moon)) {
        VERBOSEF("failed to compute the sun and moon direction");
        return Float3{0.0, 0.0, 0.0};
    }
    Body const* bodies[2] = {&moon, &sun};
    VERBOSEF("moon: distance %+.4f, k %+.14f, unit (%+.14f, %+.14f, %+.14f)", moon.distance, moon.k,
             moon.unit.x, moon.unit.y, moon.unit.z);
    VERBOSEF("sun:  distance %+.4f, k %+.14f, unit (%+.14f, %+.14f, %+.14f)", sun.distance, sun.k,
             sun.unit.x, sun.unit.y, sun.unit.z);

    Float3 total{0.0, 0.0, 0.0};

    // Step 1: degree 2 with nominal Love/Shida numbers and their latitude dependence (7.2, 7.5)
    auto p2 = (3.0 * sin_phi * sin_phi - 1.0) / 2.0;
    auto h2 = 0.6078 + (-0.0006) * p2;
    auto l2 = 0.0847 + 0.0002 * p2;
    VERBOSEF("h2: %+.14f", h2);
    VERBOSEF("l2: %+.14f", l2);
    for (auto const* body : bodies) {
        auto a = dot_product(body->unit, r_hat);
        total  = total + body->k * (h2 * (3.0 * a * a - 1.0) / 2.0 * r_hat +
                                   3.0 * l2 * a * (body->unit - a * r_hat));
        VERBOSEF("a (R_j . r): %+.14f", a);
    }
    VERBOSEF("step 1, degree 2: (%+.14f, %+.14f, %+.14f)", total.x, total.y, total.z);

    // Step 1: degree 3, Moon only (7.6)
    {
        auto h3 = 0.292;
        auto l3 = 0.015;
        auto a  = dot_product(moon.unit, r_hat);
        auto k3 = (constant::MOON_GRAVITATIONAL_CONSTANT / constant::GME) *
                  std::pow(constant::RE_WGS84, 5) / std::pow(moon.distance, 4);
        auto degree3 = k3 * (h3 * (2.5 * a * a * a - 1.5 * a) * r_hat +
                             l3 * (7.5 * a * a - 1.5) * (moon.unit - a * r_hat));
        total        = total + degree3;
        VERBOSEF("k3: %+.14f", k3);
        VERBOSEF("step 1, degree 3: (%+.14f, %+.14f, %+.14f)", degree3.x, degree3.y, degree3.z);
    }

    auto before_out_of_phase = total;

    // Step 1: transverse displacement from l(1), diurnal (7.8) and semidiurnal (7.9)
    // Step 1: out-of-phase parts, diurnal (7.10) and semidiurnal (7.11)
    {
        auto l1_diurnal      = 0.0012;
        auto l1_semidiurnal  = 0.0024;
        auto h_i_diurnal     = -0.0025;
        auto h_i_semidiurnal = -0.0022;
        auto l_i             = -0.0007;
        for (auto const* body : bodies) {
            auto dl      = lambda - body->longitude;
            auto sin_dl  = std::sin(dl);
            auto cos_dl  = std::cos(dl);
            auto sin_2dl = std::sin(2.0 * dl);
            auto cos_2dl = std::cos(2.0 * dl);

            auto p21        = 3.0 * body->sin_lat * body->cos_lat;
            auto p22        = 3.0 * body->cos_lat * body->cos_lat;
            auto sin_2big   = 2.0 * body->sin_lat * body->cos_lat;
            auto cos_sq_big = body->cos_lat * body->cos_lat;
            auto k          = body->k;

            // (7.8)
            total = total - l1_diurnal * sin_phi * k * p21 *
                                (sin_phi * cos_dl * n_hat - cos_2phi * sin_dl * e_hat);
            // (7.9)
            total = total - 0.5 * l1_semidiurnal * sin_phi * cos_phi * k * p22 *
                                (cos_2dl * n_hat + sin_phi * sin_2dl * e_hat);
            // (7.10a), (7.10b)
            total = total - 0.75 * h_i_diurnal * k * sin_2big * sin_2phi * sin_dl * r_hat;
            total = total - 1.5 * l_i * k * sin_2big *
                                (cos_2phi * sin_dl * n_hat + sin_phi * cos_dl * e_hat);
            // (7.11a), (7.11b)
            total = total -
                    0.75 * h_i_semidiurnal * k * cos_sq_big * cos_phi * cos_phi * sin_2dl * r_hat;
            total = total + 0.75 * l_i * k * cos_sq_big *
                                (sin_2phi * sin_2dl * n_hat - 2.0 * cos_phi * cos_2dl * e_hat);
        }
    }
    {
        auto delta = total - before_out_of_phase;
        VERBOSEF("step 1, l(1) and out-of-phase: (%+.14f, %+.14f, %+.14f)", delta.x, delta.y,
                 delta.z);
    }
    auto before_step2 = total;

    // Step 2: frequency dependence of the diurnal band (7.12), TN 36 Table 7.3a [mm]
    auto theta_g = gmst + constant::PI;
    for (auto const& term : DIURNAL_TERMS) {
        auto fundamental = term.l * args.l + term.lp * args.lp + term.f * args.f + term.d * args.d +
                           term.om * args.omega;
        auto arg   = theta_g - fundamental + lambda;
        auto sin_a = std::sin(arg);
        auto cos_a = std::cos(arg);
        auto dr    = (term.r_ip * sin_a + term.r_op * cos_a) * sin_2phi;
        auto dt_e  = (term.t_ip * cos_a - term.t_op * sin_a) * sin_phi;
        auto dt_n  = (term.t_ip * sin_a + term.t_op * cos_a) * cos_2phi;
        total      = total + 1e-3 * (dr * r_hat + dt_n * n_hat + dt_e * e_hat);
        TRACEF(
            "diurnal term (l=%d lp=%d f=%d d=%d om=%d): arg %+.14f, dr %+.14f mm, dt_n %+.14f mm, "
            "dt_e %+.14f mm",
            term.l, term.lp, term.f, term.d, term.om, arg, dr, dt_n, dt_e);
    }
    {
        auto delta = total - before_step2;
        VERBOSEF("step 2, diurnal: (%+.14f, %+.14f, %+.14f)", delta.x, delta.y, delta.z);
    }
    before_step2 = total;

    // Step 2: zonal (long period) band (7.13), TN 36 Table 7.3b [mm]
    for (auto const& term : ZONAL_TERMS) {
        auto fundamental = term.l * args.l + term.lp * args.lp + term.f * args.f + term.d * args.d +
                           term.om * args.omega;
        auto theta = -fundamental;
        auto cos_t = std::cos(theta);
        auto sin_t = std::sin(theta);
        auto dr    = (1.5 * sin_phi * sin_phi - 0.5) * (term.r_ip * cos_t + term.r_op * sin_t);
        auto dt_n  = (term.t_ip * cos_t + term.t_op * sin_t) * sin_2phi;
        total      = total + 1e-3 * (dr * r_hat + dt_n * n_hat);
        TRACEF(
            "zonal term (l=%d lp=%d f=%d d=%d om=%d): theta %+.14f, dr %+.14f mm, dt_n %+.14f mm",
            term.l, term.lp, term.f, term.d, term.om, theta, dr, dt_n);
    }
    {
        auto delta = total - before_step2;
        VERBOSEF("step 2, zonal: (%+.14f, %+.14f, %+.14f)", delta.x, delta.y, delta.z);
    }

    VERBOSEF("total: (%+.14f, %+.14f, %+.14f)", total.x, total.y, total.z);
    return total;
}

EarthSolidTides model_earth_solid_tides(ts::Tai const& time, SatelliteState const& satellite,
                                        Float3 ground_position_ecef) {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    auto sm   = sun_and_moon_position_ecef(time);
    auto args = AstronomicalArguments::evaluate(terrestrial_time_century(time));

    EarthSolidTides result{};
    result.displacement_vector =
        solid_earth_tide_displacement(ground_position_ecef, sm.sun, sm.moon, sm.gmst, args);
    result.displacement = -dot_product(satellite.true_line_of_sight, result.displacement_vector);
    result.valid        = true;
    VERBOSEF("displacement (along line of sight): %+.14f", result.displacement);
    return result;
}

}  // namespace tokoro
}  // namespace generator
