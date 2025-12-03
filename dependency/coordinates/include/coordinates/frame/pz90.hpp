#pragma once
#include "../ellipsoid.hpp"
#include "../frame.hpp"

namespace coordinates {

struct PZ90 {};
struct PZ90_02 {};
struct PZ90_11 {};

template <>
struct FrameTrait<PZ90> {
    static constexpr FrameId            id              = FrameId::PZ90;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1990.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Pz90;
};

template <>
struct FrameTrait<PZ90_02> {
    static constexpr FrameId            id              = FrameId::PZ90_02;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2002.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Pz90;
};

template <>
struct FrameTrait<PZ90_11> {
    static constexpr FrameId            id              = FrameId::PZ90_11;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2010.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Pz90;
};

}  // namespace coordinates
