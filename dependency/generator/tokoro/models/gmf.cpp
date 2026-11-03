/*
 * Global Mapping Function (GMF), Boehm et al. (2006).
 *
 * This file is a derived work based on the IERS Conventions software routine GMF (gmf.f, IERS
 * Conventions Center, Boehm et al. 2006) from the IERS Conventions (2010) software collection.
 *
 * How this work is based on and differs from the original Software:
 *  - The algorithm and the spherical harmonic coefficients are those of the original GMF routine.
 *  - The routine is rewritten in C++ and renamed to `global_mapping_function` (the original name
 *    is GMF). It is not IERS Conventions software and has neither been distributed by nor
 *    endorsed by the IERS Conventions Center.
 *  - The Niell (1996) height correction of the hydrostatic mapping function can be disabled
 *    (`apply_height_correction`); the original routine always applies it.
 *  - The elevation is given as the zenith distance, like in the original routine.
 *
 * The IERS Conventions Software License that applies to the original Software follows.
 *
 *  Copyright (C) 2008
 *  IERS Conventions Center
 *
 *  ==================================
 *  IERS Conventions Software License
 *  ==================================
 *
 *  NOTICE TO USER:
 *
 *  BY USING THIS SOFTWARE YOU ACCEPT THE FOLLOWING TERMS AND CONDITIONS
 *  WHICH APPLY TO ITS USE.
 *
 *  1. The Software is provided by the IERS Conventions Center ("the
 *     Center").
 *
 *  2. Permission is granted to anyone to use the Software for any
 *     purpose, including commercial applications, free of charge,
 *     subject to the conditions and restrictions listed below.
 *
 *  3. You (the user) may adapt the Software and its algorithms for your
 *     own purposes and you may distribute the resulting "derived work"
 *     to others, provided that the derived work complies with the
 *     following requirements:
 *
 *     a) Your work shall be clearly identified so that it cannot be
 *        mistaken for IERS Conventions software and that it has been
 *        neither distributed by nor endorsed by the Center.
 *
 *     b) Your work (including source code) must contain descriptions of
 *        how the derived work is based upon and/or differs from the
 *        original Software.
 *
 *     c) The name(s) of all modified routine(s) that you distribute
 *        shall be changed.
 *
 *     d) The origin of the IERS Conventions components of your derived
 *        work must not be misrepresented; you must not claim that you
 *        wrote the original Software.
 *
 *     e) The source code must be included for all routine(s) that you
 *        distribute.  This notice must be reproduced intact in any
 *        source distribution.
 *
 *  4. In any published work produced by the user and which includes
 *     results achieved by using the Software, you shall acknowledge
 *     that the Software was used in obtaining those results.
 *
 *  5. The Software is provided to the user "as is" and the Center makes
 *     no warranty as to its use or performance.   The Center does not
 *     and cannot warrant the performance or results which the user may
 *     obtain by using the Software.  The Center makes no warranties,
 *     express or implied, as to non-infringement of third party rights,
 *     merchantability, or fitness for any particular purpose.  In no
 *     event will the Center be liable to the user for any consequential,
 *     incidental, or special damages, including any lost profits or lost
 *     savings, even if a Center representative has been advised of such
 *     damages, or for any claim by any third party.
 *
 *  Correspondence concerning IERS Conventions software should be
 *  addressed as follows:
 *
 *                     Gerard Petit
 *     Internet email: gpetit[at]bipm.org
 *     Postal address: IERS Conventions Center
 *                     Time, frequency and gravimetry section, BIPM
 *                     Pavillon de Breteuil
 *                     92312 Sevres  FRANCE
 *
 *     or
 *
 *                     Brian Luzum
 *     Internet email: brian.luzum[at]usno.navy.mil
 *     Postal address: IERS Conventions Center
 *                     Earth Orientation Department
 *                     3450 Massachusetts Ave, NW
 *                     Washington, DC 20392
 *
 *
 *-----------------------------------------------------------------------
 */

#include "gmf.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, gmf);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, gmf)

namespace generator {
namespace tokoro {

namespace {

static double const GMF_AH_MEAN[55] = {
    +1.2517e+02, +8.503e-01, +6.936e-02, -6.760e+00, +1.771e-01, +1.130e-02, +5.963e-01, +1.808e-02,
    +2.801e-03,  -1.414e-03, -1.212e+00, +9.300e-02, +3.683e-03, +1.095e-03, +4.671e-05, +3.959e-01,
    -3.867e-02,  +5.413e-03, -5.289e-04, +3.229e-04, +2.067e-05, +3.000e-01, +2.031e-02, +5.900e-03,
    +4.573e-04,  -7.619e-05, +2.327e-06, +3.845e-06, +1.182e-01, +1.158e-02, +5.445e-03, +6.219e-05,
    +4.204e-06,  -2.093e-06, +1.540e-07, -4.280e-08, -4.751e-01, -3.490e-02, +1.758e-03, +4.019e-04,
    -2.799e-06,  -1.287e-06, +5.468e-07, +7.580e-08, -6.300e-09, -1.160e-01, +8.301e-03, +8.771e-04,
    +9.955e-05,  -1.718e-06, -2.012e-06, +1.170e-08, +1.790e-08, -1.300e-09, +1.000e-10,
};

static double const GMF_BH_MEAN[55] = {
    +0.000e+00, +0.000e+00, +3.249e-02, +0.000e+00, +3.324e-02, +1.850e-02, +0.000e+00, -1.115e-01,
    +2.519e-02, +4.923e-03, +0.000e+00, +2.737e-02, +1.595e-02, -7.332e-04, +1.933e-04, +0.000e+00,
    -4.796e-02, +6.381e-03, -1.599e-04, -3.685e-04, +1.815e-05, +0.000e+00, +7.033e-02, +2.426e-03,
    -1.111e-03, -1.357e-04, -7.828e-06, +2.547e-06, +0.000e+00, +5.779e-03, +3.133e-03, -5.312e-04,
    -2.028e-05, +2.323e-07, -9.100e-08, -1.650e-08, +0.000e+00, +3.688e-02, -8.638e-04, -8.514e-05,
    -2.828e-05, +5.403e-07, +4.390e-07, +1.350e-08, +1.800e-09, +0.000e+00, -2.736e-02, -2.977e-04,
    +8.113e-05, +2.329e-07, +8.451e-07, +4.490e-08, -8.100e-09, -1.500e-09, +2.000e-10,
};

static double const GMF_AH_AMP[55] = {
    -2.738e-01, -2.837e+00, +1.298e-02, -3.588e-01, +2.413e-02, +3.427e-02, -7.624e-01, +7.272e-02,
    +2.160e-02, -3.385e-03, +4.424e-01, +3.722e-02, +2.195e-02, -1.503e-03, +2.426e-04, +3.013e-01,
    +5.762e-02, +1.019e-02, -4.476e-04, +6.790e-05, +3.227e-05, +3.123e-01, -3.535e-02, +4.840e-03,
    +3.025e-06, -4.363e-05, +2.854e-07, -1.286e-06, -6.725e-01, -3.730e-02, +8.964e-04, +1.399e-04,
    -3.990e-06, +7.431e-06, -2.796e-07, -1.601e-07, +4.068e-02, -1.352e-02, +7.282e-04, +9.594e-05,
    +2.070e-06, -9.620e-08, -2.742e-07, -6.370e-08, -6.300e-09, +8.625e-02, -5.971e-03, +4.705e-04,
    +2.335e-05, +4.226e-06, +2.475e-07, -8.850e-08, -3.600e-08, -2.900e-09, +0.000e+00,
};

static double const GMF_BH_AMP[55] = {
    +0.000e+00, +0.000e+00, -1.136e-01, +0.000e+00, -1.868e-01, -1.399e-02, +0.000e+00, -1.043e-01,
    +1.175e-02, -2.240e-03, +0.000e+00, -3.222e-02, +1.333e-02, -2.647e-03, -2.316e-05, +0.000e+00,
    +5.339e-02, +1.107e-02, -3.116e-03, -1.079e-04, -1.299e-05, +0.000e+00, +4.861e-03, +8.891e-03,
    -6.448e-04, -1.279e-05, +6.358e-06, -1.417e-07, +0.000e+00, +3.041e-02, +1.150e-03, -8.743e-04,
    -2.781e-05, +6.367e-07, -1.140e-08, -4.200e-08, +0.000e+00, -2.982e-02, -3.000e-03, +1.394e-05,
    -3.290e-05, -1.705e-07, +7.440e-08, +2.720e-08, -6.600e-09, +0.000e+00, +1.236e-02, -9.981e-04,
    -3.792e-05, -1.355e-05, +1.162e-06, -1.789e-07, +1.470e-08, -2.400e-09, -4.000e-10,
};

static double const GMF_AW_MEAN[55] = {
    +5.640e+01, +1.555e+00, -1.011e+00, -3.975e+00, +3.171e-02, +1.065e-01, +6.175e-01, +1.376e-01,
    +4.229e-02, +3.028e-03, +1.688e+00, -1.692e-01, +5.478e-02, +2.473e-02, +6.059e-04, +2.278e+00,
    +6.614e-03, -3.505e-04, -6.697e-03, +8.402e-04, +7.033e-04, -3.236e+00, +2.184e-01, -4.611e-02,
    -1.613e-02, -1.604e-03, +5.420e-05, +7.922e-05, -2.711e-01, -4.406e-01, -3.376e-02, -2.801e-03,
    -4.090e-04, -2.056e-05, +6.894e-06, +2.317e-06, +1.941e+00, -2.562e-01, +1.598e-02, +5.449e-03,
    +3.544e-04, +1.148e-05, +7.503e-06, -5.667e-07, -3.660e-08, +8.683e-01, -5.931e-02, -1.864e-03,
    -1.277e-04, +2.029e-04, +1.269e-05, +1.629e-06, +9.660e-08, -1.015e-07, -5.000e-10,
};

static double const GMF_BW_MEAN[55] = {
    +0.000e+00, +0.000e+00, +2.592e-01, +0.000e+00, +2.974e-02, -5.471e-01, +0.000e+00, -5.926e-01,
    -1.030e-01, -1.567e-02, +0.000e+00, +1.710e-01, +9.025e-02, +2.689e-02, +2.243e-03, +0.000e+00,
    +3.439e-01, +2.402e-02, +5.410e-03, +1.601e-03, +9.669e-05, +0.000e+00, +9.502e-02, -3.063e-02,
    -1.055e-03, -1.067e-04, -1.130e-04, +2.124e-05, +0.000e+00, -3.129e-01, +8.463e-03, +2.253e-04,
    +7.413e-05, -9.376e-05, -1.606e-06, +2.060e-06, +0.000e+00, +2.739e-01, +1.167e-03, -2.246e-05,
    -1.287e-04, -2.438e-05, -7.561e-07, +1.158e-06, +4.950e-08, +0.000e+00, -1.344e-01, +5.342e-03,
    +3.775e-04, -6.756e-05, -1.686e-06, -1.184e-06, +2.768e-07, +2.730e-08, +5.700e-09,
};

static double const GMF_AW_AMP[55] = {
    +1.023e-01, -2.695e+00, +3.417e-01, -1.405e-01, +3.175e-01, +2.116e-01, +3.536e+00, -1.505e-01,
    -1.660e-02, +2.967e-02, +3.819e-01, -1.695e-01, -7.444e-02, +7.409e-03, -6.262e-03, -1.836e+00,
    -1.759e-02, -6.256e-02, -2.371e-03, +7.947e-04, +1.501e-04, -8.603e-01, -1.360e-01, -3.629e-02,
    -3.706e-03, -2.976e-04, +1.857e-05, +3.021e-05, +2.248e+00, -1.178e-01, +1.255e-02, +1.134e-03,
    -2.161e-04, -5.817e-06, +8.836e-07, -1.769e-07, +7.313e-01, -1.188e-01, +1.145e-02, +1.011e-03,
    +1.083e-04, +2.570e-06, -2.140e-06, -5.710e-08, +2.000e-08, -1.632e+00, -6.948e-03, -3.893e-03,
    +8.592e-04, +7.577e-05, +4.539e-06, -3.852e-07, -2.213e-07, -1.370e-08, +5.800e-09,
};

static double const GMF_BW_AMP[55] = {
    +0.000e+00, +0.000e+00, -8.865e-02, +0.000e+00, -4.309e-01, +6.340e-02, +0.000e+00, +1.162e-01,
    +6.176e-02, -4.234e-03, +0.000e+00, +2.530e-01, +4.017e-02, -6.204e-03, +4.977e-03, +0.000e+00,
    -1.737e-01, -5.638e-03, +1.488e-04, +4.857e-04, -1.809e-04, +0.000e+00, -1.514e-01, -1.685e-02,
    +5.333e-03, -7.611e-05, +2.394e-05, +8.195e-06, +0.000e+00, +9.326e-02, -1.275e-02, -3.071e-04,
    +5.374e-05, -3.391e-05, -7.436e-06, +6.747e-07, +0.000e+00, -8.637e-02, -3.807e-03, -6.833e-04,
    -3.861e-05, -2.268e-05, +1.454e-06, +3.860e-07, -1.068e-07, +0.000e+00, -2.658e-02, -1.947e-03,
    +7.131e-04, -3.506e-05, +1.885e-07, +5.792e-07, +3.990e-08, +2.000e-08, -5.700e-09,
};

}  // namespace

GlobalMappingFunction global_mapping_function(double mjd, double latitude, double longitude,
                                              double height, double zenith_distance,
                                              bool apply_height_correction) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f, %f, %f, %f, %f", mjd, latitude * 180.0 / M_PI, longitude * 180.0 / M_PI,
                     height, zenith_distance * 180.0 / M_PI);

    constexpr double PI     = 3.1415926535897932384626433;
    constexpr double TWO_PI = 6.283185307179586476925287;

    // Reference day is 28 January 1980, taken from Niell (1996) to be consistent
    auto doy = mjd - 44239.0 + 1.0 - 28.0;
    auto t   = std::sin(latitude);

    // Degree and order of the expansion (9x9 EGM)
    constexpr int N = 9;
    constexpr int M = 9;

    // n!, shifted by one: dfac[i] = (i - 1)!
    double dfac[2 * N + 3];
    dfac[0] = 1.0;
    dfac[1] = 1.0;
    for (int i = 1; i <= 2 * N + 1; ++i) {
        dfac[i + 1] = dfac[i] * static_cast<double>(i);
    }

    // Legendre functions (Heiskanen and Moritz, Physical Geodesy, 1967, eq. 1-62)
    double p[N + 1][M + 1] = {};
    for (int i = 0; i <= N; ++i) {
        for (int j = 0; j <= std::min(i, M); ++j) {
            auto ir   = (i - j) / 2;
            auto sum1 = 0.0;
            for (int k = 0; k <= ir; ++k) {
                sum1 += ((k % 2 == 0) ? 1.0 : -1.0) * dfac[2 * i - 2 * k + 1] / dfac[k + 1] /
                        dfac[i - k + 1] / dfac[i - j - 2 * k + 1] * std::pow(t, i - j - 2 * k);
            }
            p[i][j] = 1.0 / std::pow(2.0, i) * std::sqrt(std::pow(1.0 - t * t, j)) * sum1;
        }
    }

    // Spherical harmonics
    double ap[55];
    double bp[55];
    auto   index = 0;
    for (int n = 0; n <= N; ++n) {
        for (int m = 0; m <= n; ++m) {
            ap[index] = p[n][m] * std::cos(m * longitude);
            bp[index] = p[n][m] * std::sin(m * longitude);
            ++index;
        }
    }

    // Hydrostatic mapping function
    auto   bh  = 0.0029;
    auto   c0h = 0.062;
    double phh;
    double c11h;
    double c10h;
    if (latitude < 0.0) {
        phh  = PI;
        c11h = 0.007;
        c10h = 0.002;
    } else {
        phh  = 0.0;
        c11h = 0.005;
        c10h = 0.001;
    }
    auto ch = c0h + ((std::cos(doy / 365.25 * TWO_PI + phh) + 1.0) * c11h / 2.0 + c10h) *
                        (1.0 - std::cos(latitude));

    auto ahm = 0.0;
    auto aha = 0.0;
    for (int i = 0; i < 55; ++i) {
        ahm += (GMF_AH_MEAN[i] * ap[i] + GMF_BH_MEAN[i] * bp[i]) * 1e-5;
        aha += (GMF_AH_AMP[i] * ap[i] + GMF_BH_AMP[i] * bp[i]) * 1e-5;
    }
    auto ah = ahm + aha * std::cos(doy / 365.25 * TWO_PI);

    auto sine   = std::sin(PI / 2.0 - zenith_distance);
    auto beta   = bh / (sine + ch);
    auto gamma  = ah / (sine + beta);
    auto topcon = 1.0 + ah / (1.0 + bh / (1.0 + ch));

    GlobalMappingFunction result{};
    result.hydrostatic = topcon / (sine + gamma);

    // Height correction for the hydrostatic mapping function from Niell (1996)
    if (apply_height_correction) {
        auto a_ht     = 2.53e-5;
        auto b_ht     = 5.49e-3;
        auto c_ht     = 1.14e-3;
        auto hs_km    = height / 1000.0;
        auto beta_ht  = b_ht / (sine + c_ht);
        auto gamma_ht = a_ht / (sine + beta_ht);
        auto top_ht   = 1.0 + a_ht / (1.0 + b_ht / (1.0 + c_ht));
        auto coef     = 1.0 / sine - top_ht / (sine + gamma_ht);
        result.hydrostatic += coef * hs_km;
    }

    // Wet mapping function
    auto bw = 0.00146;
    auto cw = 0.04391;

    auto awm = 0.0;
    auto awa = 0.0;
    for (int i = 0; i < 55; ++i) {
        awm += (GMF_AW_MEAN[i] * ap[i] + GMF_BW_MEAN[i] * bp[i]) * 1e-5;
        awa += (GMF_AW_AMP[i] * ap[i] + GMF_BW_AMP[i] * bp[i]) * 1e-5;
    }
    auto aw = awm + awa * std::cos(doy / 365.25 * TWO_PI);

    beta       = bw / (sine + cw);
    gamma      = aw / (sine + beta);
    topcon     = 1.0 + aw / (1.0 + bw / (1.0 + cw));
    result.wet = topcon / (sine + gamma);

    VERBOSEF("hydrostatic: %+.14f", result.hydrostatic);
    VERBOSEF("wet:         %+.14f", result.wet);
    return result;
}

}  // namespace tokoro
}  // namespace generator
