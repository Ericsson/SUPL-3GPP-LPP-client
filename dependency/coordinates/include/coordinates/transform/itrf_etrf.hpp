#pragma once
#include "../frame/etrf.hpp"
#include "../frame/itrf.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// Source: EUREF Technical Note 1: Relationship and Transformation between
// the International and the European Terrestrial Reference Systems, 2024
// Table 1: Transformation parameters from ITRFyy to ETRFyy at epoch 1989.0 and their rates/year
//
// ETRFyy T1 T2 T3 D R1 R2 R3
// mm mm mm 10−9 mas mas mas
// ETRF2020 0.0 0.0 0.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.086 0.519 -0.753
// ETRF2014 0.0 0.0 0.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.085 0.531 -0.770
// ETRF2005 56.0 48.0 -37.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.054 0.518 -0.781
// ETRF2000 54.0 51.0 -48.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.081 0.490 -0.792
// ETRF97 41.0 41.0 -49.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.200 0.500 -0.650
// ETRF96 41.0 41.0 -49.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.200 0.500 -0.650
// ETRF94 41.0 41.0 -49.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.200 0.500 -0.650
// ETRF93 19.0 53.0 -21.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.320 0.780 -0.670
// ETRF92 38.0 40.0 -37.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.210 0.520 -0.680
// ETRF91 21.0 25.0 -37.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.210 0.520 -0.680
// ETRF90 19.0 28.0 -23.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.110 0.570 -0.710
// ETRF89 0.0 0.0 0.0 0.00 0.000 0.000 0.000
// rates 0.0 0.0 0.0 0.00 0.110 0.570 -0.710

#define TRANSFORM(year, t1, t2, t3, d, r1, r2, r3, t1_rate, t2_rate, t3_rate, d_rate, r1_rate,     \
                  r2_rate, r3_rate)                                                                \
    template <>                                                                                    \
    struct TimeDependentHelmertTransform<ITRF##year, ETRF##year> {                                 \
        static constexpr bool                       is_defined = true;                             \
        static constexpr TimeDependentHelmertParams params() {                                     \
            return {                                                                               \
                {t1, t2, t3, r1, r2, r3, d},                                                       \
                {t1_rate, t2_rate, t3_rate, r1_rate, r2_rate, r3_rate, d_rate},                    \
                1989.0,                                                                            \
            };                                                                                     \
        }                                                                                          \
    };                                                                                             \
    template <>                                                                                    \
    struct TimeDependentHelmertTransform<ETRF##year, ITRF##year> {                                 \
        static constexpr bool             is_defined = true;                                       \
        static TimeDependentHelmertParams params() {                                               \
            return {                                                                               \
                Helmert7Params{t1, t2, t3, r1, r2, r3, d}.inverse(),                               \
                Helmert7Params{t1_rate, t2_rate, t3_rate, r1_rate, r2_rate, r3_rate, d_rate}       \
                    .inverse(),                                                                    \
                1989.0,                                                                            \
            };                                                                                     \
        }                                                                                          \
    };

TRANSFORM(2020, 0.0, 0.0, 0.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.086, 0.519, -0.753)
TRANSFORM(2014, 0.0, 0.0, 0.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.085, 0.531, -0.770)
TRANSFORM(2005, 56.0, 48.0, -37.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.054, 0.518,
          -0.781)
TRANSFORM(2000, 54.0, 51.0, -48.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.081, 0.490,
          -0.792)
TRANSFORM(97, 41.0, 41.0, -49.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.200, 0.500,
          -0.650)
TRANSFORM(96, 41.0, 41.0, -49.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.200, 0.500,
          -0.650)
TRANSFORM(94, 41.0, 41.0, -49.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.200, 0.500,
          -0.650)
TRANSFORM(93, 19.0, 53.0, -21.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.320, 0.780,
          -0.670)
TRANSFORM(92, 38.0, 40.0, -37.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.210, 0.520,
          -0.680)
TRANSFORM(91, 21.0, 25.0, -37.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.210, 0.520,
          -0.680)
TRANSFORM(90, 19.0, 28.0, -23.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.110, 0.570,
          -0.710)
TRANSFORM(89, 0.0, 0.0, 0.0, 0.00, 0.000, 0.000, 0.000, 0.0, 0.0, 0.0, 0.00, 0.110, 0.570, -0.710)

#undef TRANSFORM

}  // namespace coordinates
