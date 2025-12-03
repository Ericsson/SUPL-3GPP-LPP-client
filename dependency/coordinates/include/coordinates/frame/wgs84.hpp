#pragma once
#include "../ellipsoid.hpp"
#include "../frame.hpp"

namespace coordinates {

struct WGS84_G730 {};
struct WGS84_G873 {};
struct WGS84_G1150 {};
struct WGS84_G1674 {};
struct WGS84_G1762 {};
struct WGS84_G2139 {};
struct WGS84_G2296 {};

using Wgs84        = WGS84_G2296;
using Wgs84Current = Wgs84;

template <>
struct FrameTrait<WGS84_G730> {
    static constexpr FrameId            id              = FrameId::WGS84_G730;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1994.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<WGS84_G873> {
    static constexpr FrameId            id              = FrameId::WGS84_G873;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1997.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<WGS84_G1150> {
    static constexpr FrameId            id              = FrameId::WGS84_G1150;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2002.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<WGS84_G1674> {
    static constexpr FrameId            id              = FrameId::WGS84_G1674;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2012.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<WGS84_G1762> {
    static constexpr FrameId            id              = FrameId::WGS84_G1762;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2010.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<WGS84_G2139> {
    static constexpr FrameId            id              = FrameId::WGS84_G2139;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2021.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<WGS84_G2296> {
    static constexpr FrameId            id              = FrameId::WGS84_G2296;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2024.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

using WGS84 = WGS84_G1762;

}  // namespace coordinates
