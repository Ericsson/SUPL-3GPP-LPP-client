#pragma once
#include "../frame/itrf.hpp"
#include "../frame/pz90.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// PZ-90.11 to ITRF2008 at epoch 2010.0, no rates. Source: "Parametry Zemli 1990 (PZ-90.11)"
// reference document (2014), as given in the GLONASS ICD (2016 edition):
//   dX = -0.003 m, dY = -0.001 m, dZ = 0.000 m,
//   wx = +0.019 mas, wy = -0.042 mas, wz = +0.002 mas, m = -0.000 ppm
// The rotations are used with the IERS (position vector) convention of this library.
// TODO: confirm the values and the rotation sign convention against the source document.
template <>
struct TimeDependentHelmertTransform<PZ90_11, ITRF2008> {
    static constexpr bool                       is_defined = true;
    static constexpr TimeDependentHelmertParams params() {
        return {
            {-3.0, -1.0, 0.0, 0.019, -0.042, 0.002, 0.0},
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
            Helmert7Params{-3.0, -1.0, 0.0, 0.019, -0.042, 0.002, 0.0}.inverse(),
            {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
            2010.0,
        };
    }
};

}  // namespace coordinates
