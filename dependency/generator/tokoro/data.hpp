#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <time/tai.hpp>

#include "ecef.hpp"
#include "sv_id.hpp"
#include "wgs84.hpp"

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

    inline bool has_grid_point(GridIndex grid_index) const {
        auto index = 64 - 1 - grid_index.i;
        return (bitmask & (1ULL << index)) != 0;
    }

    NODISCARD GridIndex next_latitude(GridIndex index) const NOEXCEPT {
        return {index.x, index.y + 1, index.i + 1};
    }

    NODISCARD GridIndex prev_latitude(GridIndex index) const NOEXCEPT {
        return {index.x, index.y - 1, index.i - 1};
    }

    NODISCARD GridIndex next_longitude(GridIndex index) const NOEXCEPT {
        return {index.x + 1, index.y, index.i + number_of_steps_latitude + 1};
    }

    NODISCARD GridIndex prev_longitude(GridIndex index) const NOEXCEPT {
        return {index.x - 1, index.y, index.i - number_of_steps_latitude - 1};
    }

    NODISCARD Wgs84Position grid_point_position(GridIndex index) const NOEXCEPT;
    NODISCARD bool          array_to_index(long array_i, GridIndex& index) const NOEXCEPT;
    NODISCARD bool position_to_index(Wgs84Position position, GridIndex& index) const NOEXCEPT;

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
    ts::Tai reference_time;
    Float3  delta;      // {radial, along_track, cross_track}
    Float3  dot_delta;  // {radial, along_track, cross_track}

    NODISCARD bool correction(ts::Tai time, Float3 eph_position, Float3 eph_velocity,
                              Float3& result) const NOEXCEPT;
};

struct ClockCorrection {
    ts::Tai reference_time;
    double  c0;
    double  c1;
    double  c2;

    NODISCARD double correction(ts::Tai time) const NOEXCEPT;
};

struct CodeBiasCorrection {
    double bias;
};

struct PhaseBiasCorrection {
    double bias;
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
};

struct IonosphericPolynomial {
    double c00;
    double c01;
    double c10;
    double c11;
    double reference_point_latitude;
    double reference_point_longitude;
};

struct IonosphereGridPoint {
    Wgs84Position position;
    double        ionospheric;
};

struct IonosphereGrid {
    std::unordered_map<GridIndex, IonosphereGridPoint> points;

    NODISCARD bool interpolate_4_points(GridIndex top_left_index, GridIndex top_right_index,
                                        GridIndex bottom_left_index, GridIndex bottom_right_index,
                                        Wgs84Position position, double& correction) const NOEXCEPT;
};

struct TroposphereGridPoint {
    Wgs84Position          position;
    TroposphericCorrection tropospheric;
};

struct TroposphereGrid {
    std::unordered_map<GridIndex, TroposphereGridPoint> points;

    NODISCARD bool interpolate_4_points(GridIndex top_left_index, GridIndex top_right_index,
                                        GridIndex bottom_left_index, GridIndex bottom_right_index,
                                        Wgs84Position           position,
                                        TroposphericCorrection& correction) const NOEXCEPT;
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

    bool tropospheric(SatelliteId sv_id, EcefPosition position,
                      TroposphericCorrection& correction) const NOEXCEPT;
    bool ionospheric(SatelliteId sv_id, EcefPosition position,
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
    std::unordered_map<SatelliteId, IonosphereGrid>        mIonosphereGrid;
    std::unordered_map<SatelliteId::Gnss, TroposphereGrid> mTroposphereGrid;
};

}  // namespace tokoro
}  // namespace generator
