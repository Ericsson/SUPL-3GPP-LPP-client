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

#include <generator/tokoro/grid_interpolation.hpp>
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_STEC_Correction_r16;
struct GNSS_SSR_GriddedCorrection_r16;

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

    std::unordered_map<SatelliteId, double>  ionospheric_residual;
    std::unordered_map<SatelliteId, ts::Tai> ionospheric_residual_epoch;
    double                                   tropospheric_wet;
    double                                   tropospheric_dry;
    ts::Tai                                  tropospheric_epoch;

    bool has_ionospheric_residual(SatelliteId sv_id) const {
        return ionospheric_valid && ionospheric_residual.find(sv_id) != ionospheric_residual.end();
    }

    bool has_tropospheric_data() const { return tropspheric_valid; }
};

struct GridOptions {
    GridInterpolation interpolation{GridInterpolation::FourPoint};
    // Corners without data (masked, or without a value for the satellite) count as a zero residual
    // when the interpolation would otherwise fail; at least one corner must have data
    bool missing_as_zero{false};

    GridExtrapolation extrapolation{GridExtrapolation::None};
    // Extrapolation: the furthest distance [m] from the nearest point, 0 for no limit
    double extrapolation_distance{0.0};
};

struct TroposphericCorrection;
struct GridData {
    enum class GridStatus {
        Success,
        PositionOutsideGrid,
        MissingSatelliteData,
        StaleData,
    };

    // A cell of the grid that may contain a position. `corner` is ordered top-left, top-right,
    // bottom-left, bottom-right and is null for a masked point. `row`/`col` are the position
    // within the cell, 0..1 from the top-left corner towards south/east.
    struct Cell {
        GridPoint const* corner[4];
        double           row;
        double           col;
    };

    NODISCARD GridPoint const* point_at(long latitude_index, long longitude_index) const NOEXCEPT;
    // The cells that contain `llh` (up to 4, when the position is on the edge of cells), or 0
    NODISCARD long find_cells(Float3 llh, Cell* cells) const NOEXCEPT;

    GridStatus ionospheric(SatelliteId sv_id, Float3 llh, ts::Tai const& now, double max_age,
                           GridOptions const& options, double& ionospheric_residual) const NOEXCEPT;
    GridStatus tropospheric(Float3 llh, ts::Tai const& now, double max_age,
                            GridOptions const&      options,
                            TroposphericCorrection& correction) const NOEXCEPT;

    // True if the grid was built from a correction point set with the same id and layout
    NODISCARD bool matches(CorrectionPointSet const& set) const NOEXCEPT {
        if (is_list != (set.layout == CorrectionPointSet::Layout::List)) return false;
        if (is_list) {
            if (correction_point_set_id != set.set_id) return false;
            if (grid_points.size() != set.list_points.size()) return false;
            for (size_t i = 0; i < grid_points.size(); i++) {
                if (grid_points[i].position.x != set.list_points[i].x ||
                    grid_points[i].position.y != set.list_points[i].y)
                    return false;
            }
            return true;
        }
        return correction_point_set_id == set.set_id &&
               reference_latitude == set.reference_point_latitude &&
               reference_longitude == set.reference_point_longitude &&
               delta_latitude == set.step_of_latitude && delta_longitude == set.step_of_longitude &&
               number_of_steps_latitude == set.number_of_steps_latitude &&
               number_of_steps_longitude == set.number_of_steps_longitude && bitmask == set.bitmask;
    }

    void init(CorrectionPointSet const& correction_point_set) NOEXCEPT {
        is_list                   = correction_point_set.layout == CorrectionPointSet::Layout::List;
        correction_point_set_id   = correction_point_set.set_id;
        bitmask                   = correction_point_set.bitmask;
        reference_latitude        = correction_point_set.reference_point_latitude;
        reference_longitude       = correction_point_set.reference_point_longitude;
        delta_latitude            = correction_point_set.step_of_latitude;
        delta_longitude           = correction_point_set.step_of_longitude;
        number_of_steps_latitude  = correction_point_set.number_of_steps_latitude;
        number_of_steps_longitude = correction_point_set.number_of_steps_longitude;
        auto grid_point_count =
            is_list ? static_cast<long>(correction_point_set.list_points.size()) :
                      (number_of_steps_latitude + 1) * (number_of_steps_longitude + 1);
        // Start from empty storage: resizing in place would keep the old set's points (and
        // their residuals/epochs) wherever the old and new grids overlap.
        grid_points.clear();
        grid_points.resize(static_cast<size_t>(grid_point_count));
        array_lookup.clear();
    }

    void add_point(CorrectionPointInfo const& info) NOEXCEPT {
        assert(info.absolute_index >= 0);
        assert(info.absolute_index < static_cast<long>(grid_points.size()));
        auto& grid_point             = grid_points[static_cast<size_t>(info.absolute_index)];
        grid_point.valid             = info.is_valid;
        grid_point.position          = info.position;
        grid_point.array_index       = info.array_index;
        grid_point.absolute_index    = info.absolute_index;
        grid_point.tropspheric_valid = false;
        grid_point.ionospheric_valid = false;
        grid_point.ionospheric_residual.clear();
        grid_point.ionospheric_residual_epoch.clear();
        grid_point.latitude_index  = info.latitude_index;
        grid_point.longitude_index = info.longitude_index;

        if (info.is_valid && info.array_index >= 0) {
            auto array_index = static_cast<size_t>(info.array_index);
            if (array_lookup.size() <= array_index) array_lookup.resize(array_index + 1, -1);
            array_lookup[array_index] = info.absolute_index;
        }
    }

    GridPoint* point_from_array_index(long array_index) {
        if (array_index >= 0 && static_cast<size_t>(array_index) < array_lookup.size()) {
            auto absolute_index = array_lookup[static_cast<size_t>(array_index)];
            if (absolute_index >= 0 && absolute_index < static_cast<long>(grid_points.size())) {
                auto& grid_point = grid_points[static_cast<size_t>(absolute_index)];
                if (grid_point.valid && grid_point.array_index == array_index) return &grid_point;
            }
        }
        for (auto& grid_point : grid_points) {
            if (grid_point.valid && grid_point.array_index == array_index) return &grid_point;
        }
        return nullptr;
    }

    void print_grid();

    bool                   is_list;  // list of points, otherwise a regular grid
    uint16_t               correction_point_set_id;
    double                 reference_latitude;   // north-west corner [deg]
    double                 reference_longitude;  // north-west corner [deg]
    uint64_t               bitmask;
    double                 delta_latitude;
    double                 delta_longitude;
    long                   number_of_steps_latitude;
    long                   number_of_steps_longitude;
    std::vector<GridPoint> grid_points;
    std::vector<long>      array_lookup;  // array index -> absolute index (-1: none)
    ts::Tai                epoch_time;
    uint16_t               ssr_iod{0};  // SSR IOD of the gridded message
    bool                   ssr_iod_valid{false};
};

}  // namespace tokoro
}  // namespace generator
