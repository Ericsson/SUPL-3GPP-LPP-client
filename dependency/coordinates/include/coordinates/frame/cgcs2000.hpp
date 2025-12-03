#pragma once
#include "../ellipsoid.hpp"
#include "../frame.hpp"

namespace coordinates {

struct CGCS2000 {};

template <>
struct FrameTrait<CGCS2000> {
    static constexpr FrameId            id              = FrameId::CGCS2000;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 2000.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Grs80;
};

}  // namespace coordinates
