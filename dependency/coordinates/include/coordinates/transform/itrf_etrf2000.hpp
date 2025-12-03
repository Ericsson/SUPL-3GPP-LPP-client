#pragma once
#include "../frame/etrf.hpp"
#include "../frame/itrf.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// Source: EUREF Technical Note 1: Relationship and Transformation between
// the International and the European Terrestrial Reference Systems, 2024
// Table 4: Transformation parameters from ITRFyy to ETRF2000 at epoch 2015.0 and their rates/year
//
// ITRF2020 53.8 51.8 -82.2 2.25 2.106 12.740 -20.592
// rates 0.1 0.0 -1.7 0.11 0.081 0.490 -0.792
// ITRF2014 55.2 52.7 -83.6 2.67 2.106 12.740 -20.592
// rates 0.1 0.1 -1.9 0.11 0.081 0.490 -0.792
// ITRF2008 53.6 50.8 -85.5 2.54 2.106 12.740 -20.592
// rates 0.1 0.1 -1.8 0.08 0.081 0.490 -0.792
// ITRF2005 51.1 51.7 -80.8 1.60 2.106 12.740 -20.592
// rates -0.2 0.1 -1.8 0.08 0.081 0.490 -0.792
// ITRF2000 54.0 51.0 -48.0 0.00 2.106 12.740 -20.592
// rates 0.0 0.0 0.0 0.00 0.081 0.490 -0.792
// ITRF97 47.3 55.7 -4.3 -1.73 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812
// ITRF96 47.3 55.7 -4.3 -1.73 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812
// ITRF94 47.3 55.7 -4.3 -1.73 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812
// ITRF93 119.6 49.9 -10.9 -2.22 5.466 17.070 -21.342
// rates 2.9 0.2 0.6 -0.01 0.191 0.680 -0.862
// ITRF92 39.3 53.7 3.7 -1.02 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812
// ITRF91 27.3 39.7 9.7 -2.42 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812
// ITRF90 29.3 43.7 25.7 -2.72 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812
// ITRF89 24.3 19.7 63.7 -6.12 2.106 12.740 -20.952
// rates 0.0 0.6 1.4 -0.01 0.081 0.490 -0.812

#define TRANSFORM(year, t1, t2, t3, d, r1, r2, r3, t1_rate, t2_rate, t3_rate, d_rate, r1_rate,     \
                  r2_rate, r3_rate)                                                                \
    template <>                                                                                    \
    struct TimeDependentHelmertTransform<ITRF##year, ETRF2000> {                                   \
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
    struct TimeDependentHelmertTransform<ETRF2000, ITRF##year> {                                   \
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

TRANSFORM(2020, 53.8, 51.8, -82.2, 2.25, 2.106, 12.740, -20.592, 0.1, 0.0, -1.7, 0.11, 0.081, 0.490,
          -0.792)
TRANSFORM(2014, 55.2, 52.7, -83.6, 2.67, 2.106, 12.740, -20.592, 0.1, 0.1, -1.9, 0.11, 0.081, 0.490,
          -0.792)
TRANSFORM(2008, 53.6, 50.8, -85.5, 2.54, 2.106, 12.740, -20.592, 0.1, 0.1, -1.8, 0.08, 0.081, 0.490,
          -0.792)
TRANSFORM(2005, 51.1, 51.7, -80.8, 1.60, 2.106, 12.740, -20.592, -0.2, 0.1, -1.8, 0.08, 0.081,
          0.490, -0.792)
TRANSFORM(97, 47.3, 55.7, -4.3, -1.73, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)
TRANSFORM(96, 47.3, 55.7, -4.3, -1.73, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)
TRANSFORM(94, 47.3, 55.7, -4.3, -1.73, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)
TRANSFORM(93, 119.6, 49.9, -10.9, -2.22, 5.466, 17.070, -21.342, 2.9, 0.2, 0.6, -0.01, 0.191, 0.680,
          -0.862)
TRANSFORM(92, 39.3, 53.7, 3.7, -1.02, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)
TRANSFORM(91, 27.3, 39.7, 9.7, -2.42, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)
TRANSFORM(90, 29.3, 43.7, 25.7, -2.72, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)
TRANSFORM(89, 24.3, 19.7, 63.7, -6.12, 2.106, 12.740, -20.952, 0.0, 0.6, 1.4, -0.01, 0.081, 0.490,
          -0.812)

#undef TRANSFORM

}  // namespace coordinates
