#pragma once
#include "../frame/wgs84.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

// Source: NGA Office of Geomatics press release NGA-U-2023-02846 of 2024-01-02
// EPSG:10608 - WGS 84 (G2296) to ITRF2020
// WGS84 (G2296) is aligned to ITRF2020 at epoch 2024.0 (identity transformation)
template <>
struct Transform<WGS84_G2296, ITRF2020> {
    static double apply_epoch(double current_epoch, double) { return current_epoch; }

    static State apply(State const& state, double) {
        return {FrameTrait<ITRF2020>::id, state.epoch, state.position, state.velocity};
    }
};

template <>
struct Transform<ITRF2020, WGS84_G2296> {
    static double apply_epoch(double current_epoch, double) { return current_epoch; }

    static State apply(State const& state, double) {
        return {FrameTrait<WGS84_G2296>::id, state.epoch, state.position, state.velocity};
    }
};

// Source: EPSG, the realizations of WGS 84 that are aligned with an ITRF realization (identity
// transformations, accuracy 1 to 2 cm)
//   EPSG:9757 - WGS 84 (G2139) to ITRF2014
//   EPSG:7666 - WGS 84 (G1762) to ITRF2008
//   EPSG:7669 - WGS 84 (G1674) to ITRF2008
//   EPSG:7670 - WGS 84 (G1150) to ITRF2000
//   EPSG:9076 - WGS 84 (G873) to ITRF94
//   EPSG:7672 - WGS 84 (G730) to ITRF92
template <typename From, typename To>
struct IdentityTransform {
    static double apply_epoch(double current_epoch, double) { return current_epoch; }

    static State apply(State const& state, double) {
        return {FrameTrait<To>::id, state.epoch, state.position, state.velocity};
    }
};

#define COORDINATES_IDENTITY_TRANSFORM(WGS84_FRAME, ITRF_FRAME)                                    \
    template <>                                                                                    \
    struct Transform<WGS84_FRAME, ITRF_FRAME> : IdentityTransform<WGS84_FRAME, ITRF_FRAME> {};     \
    template <>                                                                                    \
    struct Transform<ITRF_FRAME, WGS84_FRAME> : IdentityTransform<ITRF_FRAME, WGS84_FRAME> {};

COORDINATES_IDENTITY_TRANSFORM(WGS84_G2139, ITRF2014)
COORDINATES_IDENTITY_TRANSFORM(WGS84_G1762, ITRF2008)
COORDINATES_IDENTITY_TRANSFORM(WGS84_G1674, ITRF2008)
COORDINATES_IDENTITY_TRANSFORM(WGS84_G1150, ITRF2000)
COORDINATES_IDENTITY_TRANSFORM(WGS84_G873, ITRF94)
COORDINATES_IDENTITY_TRANSFORM(WGS84_G730, ITRF92)

#undef COORDINATES_IDENTITY_TRANSFORM

}  // namespace coordinates
