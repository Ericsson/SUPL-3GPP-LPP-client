#pragma once
#include "../ellipsoid.hpp"
#include "../frame.hpp"

namespace coordinates {

struct ITRF88 {};
struct ITRF89 {};
struct ITRF90 {};
struct ITRF91 {};
struct ITRF92 {};
struct ITRF93 {};
struct ITRF94 {};
struct ITRF96 {};
struct ITRF97 {};
struct ITRF2000 {};
struct ITRF2005 {};
struct ITRF2008 {};
struct ITRF2014 {};
struct ITRF2020 {};

using Itrf        = ITRF2020;
using ItrfCurrent = Itrf;

template <>
struct FrameTrait<ITRF88> {
    static constexpr FrameId            id              = FrameId::ITRF88;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF89> {
    static constexpr FrameId            id              = FrameId::ITRF89;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF90> {
    static constexpr FrameId            id              = FrameId::ITRF90;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF91> {
    static constexpr FrameId            id              = FrameId::ITRF91;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF92> {
    static constexpr FrameId            id              = FrameId::ITRF92;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF93> {
    static constexpr FrameId            id              = FrameId::ITRF93;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF94> {
    static constexpr FrameId            id              = FrameId::ITRF94;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1988.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF96> {
    static constexpr FrameId            id              = FrameId::ITRF96;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1997.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF97> {
    static constexpr FrameId            id              = FrameId::ITRF97;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1997.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF2000> {
    static constexpr FrameId            id              = FrameId::ITRF2000;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 1997.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF2005> {
    static constexpr FrameId            id              = FrameId::ITRF2005;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2005.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF2008> {
    static constexpr FrameId            id              = FrameId::ITRF2008;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2008.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF2014> {
    static constexpr FrameId            id              = FrameId::ITRF2014;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2010.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

template <>
struct FrameTrait<ITRF2020> {
    static constexpr FrameId            id              = FrameId::ITRF2020;
    static constexpr FrameTemporalModel temporal_model  = FrameTemporalModel::TimeDependent;
    static constexpr double             reference_epoch = 2015.0;
    static constexpr Ellipsoid          ellipsoid       = ellipsoids::Wgs84;
};

}  // namespace coordinates
