#pragma once
#include "../frame/itrf.hpp"
#include "../frame/pz90.hpp"
#include "../frame/wgs84.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// PZ-90.11 to ITRF2008 at epoch 2010.0, no rates. Source: EPSG:7960 (from "Parametry Zemli 1990
// (PZ-90.11)"), which matches ICG-11 WG-D (2016) slide 11
// (https://www.unoosa.org/pdf/icg/2016/icg11/wgd/12wgd.pdf):
//   dX = -0.003 m, dY = -0.001 m, dZ = 0.000 m,
//   wx = +0.019 mas, wy = -0.042 mas, wz = +0.002 mas, m = 0
// EPSG gives the rotations in the coordinate frame convention; this library applies them as
// position vector (IERS) rotations, which have the opposite sign, so they are negated below.
template <>
struct TimeDependentHelmertTransform<PZ90_11, ITRF2008> {
    static constexpr bool                       is_defined = true;
    static constexpr TimeDependentHelmertParams params() {
        return {
            {-3.0, -1.0, 0.0, -0.019, 0.042, -0.002, 0.0},
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

template <>
struct TimeDependentHelmertTransform<ITRF2008, PZ90_11> {
    static constexpr bool             is_defined = true;
    static TimeDependentHelmertParams params() {
        return {
            Helmert7Params{-3.0, -1.0, 0.0, -0.019, 0.042, -0.002, 0.0}.inverse(),
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

// PZ-90 to PZ-90.02, EPSG:7702, epoch 2002.0, no rates, coordinate frame rotations in EPSG:
//   dX = -1.07 m, dY = -0.03 m, dZ = +0.02 m, wx = 0, wy = 0, wz = -130.0 mas, m = -0.22 ppm
template <>
struct TimeDependentHelmertTransform<PZ90, PZ90_02> {
    static constexpr bool                       is_defined = true;
    static constexpr TimeDependentHelmertParams params() {
        return {
            {-1070.0, -30.0, 20.0, 0.0, 0.0, 130.0, -220.0},
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2002.0,
        };
    }
};

template <>
struct TimeDependentHelmertTransform<PZ90_02, PZ90> {
    static constexpr bool             is_defined = true;
    static TimeDependentHelmertParams params() {
        return {
            Helmert7Params{-1070.0, -30.0, 20.0, 0.0, 0.0, 130.0, -220.0}.inverse(),
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2002.0,
        };
    }
};

// PZ-90.02 to PZ-90.11, EPSG:7703, epoch 2010.0, no rates, coordinate frame rotations in EPSG:
//   dX = -0.373 m, dY = +0.186 m, dZ = +0.202 m,
//   wx = -2.3 mas, wy = +3.54 mas, wz = -4.21 mas, m = -0.008 ppm
template <>
struct TimeDependentHelmertTransform<PZ90_02, PZ90_11> {
    static constexpr bool                       is_defined = true;
    static constexpr TimeDependentHelmertParams params() {
        return {
            {-373.0, 186.0, 202.0, 2.3, -3.54, 4.21, -8.0},
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

template <>
struct TimeDependentHelmertTransform<PZ90_11, PZ90_02> {
    static constexpr bool             is_defined = true;
    static TimeDependentHelmertParams params() {
        return {
            Helmert7Params{-373.0, 186.0, 202.0, 2.3, -3.54, 4.21, -8.0}.inverse(),
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

// PZ-90 to PZ-90.11, EPSG:7704, no epoch (the reference epoch below is not used, there are no
// rates), coordinate frame rotations in EPSG:
//   dX = -1.443 m, dY = +0.156 m, dZ = +0.222 m,
//   wx = -2.3 mas, wy = +3.54 mas, wz = -134.21 mas, m = -0.228 ppm
template <>
struct TimeDependentHelmertTransform<PZ90, PZ90_11> {
    static constexpr bool                       is_defined = true;
    static constexpr TimeDependentHelmertParams params() {
        return {
            {-1443.0, 156.0, 222.0, 2.3, -3.54, 134.21, -228.0},
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

template <>
struct TimeDependentHelmertTransform<PZ90_11, PZ90> {
    static constexpr bool             is_defined = true;
    static TimeDependentHelmertParams params() {
        return {
            Helmert7Params{-1443.0, 156.0, 222.0, 2.3, -3.54, 134.21, -228.0}.inverse(),
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

// WGS 84 (G1150) to PZ-90.02, EPSG:7961, epoch 2002.0, no rates, coordinate frame rotations in
// EPSG (all zero):
//   dX = +0.36 m, dY = -0.08 m, dZ = -0.18 m, no rotations, no scale
template <>
struct TimeDependentHelmertTransform<WGS84_G1150, PZ90_02> {
    static constexpr bool                       is_defined = true;
    static constexpr TimeDependentHelmertParams params() {
        return {
            {360.0, -80.0, -180.0, 0.0, 0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2002.0,
        };
    }
};

template <>
struct TimeDependentHelmertTransform<PZ90_02, WGS84_G1150> {
    static constexpr bool             is_defined = true;
    static TimeDependentHelmertParams params() {
        return {
            Helmert7Params{360.0, -80.0, -180.0, 0.0, 0.0, 0.0, 0.0}.inverse(),
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2002.0,
        };
    }
};

}  // namespace coordinates
