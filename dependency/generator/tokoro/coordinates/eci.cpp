#include "eci.hpp"
#include "constant.hpp"
#include "models/astronomical_arguments.hpp"
#include "models/nutation.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(tokoro, eci);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, eci)

namespace generator {
namespace tokoro {

void eci_to_ecef_matrix(ts::Tai const& time, EciEarthParameters const& earth_params,
                        Mat3* transform_out, double* gmst_out) {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    auto xp      = earth_params.xp;
    auto yp      = earth_params.yp;
    auto ut1_utc = earth_params.ut1_utc;

    // Get Julian centuries since J2000
    auto t  = ts::Utc{time}.j2000_century(ut1_utc);
    auto t2 = t * t;
    auto t3 = t2 * t;
    VERBOSEF("t: %f", t);

    // Get Astronomical Arguments and Nutation
    auto args     = AstronomicalArguments::evaluate(t);
    auto nutation = Nutation::evaluate(t, args);

    // Precession angles based on IAU 1976 model
    auto ze  = (2306.2181 * t + 0.30188 * t2 + 0.017998 * t3) * constant::ARCSEC2RAD;
    auto th  = (2004.3109 * t - 0.42665 * t2 - 0.041833 * t3) * constant::ARCSEC2RAD;
    auto z   = (2306.2181 * t + 1.09468 * t2 + 0.018203 * t3) * constant::ARCSEC2RAD;
    auto eps = (84381.448 - 46.8150 * t - 0.00059 * t2 + 0.001813 * t3) * constant::ARCSEC2RAD;
    VERBOSEF("ze: %f", ze);
    VERBOSEF("th: %f", th);
    VERBOSEF("z: %f", z);
    VERBOSEF("eps: %f", eps);

    // Calculate precession matrix
    auto p = Mat3::rotate_z(-z) * Mat3::rotate_y(th) * Mat3::rotate_z(-ze);

    VERBOSEF("p:");
    VERBOSEF("  %+.14f %+.14f %+.14f", p.m[0], p.m[1], p.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", p.m[3], p.m[4], p.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", p.m[6], p.m[7], p.m[8]);

    VERBOSEF("  sin(-d_psi): %+.14f", sin(-nutation.d_psi));

    // Calculate nutation matrix
    auto n1 = Mat3::rotate_x(-eps - nutation.d_eps);
    auto n2 = Mat3::rotate_z(-nutation.d_psi);
    auto n3 = Mat3::rotate_x(eps);

    auto r = n1 * n2;
    auto n = r * n3;

    TRACEF("n1:");
    TRACEF("  %+.14f %+.14f %+.14f", n1.m[0], n1.m[1], n1.m[2]);
    TRACEF("  %+.14f %+.14f %+.14f", n1.m[3], n1.m[4], n1.m[5]);
    TRACEF("  %+.14f %+.14f %+.14f", n1.m[6], n1.m[7], n1.m[8]);
    TRACEF("n2:");
    TRACEF("  %+.14f %+.14f %+.14f", n2.m[0], n2.m[1], n2.m[2]);
    TRACEF("  %+.14f %+.14f %+.14f", n2.m[3], n2.m[4], n2.m[5]);
    TRACEF("  %+.14f %+.14f %+.14f", n2.m[6], n2.m[7], n2.m[8]);
    TRACEF("n3:");
    TRACEF("  %+.14f %+.14f %+.14f", n3.m[0], n3.m[1], n3.m[2]);
    TRACEF("  %+.14f %+.14f %+.14f", n3.m[3], n3.m[4], n3.m[5]);
    TRACEF("  %+.14f %+.14f %+.14f", n3.m[6], n3.m[7], n3.m[8]);
    TRACEF("r:");
    TRACEF("  %+.14f %+.14f %+.14f", r.m[0], r.m[1], r.m[2]);
    TRACEF("  %+.14f %+.14f %+.14f", r.m[3], r.m[4], r.m[5]);
    TRACEF("  %+.14f %+.14f %+.14f", r.m[6], r.m[7], r.m[8]);
    TRACEF("n:");
    TRACEF("  %+.14f %+.14f %+.14f", n.m[0], n.m[1], n.m[2]);
    TRACEF("  %+.14f %+.14f %+.14f", n.m[3], n.m[4], n.m[5]);
    TRACEF("  %+.14f %+.14f %+.14f", n.m[6], n.m[7], n.m[8]);

    // Calculate GMST and GAST
    auto gmst = ts::Utc{time}.gmst(ut1_utc);
    auto gast = gmst + nutation.d_psi * cos(eps);
    gast += (0.00264 * sin(args.d) + 0.000063 * sin(2.0 * args.d)) * constant::ARCSEC2RAD;

    VERBOSEF("gmst: %f", gmst);
    VERBOSEF("gast: %f", gast);

    if (gmst_out) {
        *gmst_out = gmst;
    }

    // ECI to ECEF matrix
    auto w = Mat3::rotate_y(-xp) * Mat3::rotate_x(-yp);
    auto u = (w * Mat3::rotate_z(gast)) * (n * p);

#ifdef EXTRA_VERBOSE
    VERBOSEF("w:");
    VERBOSEF("  %+.14f %+.14f %+.14f", w.m[0], w.m[1], w.m[2]);
    VERBOSEF("  %+.14f %+.14f %+.14f", w.m[3], w.m[4], w.m[5]);
    VERBOSEF("  %+.14f %+.14f %+.14f", w.m[6], w.m[7], w.m[8]);
    VERBOSEF("u:");
    VERBOSEF("  %+.4f %+.4f %+.4f", u.m[0], u.m[1], u.m[2]);
    VERBOSEF("  %+.4f %+.4f %+.4f", u.m[3], u.m[4], u.m[5]);
    VERBOSEF("  %+.4f %+.4f %+.4f", u.m[6], u.m[7], u.m[8]);
#endif

    if (transform_out) {
        *transform_out = u;
    }
}

}  // namespace tokoro
}  // namespace generator
