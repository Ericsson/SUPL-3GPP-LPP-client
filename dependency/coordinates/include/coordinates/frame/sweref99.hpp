#pragma once
#include "../ellipsoid.hpp"
#include "../frame.hpp"

namespace coordinates {

struct Sweref99 {};

template <>
struct FrameTrait<Sweref99> {
    static constexpr FrameId            id              = FrameId::Sweref99;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1999.5;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Grs80;
};

}  // namespace coordinates
