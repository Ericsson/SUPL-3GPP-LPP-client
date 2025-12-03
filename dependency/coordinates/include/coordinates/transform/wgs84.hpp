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

}  // namespace coordinates
