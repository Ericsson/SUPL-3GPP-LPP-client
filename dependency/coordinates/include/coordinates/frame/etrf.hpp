#pragma once
#include "../ellipsoid.hpp"
#include "../frame.hpp"

namespace coordinates {

struct ETRF89 {};
struct ETRF90 {};
struct ETRF91 {};
struct ETRF92 {};
struct ETRF93 {};
struct ETRF94 {};
struct ETRF96 {};
struct ETRF97 {};
struct ETRF2000 {};
struct ETRF2005 {};
struct ETRF2014 {};
struct ETRF2020 {};

template <>
struct FrameTrait<ETRF89> {
    static constexpr FrameId            id              = FrameId::ETRF89;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF90> {
    static constexpr FrameId            id              = FrameId::ETRF90;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF91> {
    static constexpr FrameId            id              = FrameId::ETRF91;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF92> {
    static constexpr FrameId            id              = FrameId::ETRF92;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF93> {
    static constexpr FrameId            id              = FrameId::ETRF93;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF94> {
    static constexpr FrameId            id              = FrameId::ETRF94;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF96> {
    static constexpr FrameId            id              = FrameId::ETRF96;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF97> {
    static constexpr FrameId            id              = FrameId::ETRF97;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF2000> {
    static constexpr FrameId            id              = FrameId::ETRF2000;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF2005> {
    static constexpr FrameId            id              = FrameId::ETRF2005;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF2014> {
    static constexpr FrameId            id              = FrameId::ETRF2014;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ETRF2020> {
    static constexpr FrameId            id              = FrameId::ETRF2020;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::FixedEpoch;
    static constexpr double             reference_epoch = 1989.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

}  // namespace coordinates
