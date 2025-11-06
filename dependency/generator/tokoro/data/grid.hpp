#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "set.hpp"
#include "sv_id.hpp"

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_STEC_Correction_r16;
struct GNSS_SSR_GriddedCorrection_r16;

namespace generator {
namespace tokoro {

struct GridIndex {
    long x;
    long y;
    long i;

    bool operator==(GridIndex const& other) const {
        return x == other.x && y == other.y && i == other.i;
    }
};

}  // namespace tokoro
}  // namespace generator

namespace std {
template <>
struct hash<generator::tokoro::GridIndex> {
    size_t operator()(generator::tokoro::GridIndex const& index) const {
        return hash<long>()(index.x) ^ hash<long>()(index.y) ^ hash<long>()(index.i);
    }
};
}  // namespace std

namespace generator {
namespace tokoro {

struct GridPoint {
    bool   valid;
    bool   tropspheric_valid;
    bool   ionospheric_valid;
    Float3 position;

    long array_index;
    long absolute_index;
    long latitude_index;
    long longitude_index;

    std::unordered_map<SatelliteId, double> ionospheric_residual;
    double                                  tropospheric_wet;
    double                                  tropospheric_dry;

    bool has_ionospheric_residual(SatelliteId sv_id) const {
        return ionospheric_residual.find(sv_id) != ionospheric_residual.end();
    }

    bool has_tropospheric_data() const { return tropspheric_valid; }
};

struct TroposphericCorrection;
struct GridData {
    enum class GridStatus {
        Success,
        PositionOutsideGrid,
        MissingSatelliteData,
    };

    GridPoint const* find_top_left(Float3 llh) const NOEXCEPT;
    GridPoint const* find_with_absolute_index(long absolute_index) const NOEXCEPT;
    bool find_4_points(Float3 llh, GridPoint const*& tl, GridPoint const*& tr, GridPoint const*& bl,
                       GridPoint const*& br) const NOEXCEPT;

    GridStatus ionospheric(SatelliteId sv_id, Float3 llh, double& ionospheric_residual) const NOEXCEPT;
    GridStatus tropospheric(Float3 llh, TroposphericCorrection& correction) const NOEXCEPT;

    void init(CorrectionPointSet const& correction_point_set) NOEXCEPT {
        mCorrectionPointSetId   = correction_point_set.set_id;
        mDeltaLatitude          = correction_point_set.step_of_latitude;
        mDeltaLongitude         = correction_point_set.step_of_longitude;
        mNumberOfStepsLatitude  = correction_point_set.number_of_steps_latitude;
        mNumberOfStepsLongitude = correction_point_set.number_of_steps_longitude;
        auto grid_point_count   = (mNumberOfStepsLatitude + 1) * (mNumberOfStepsLongitude + 1);
        mGridPoints.resize(static_cast<size_t>(grid_point_count));
    }

    void add_point(CorrectionPointInfo const& info) NOEXCEPT {
        assert(info.absolute_index >= 0);
        assert(info.absolute_index < static_cast<long>(mGridPoints.size()));
        auto& grid_point             = mGridPoints[static_cast<size_t>(info.absolute_index)];
        grid_point.valid             = info.is_valid;
        grid_point.position          = info.position;
        grid_point.array_index       = info.array_index;
        grid_point.absolute_index    = info.absolute_index;
        grid_point.tropspheric_valid = false;
        grid_point.ionospheric_valid = false;
        grid_point.latitude_index    = info.latitude_index;
        grid_point.longitude_index   = info.longitude_index;
    }

    GridPoint* point_from_array_index(long array_index) {
        for (auto& grid_point : mGridPoints) {
            if (grid_point.array_index == array_index) return &grid_point;
        }
        return nullptr;
    }

    void print_grid();

    uint16_t               mCorrectionPointSetId;
    double                 mDeltaLatitude;
    double                 mDeltaLongitude;
    long                   mNumberOfStepsLatitude;
    long                   mNumberOfStepsLongitude;
    std::vector<GridPoint> mGridPoints;
};

}  // namespace tokoro
}  // namespace generator
