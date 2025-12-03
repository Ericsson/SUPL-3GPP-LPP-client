#pragma once
#include "../frame/etrf.hpp"
#include "../frame/itrf.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// Source: EUREF Technical Note 1: Relationship and Transformation between
// the International and the European Terrestrial Reference Systems, 2024
// Table 3: Transformation parameters from ITRFyy to ETRF2014 at epoch 2015.0 and their rates/year
//
// ITRF2020 -1.4 -0.9 1.4 -0.42 2.210 13.806 -20.020
// rates 0.0 -0.1 0.2 0.00 0.085 0.531 -0.770
// ITRF2014 0.0 0.0 0.0 0.00 2.210 13.806 -20.020
// rates 0.0 0.0 0.0 0.00 0.085 0.531 -0.770
// ITRF2008 -1.6 -1.9 -1.9 -0.13 2.210 13.806 -20.020
// rates 0.0 0.0 0.1 -0.03 0.085 0.531 -0.770
// ITRF2005 -4.1 -1.0 2.8 -1.07 2.210 13.806 -20.020
// rates -0.3 0.0 0.1 -0.03 0.085 0.531 -0.770
// ITRF2000 -1.2 -1.7 35.6 -2.67 2.210 13.806 -20.020
// rates -0.1 -0.1 1.9 -0.11 0.085 0.531 -0.770
// ITRF97 -7.9 3.0 79.3 -4.40 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790
// ITRF96 -7.9 3.0 79.3 -4.40 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790
// ITRF94 -7.9 3.0 79.3 -4.40 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790
// ITRF93 64.4 -2.8 72.7 -4.89 5.570 18.136 -20.770
// rates 2.8 0.1 2.5 -0.12 0.195 0.721 -0.840
// ITRF92 -15.9 1.0 87.3 -3.69 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790
// ITRF91 -27.9 -13.0 93.3 -5.09 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790
// ITRF90 -25.9 -9.0 109.3 -5.39 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790
// ITRF89 -30.9 -33.0 147.3 -8.79 2.210 13.806 -20.380
// rates -0.1 0.5 3.3 -0.12 0.085 0.531 -0.790

#define TRANSFORM(year, t1, t2, t3, d, r1, r2, r3, t1_rate, t2_rate, t3_rate, d_rate, r1_rate,     \
                  r2_rate, r3_rate)                                                                \
    template <>                                                                                    \
    struct TimeDependentHelmertTransform<ITRF##year, ETRF2014> {                                   \
        static constexpr bool                       is_defined = true;                             \
        static constexpr TimeDependentHelmertParams params() {                                     \
            return {                                                                               \
                {t1, t2, t3, r1, r2, r3, d},                                                       \
                {t1_rate, t2_rate, t3_rate, r1_rate, r2_rate, r3_rate, d_rate},                    \
                2015.0,                                                                            \
            };                                                                                     \
        }                                                                                          \
    };                                                                                             \
    template <>                                                                                    \
    struct TimeDependentHelmertTransform<ETRF2014, ITRF##year> {                                   \
        static constexpr bool             is_defined = true;                                       \
        static TimeDependentHelmertParams params() {                                               \
            return {                                                                               \
                Helmert7Params{t1, t2, t3, r1, r2, r3, d}.inverse(),                               \
                Helmert7Params{t1_rate, t2_rate, t3_rate, r1_rate, r2_rate, r3_rate, d_rate}       \
                    .inverse(),                                                                    \
                2015.0,                                                                            \
            };                                                                                     \
        }                                                                                          \
    };

TRANSFORM(2020, -1.4, -0.9, 1.4, -0.42, 2.210, 13.806, -20.020, 0.0, -0.1, 0.2, 0.00, 0.085, 0.531,
          -0.770)
TRANSFORM(2008, -1.6, -1.9, -1.9, -0.13, 2.210, 13.806, -20.020, 0.0, 0.0, 0.1, -0.03, 0.085, 0.531,
          -0.770)
TRANSFORM(2005, -4.1, -1.0, 2.8, -1.07, 2.210, 13.806, -20.020, -0.3, 0.0, 0.1, -0.03, 0.085, 0.531,
          -0.770)
TRANSFORM(2000, -1.2, -1.7, 35.6, -2.67, 2.210, 13.806, -20.020, -0.1, -0.1, 1.9, -0.11, 0.085,
          0.531, -0.770)
TRANSFORM(97, -7.9, 3.0, 79.3, -4.40, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085, 0.531,
          -0.790)
TRANSFORM(96, -7.9, 3.0, 79.3, -4.40, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085, 0.531,
          -0.790)
TRANSFORM(94, -7.9, 3.0, 79.3, -4.40, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085, 0.531,
          -0.790)
TRANSFORM(93, 64.4, -2.8, 72.7, -4.89, 5.570, 18.136, -20.770, 2.8, 0.1, 2.5, -0.12, 0.195, 0.721,
          -0.840)
TRANSFORM(92, -15.9, 1.0, 87.3, -3.69, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085, 0.531,
          -0.790)
TRANSFORM(91, -27.9, -13.0, 93.3, -5.09, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085,
          0.531, -0.790)
TRANSFORM(90, -25.9, -9.0, 109.3, -5.39, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085,
          0.531, -0.790)
TRANSFORM(89, -30.9, -33.0, 147.3, -8.79, 2.210, 13.806, -20.380, -0.1, 0.5, 3.3, -0.12, 0.085,
          0.531, -0.790)

#undef TRANSFORM

}  // namespace coordinates
