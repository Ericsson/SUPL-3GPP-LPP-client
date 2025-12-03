#pragma once
#include "../frame/etrf.hpp"
#include "../frame/itrf.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// Source: EUREF Technical Note 1: Relationship and Transformation between
// the International and the European Terrestrial Reference Systems, 2024
// Table 2: Transformation parameters from ITRFyy to ETRF2020 at epoch 2015.0 and their rates/year
//
// ITRF Solution T1 T2 T3 D R1 R2 R3
// mm mm mm 10−9 mas mas mas
// ITRF2020 0.0 0.0 0.0 0.00 2.236 13.494 -19.578
// rates 0.0 0.0 0.0 0.00 0.086 0.519 -0.753
// ITRF2014 1.4 0.9 -1.4 0.42 2.236 13.494 -19.578
// rates 0.0 0.1 -0.2 0.00 0.086 0.519 -0.753
// ITRF2008 -0.2 -1.0 -3.3 0.29 2.236 13.494 -19.578
// rates 0.0 0.1 -0.1 -0.03 0.086 0.519 -0.753
// ITRF2005 -2.7 -0.1 1.4 -0.65 2.236 13.494 -19.578
// rates -0.3 0.1 -0.1 -0.03 0.086 0.519 -0.753
// ITRF2000 0.2 -0.8 34.2 -2.25 2.236 13.494 -19.578
// rates -0.1 0.0 1.7 -0.11 0.086 0.519 -0.753
// ITRF97 -6.5 3.9 77.9 -3.98 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773
// ITRF96 -6.5 3.9 77.9 -3.98 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773
// ITRF94 -6.5 3.9 77.9 -3.98 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773
// ITRF93 65.8 -1.9 71.3 -4.47 5.596 17.824 -20.328
// rates 2.8 0.2 2.3 -0.12 0.196 0.709 -0.823
// ITRF92 -14.5 1.9 85.9 -3.27 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773
// ITRF91 -26.5 -12.1 91.9 -4.67 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773
// ITRF90 -24.5 -8.1 107.9 -4.97 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773
// ITRF89 -29.5 -32.1 145.9 -8.37 2.236 13.494 -19.938
// rates -0.1 0.6 3.1 -0.12 0.086 0.519 -0.773

#define TRANSFORM(year, t1, t2, t3, d, r1, r2, r3, t1_rate, t2_rate, t3_rate, d_rate, r1_rate,     \
                  r2_rate, r3_rate)                                                                \
    template <>                                                                                    \
    struct TimeDependentHelmertTransform<ITRF##year, ETRF2020> {                                   \
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
    struct TimeDependentHelmertTransform<ETRF2020, ITRF##year> {                                   \
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

TRANSFORM(2014, 1.4, 0.9, -1.4, 0.42, 2.236, 13.494, -19.578, 0.0, 0.1, -0.2, 0.00, 0.086, 0.519,
          -0.753)
TRANSFORM(2008, -0.2, -1.0, -3.3, 0.29, 2.236, 13.494, -19.578, 0.0, 0.1, -0.1, -0.03, 0.086, 0.519,
          -0.753)
TRANSFORM(2005, -2.7, -0.1, 1.4, -0.65, 2.236, 13.494, -19.578, -0.3, 0.1, -0.1, -0.03, 0.086,
          0.519, -0.753)
TRANSFORM(2000, 0.2, -0.8, 34.2, -2.25, 2.236, 13.494, -19.578, -0.1, 0.0, 1.7, -0.11, 0.086, 0.519,
          -0.753)
TRANSFORM(97, -6.5, 3.9, 77.9, -3.98, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086, 0.519,
          -0.773)
TRANSFORM(96, -6.5, 3.9, 77.9, -3.98, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086, 0.519,
          -0.773)
TRANSFORM(94, -6.5, 3.9, 77.9, -3.98, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086, 0.519,
          -0.773)
TRANSFORM(93, 65.8, -1.9, 71.3, -4.47, 5.596, 17.824, -20.328, 2.8, 0.2, 2.3, -0.12, 0.196, 0.709,
          -0.823)
TRANSFORM(92, -14.5, 1.9, 85.9, -3.27, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086, 0.519,
          -0.773)
TRANSFORM(91, -26.5, -12.1, 91.9, -4.67, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086,
          0.519, -0.773)
TRANSFORM(90, -24.5, -8.1, 107.9, -4.97, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086,
          0.519, -0.773)
TRANSFORM(89, -29.5, -32.1, 145.9, -8.37, 2.236, 13.494, -19.938, -0.1, 0.6, 3.1, -0.12, 0.086,
          0.519, -0.773)

#undef TRANSFORM

}  // namespace coordinates
