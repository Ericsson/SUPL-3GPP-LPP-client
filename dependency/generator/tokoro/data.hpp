#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <maths/float3.hpp>
#include <time/tai.hpp>

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

struct CorrectionPointSet {
    uint16_t set_id;
    long     grid_point_count;
    double   reference_point_latitude;
    double   reference_point_longitude;
    double   step_of_latitude;
    double   step_of_longitude;
    long     number_of_steps_latitude;
    long     number_of_steps_longitude;
    uint64_t bitmask;

    // Convert array index (the index of the grid point in the array from LPP) to an absolute index
    // that includes invalid grid points.
    NODISCARD bool array_to_index(long array_index, long& index, bool& valid,
                                  Float3& position) const NOEXCEPT;

    NODISCARD double latitude_min() const NOEXCEPT { return reference_point_latitude; }
    NODISCARD double latitude_max() const NOEXCEPT {
        return reference_point_latitude +
               static_cast<double>(number_of_steps_latitude) * step_of_latitude;
    }
    NODISCARD double longitude_min() const NOEXCEPT { return reference_point_longitude; }
    NODISCARD double longitude_max() const NOEXCEPT {
        return reference_point_longitude +
               static_cast<double>(number_of_steps_longitude) * step_of_longitude;
    }
};

struct OrbitCorrection {
    ts::Tai  reference_time;
    uint16_t ssr_iod;
    uint16_t iod;
    Float3   delta;      // {radial, along_track, cross_track}
    Float3   dot_delta;  // {radial, along_track, cross_track}

    NODISCARD bool correction(ts::Tai time, Float3 eph_position, Float3 eph_velocity,
                              Float3& result, Float3* output_radial, Float3* output_along,
                              Float3* output_cross, double* output_delta) const NOEXCEPT;
};

struct ClockCorrection {
    ts::Tai  reference_time;
    uint16_t ssr_iod;
    double   c0;
    double   c1;
    double   c2;

    NODISCARD double correction(ts::Tai time) const NOEXCEPT;
};

struct CodeBiasCorrection {
    uint16_t ssr_iod;
    double   bias;
};

struct PhaseBiasCorrection {
    uint16_t ssr_iod;
    double   bias;
};

struct SignalCorrection {
    std::unordered_map<SignalId, CodeBiasCorrection>  code_bias;
    std::unordered_map<SignalId, PhaseBiasCorrection> phase_bias;
};

struct TroposphericCorrection {
    double wet;
    double dry;
};

struct IonosphericCorrection {
    double grid_residual;
    double polynomial_residual;
    double vtec_grid_residual;
    double vtec_polynomial_residual;
    double quality;
    bool   quality_valid;
};

struct IonosphericPolynomial {
    double c00;
    double c01;
    double c10;
    double c11;
    double reference_point_latitude;
    double reference_point_longitude;
    double quality_indicator;
    double quality_indicator_valid;
};

struct IonosphereGridPoint {
    Float3 position;
    double ionospheric;
};

struct IonosphereGrid {
    std::unordered_map<GridIndex, IonosphereGridPoint> points;

    NODISCARD bool interpolate_4_points(GridIndex top_left_index, GridIndex top_right_index,
                                        GridIndex bottom_left_index, GridIndex bottom_right_index,
                                        Float3 position, double& correction) const NOEXCEPT;
};

struct TroposphereGridPoint {
    Float3                 position;
    TroposphericCorrection tropospheric;
};

struct TroposphereGrid {
    std::unordered_map<GridIndex, TroposphereGridPoint> points;

    NODISCARD bool interpolate_4_points(GridIndex top_left_index, GridIndex top_right_index,
                                        GridIndex bottom_left_index, GridIndex bottom_right_index,
                                        Float3                  position,
                                        TroposphericCorrection& correction) const NOEXCEPT;
};

struct GridPoint {
    bool   valid;
    bool   tropspheric_valid;
    bool   ionospheric_valid;
    Float3 position;

    long array_index;
    long absolute_index;

    std::unordered_map<SatelliteId, double> ionospheric_residual;
    double                                  tropospheric_wet;
    double                                  tropospheric_dry;

    bool has_ionospheric_residual(SatelliteId sv_id) const {
        return ionospheric_residual.find(sv_id) != ionospheric_residual.end();
    }

    bool has_tropospheric_data() const { return tropspheric_valid; }
};

struct GridData {
    GridPoint const* find_top_left(Float3 llh) const NOEXCEPT;
    GridPoint const* find_with_absolute_index(long absolute_index) const NOEXCEPT;
    bool find_4_points(Float3 llh, GridPoint const*& tl, GridPoint const*& tr, GridPoint const*& bl,
                       GridPoint const*& br) const NOEXCEPT;

    bool ionospheric(SatelliteId sv_id, Float3 llh, double& ionospheric_residual) const NOEXCEPT;
    bool tropospheric(Float3 llh, TroposphericCorrection& correction) const NOEXCEPT;

    void init(CorrectionPointSet const& correction_point_set) NOEXCEPT {
        mDeltaLatitude          = correction_point_set.step_of_latitude;
        mDeltaLongitude         = correction_point_set.step_of_longitude;
        mNumberOfStepsLatitude  = correction_point_set.number_of_steps_latitude;
        mNumberOfStepsLongitude = correction_point_set.number_of_steps_longitude;
        auto grid_point_count   = (mNumberOfStepsLatitude + 1) * (mNumberOfStepsLongitude + 1);
        mGridPoints.resize(static_cast<size_t>(grid_point_count));
    }

    void add_point(long array_index, long absolute_index, bool valid, Float3 llh) {
        assert(absolute_index >= 0);
        assert(absolute_index < static_cast<long>(mGridPoints.size()));
        auto& grid_point             = mGridPoints[static_cast<size_t>(absolute_index)];
        grid_point.valid             = valid;
        grid_point.position          = llh;
        grid_point.array_index       = array_index;
        grid_point.absolute_index    = absolute_index;
        grid_point.tropspheric_valid = false;
        grid_point.ionospheric_valid = false;
    }

    GridPoint* point_from_array_index(long array_index) {
        for (auto& grid_point : mGridPoints) {
            if (grid_point.array_index == array_index) return &grid_point;
        }
        return nullptr;
    }

    double                 mDeltaLatitude;
    double                 mDeltaLongitude;
    long                   mNumberOfStepsLatitude;
    long                   mNumberOfStepsLongitude;
    std::vector<GridPoint> mGridPoints;
};

struct CorrectionData {
    ts::Tai const& latest_correction_time() const { return mLatestCorrectionTime; }

    SignalCorrection const* signal_corrections(SatelliteId id) const {
        auto it = mSignal.find(id);
        if (it != mSignal.end()) return &it->second;
        return nullptr;
    }

    OrbitCorrection const* orbit_correction(SatelliteId id) const {
        auto it = mOrbit.find(id);
        if (it != mOrbit.end()) return &it->second;
        return nullptr;
    }

    ClockCorrection const* clock_correction(SatelliteId id) const {
        auto it = mClock.find(id);
        if (it != mClock.end()) return &it->second;
        return nullptr;
    }

    bool tropospheric(SatelliteId sv_id, Float3 llh,
                      TroposphericCorrection& correction) const NOEXCEPT;
    bool ionospheric(SatelliteId sv_id, Float3 llh,
                     IonosphericCorrection& correction) const NOEXCEPT;

    void add_correction(long gnss_id, GNSS_SSR_OrbitCorrections_r15 const* orbit) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_ClockCorrections_r15 const* clock) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_CodeBias_r15 const* code_bias) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_PhaseBias_r16 const* code_bias) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16 const* stec,
                        CorrectionPointSet const& correction_point_set) NOEXCEPT;
    void add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16 const* grid,
                        CorrectionPointSet const& correction_point_set) NOEXCEPT;

    NODISCARD std::unordered_set<SignalId> const* signals(SatelliteId id) const {
        auto it = mSignals.find(id);
        if (it != mSignals.end()) return &it->second;
        return nullptr;
    }

    CorrectionPointSet const* mCorrectionPointSet;

private:
    ts::Tai mLatestCorrectionTime;

    std::unordered_map<SatelliteId, std::unordered_set<SignalId>> mSignals;

    std::unordered_map<SatelliteId, OrbitCorrection>  mOrbit;
    std::unordered_map<SatelliteId, ClockCorrection>  mClock;
    std::unordered_map<SatelliteId, SignalCorrection> mSignal;

    std::unordered_map<SatelliteId, IonosphericPolynomial> mIonosphericPolynomial;
    std::unordered_map<SatelliteId::Gnss, GridData>        mGrid;
};

}  // namespace tokoro
}  // namespace generator
