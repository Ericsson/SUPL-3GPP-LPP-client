#include "grid.hpp"
#include "constant.hpp"
#include "decode.hpp"
#include "ionosphere.hpp"
#include "troposphere.hpp"

#include <cmath>

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <GridElement-r16.h>
#include <SSR-ClockCorrectionSatelliteElement-r15.h>
#include <SSR-CodeBiasSatElement-r15.h>
#include <SSR-CodeBiasSignalElement-r15.h>
#include <SSR-OrbitCorrectionSatelliteElement-r15.h>
#include <SSR-PhaseBiasSatElement-r16.h>
#include <SSR-PhaseBiasSignalElement-r16.h>
#include <STEC-ResidualSatElement-r16.h>
#include <STEC-ResidualSatList-r16.h>
#include <STEC-SatElement-r16.h>
#include <TropospericDelayCorrection-r16.h>
EXTERNAL_WARNINGS_POP

#include <asn.1/bit_string.hpp>
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#include <iomanip>
#include <sstream>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE3(tokoro, data, grid);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, grid)

namespace generator {
namespace tokoro {

static double interpolate(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

GridPoint const* GridData::point_at(long latitude_index, long longitude_index) const NOEXCEPT {
    if (latitude_index < 0 || latitude_index > number_of_steps_latitude) return nullptr;
    if (longitude_index < 0 || longitude_index > number_of_steps_longitude) return nullptr;
    auto  index = latitude_index * (number_of_steps_longitude + 1) + longitude_index;
    auto& point = grid_points[static_cast<size_t>(index)];
    return point.valid ? &point : nullptr;
}

// Cells adjacent to `position` (in steps from the reference point): the cell containing it, plus
// the neighbouring cell when it lies on a cell edge. Positions on the outer edge belong to the
// last cell.
static long cell_candidates(double position, long steps, long* cells) {
    constexpr double EDGE_TOLERANCE = 1e-6;
    if (steps <= 0) return 0;
    if (position < -EDGE_TOLERANCE || position > static_cast<double>(steps) + EDGE_TOLERANCE)
        return 0;

    auto nearest = std::round(position);
    if (std::fabs(position - nearest) <= EDGE_TOLERANCE) {
        auto edge  = static_cast<long>(nearest);
        long count = 0;
        if (edge < steps) cells[count++] = edge;
        if (edge > 0) cells[count++] = edge - 1;
        return count;
    }

    cells[0] = static_cast<long>(std::floor(position));
    return 1;
}

long GridData::find_cells(Float3 llh, Cell* cells) const NOEXCEPT {
    FUNCTION_SCOPE();

    if (delta_latitude <= 0.0 || delta_longitude <= 0.0) return 0;

    auto row = (reference_latitude - llh.x * constant::RAD2DEG) / delta_latitude;
    auto col = (llh.y * constant::RAD2DEG - reference_longitude) / delta_longitude;
    // grids crossing the antimeridian: the position may be one revolution behind the reference
    auto revolution = 360.0 / delta_longitude;
    if (col < 0.0 && col + revolution <= static_cast<double>(number_of_steps_longitude) + 1e-6) {
        col += revolution;
    }
    VERBOSEF("grid position: row %+.6f col %+.6f", row, col);

    long rows[2];
    long cols[2];
    auto row_count = cell_candidates(row, number_of_steps_latitude, rows);
    auto col_count = cell_candidates(col, number_of_steps_longitude, cols);
    long count     = 0;
    for (long r = 0; r < row_count; r++) {
        for (long c = 0; c < col_count; c++) {
            auto& cell     = cells[count++];
            cell.corner[0] = point_at(rows[r], cols[c]);
            cell.corner[1] = point_at(rows[r], cols[c] + 1);
            cell.corner[2] = point_at(rows[r] + 1, cols[c]);
            cell.corner[3] = point_at(rows[r] + 1, cols[c] + 1);
            cell.row       = row - static_cast<double>(rows[r]);
            cell.col       = col - static_cast<double>(cols[c]);
        }
    }
    return count;
}

namespace {

// Nominal zenith delays that the SSR troposphere variations are relative to (TS 37.355)
constexpr double TROPO_NOMINAL_DRY = 2.3;
constexpr double TROPO_NOMINAL_WET = 0.252;

enum class Kind {
    Bilinear,  // all 4 corners, a corner without data is zero
    Plane,     // the 3 corners with data
    Nearest,   // the nearest corner
};

struct Plan {
    Kind kind;
    bool used[4];  // the corner has data and contributes to the value
    int  pivot;    // Plane: the corner next to both of the others
    int  nearest;  // Nearest
};

struct CellValue {
    double value;
    // Slope per cell, towards south (`row`) and east (`col`)
    bool   has_slope;
    double d_row;
    double d_col;
};

// The 3 corners with data span a right triangle with the corner opposite to the missing one in the
// right angle. The position has to be inside it.
bool plane_plan(bool const present[4], double row, double col, Plan& plan) {
    constexpr double TOLERANCE = 1e-9;

    int missing = 0;
    for (int i = 0; i < 4; i++) {
        if (!present[i]) missing = i;
    }
    auto pivot  = 3 - missing;
    auto inside = std::fabs(row - static_cast<double>(pivot >> 1)) +
                  std::fabs(col - static_cast<double>(pivot & 1));
    if (inside > 1.0 + TOLERANCE) return false;

    plan.kind  = Kind::Plane;
    plan.pivot = pivot;
    for (int i = 0; i < 4; i++)
        plan.used[i] = present[i];
    return true;
}

// Chooses how to interpolate in `cell` from the corners that have data. False if that is not
// possible with the interpolation of `options`.
bool make_plan(GridData::Cell const& cell, bool const present[4], GridOptions const& options,
               Plan& plan) {
    plan = {};

    int count = 0;
    for (int i = 0; i < 4; i++)
        if (present[i]) count++;

    auto nearest = (cell.row > 0.5 ? 2 : 0) + (cell.col > 0.5 ? 1 : 0);

    switch (options.interpolation) {
    case GridInterpolation::FourPoint: break;
    case GridInterpolation::ThreePoint:
        if (count == 3 && plane_plan(present, cell.row, cell.col, plan)) return true;
        break;
    case GridInterpolation::Nearest:
        if (present[nearest]) {
            plan.kind          = Kind::Nearest;
            plan.nearest       = nearest;
            plan.used[nearest] = true;
            return true;
        }
        break;
    }

    if (options.interpolation != GridInterpolation::Nearest && count == 4) {
        plan.kind = Kind::Bilinear;
        for (int i = 0; i < 4; i++)
            plan.used[i] = true;
        return true;
    }

    if (!options.missing_as_zero || count == 0) return false;

    if (options.interpolation == GridInterpolation::Nearest) {
        plan.kind    = Kind::Nearest;
        plan.nearest = nearest;
        return true;
    }

    plan.kind = Kind::Bilinear;
    for (int i = 0; i < 4; i++)
        plan.used[i] = present[i];
    return true;
}

// The first cell that can be used for `llh` and how. `has_data` tells if a corner has the data.
template <typename HasData>
GridData::GridStatus select_cell(GridData const& grid, Float3 llh, GridOptions const& options,
                                 HasData has_data, GridData::Cell& cell, Plan& plan) {
    GridData::Cell cells[4];
    auto           count        = grid.find_cells(llh, cells);
    auto           data_missing = false;
    for (long i = 0; i < count; i++) {
        bool valid[4];
        bool present[4];
        for (int j = 0; j < 4; j++) {
            valid[j]   = cells[i].corner[j] != nullptr;
            present[j] = valid[j] && has_data(*cells[i].corner[j]);
        }

        if (make_plan(cells[i], present, options, plan)) {
            cell = cells[i];
            return GridData::GridStatus::Success;
        }

        // Would the cell work if the corners had data? Then data is what is missing.
        Plan unused;
        if (make_plan(cells[i], valid, options, unused)) data_missing = true;
    }

    VERBOSEF("no usable cell found (%ld candidate cells)", count);
    return data_missing ? GridData::GridStatus::MissingSatelliteData :
                          GridData::GridStatus::PositionOutsideGrid;
}

CellValue interpolate_cell(Plan const& plan, GridData::Cell const& cell, double const v[4]) {
    CellValue result{};
    switch (plan.kind) {
    case Kind::Bilinear:
        result.value     = interpolate(interpolate(v[0], v[2], cell.row),
                                       interpolate(v[1], v[3], cell.row), cell.col);
        result.has_slope = true;
        result.d_row     = (1.0 - cell.col) * (v[2] - v[0]) + cell.col * (v[3] - v[1]);
        result.d_col     = (1.0 - cell.row) * (v[1] - v[0]) + cell.row * (v[3] - v[2]);
        break;
    case Kind::Plane: {
        auto pivot_row = plan.pivot >> 1;
        auto pivot_col = plan.pivot & 1;
        // the other corner in the same column (towards the other row) and in the same row
        auto row_corner  = (1 - pivot_row) * 2 + pivot_col;
        auto col_corner  = pivot_row * 2 + (1 - pivot_col);
        result.has_slope = true;
        result.d_row     = (v[row_corner] - v[plan.pivot]) * (pivot_row == 0 ? 1.0 : -1.0);
        result.d_col     = (v[col_corner] - v[plan.pivot]) * (pivot_col == 0 ? 1.0 : -1.0);
        result.value     = v[plan.pivot] + result.d_row * (cell.row - pivot_row) +
                       result.d_col * (cell.col - pivot_col);
        break;
    }
    case Kind::Nearest: result.value = v[plan.nearest]; break;
    }
    return result;
}

char const* kind_name(Kind kind) {
    switch (kind) {
    case Kind::Bilinear: return "bilinear";
    case Kind::Plane: return "plane (3 points)";
    case Kind::Nearest: return "nearest";
    }
    return "unknown";
}

}  // namespace

GridData::GridStatus GridData::ionospheric(SatelliteId sv_id, Float3 llh, ts::Tai const& now,
                                           double max_age, GridOptions const& options,
                                           double& ionospheric_residual) const NOEXCEPT {
    FUNCTION_SCOPE();

    Cell cell{};
    Plan plan{};
    auto status = select_cell(
        *this, llh, options,
        [sv_id](GridPoint const& point) {
            return point.has_ionospheric_residual(sv_id);
        },
        cell, plan);
    if (status != GridStatus::Success) {
        VERBOSEF("ionospheric grid correction not available");
        return status;
    }

    VERBOSEF("%s interpolation", kind_name(plan.kind));

    if (max_age > 0.0) {
        for (int i = 0; i < 4; i++) {
            if (!plan.used[i]) continue;
            auto age = now - cell.corner[i]->ionospheric_residual_epoch.at(sv_id);
            if (age > max_age) {
                VERBOSEF("ionospheric grid point residual is too old (%.1fs > %.1fs limit)", age,
                         max_age);
                return GridStatus::StaleData;
            }
        }
    }

    VERBOSEF("dx: %+.14f", cell.row);
    VERBOSEF("dy: %+.14f", cell.col);

    double values[4] = {0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < 4; i++) {
        if (!plan.used[i]) {
            VERBOSEF("corner %d: no data", i);
            continue;
        }
        values[i] = cell.corner[i]->ionospheric_residual.at(sv_id);
        VERBOSEF("corner %d: %ld/%ld: %+.14f", i, cell.corner[i]->array_index,
                 cell.corner[i]->absolute_index, values[i]);
    }

    ionospheric_residual = interpolate_cell(plan, cell, values).value;
    VERBOSEF("ionospheric: %+.14f", ionospheric_residual);
    return GridStatus::Success;
}

GridData::GridStatus GridData::tropospheric(Float3 llh, ts::Tai const& now, double max_age,
                                            GridOptions const&      options,
                                            TroposphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    Cell cell{};
    Plan plan{};
    auto status = select_cell(
        *this, llh, options,
        [](GridPoint const& point) {
            return point.has_tropospheric_data();
        },
        cell, plan);
    if (status != GridStatus::Success) {
        VERBOSEF("tropospheric grid correction not available");
        return status;
    }

    VERBOSEF("%s interpolation", kind_name(plan.kind));

    if (max_age > 0.0) {
        for (int i = 0; i < 4; i++) {
            if (!plan.used[i]) continue;
            auto age = now - cell.corner[i]->tropospheric_epoch;
            if (age > max_age) {
                VERBOSEF("tropospheric grid point is too old (%.1fs > %.1fs limit)", age, max_age);
                return GridStatus::StaleData;
            }
        }
    }

    VERBOSEF("dx: %+.14f", cell.row);
    VERBOSEF("dy: %+.14f", cell.col);

    // A corner without data has no variation from the nominal delay
    double wet[4] = {TROPO_NOMINAL_WET, TROPO_NOMINAL_WET, TROPO_NOMINAL_WET, TROPO_NOMINAL_WET};
    double dry[4] = {TROPO_NOMINAL_DRY, TROPO_NOMINAL_DRY, TROPO_NOMINAL_DRY, TROPO_NOMINAL_DRY};
    for (int i = 0; i < 4; i++) {
        if (!plan.used[i]) {
            VERBOSEF("corner %d: no data", i);
            continue;
        }
        wet[i] = cell.corner[i]->tropospheric_wet;
        dry[i] = cell.corner[i]->tropospheric_dry;
        VERBOSEF("corner %d wet: %ld/%ld: %+.14f", i, cell.corner[i]->array_index,
                 cell.corner[i]->absolute_index, wet[i]);
        VERBOSEF("corner %d dry: %ld/%ld: %+.14f", i, cell.corner[i]->array_index,
                 cell.corner[i]->absolute_index, dry[i]);
    }

    auto wet_value = interpolate_cell(plan, cell, wet);
    auto dry_value = interpolate_cell(plan, cell, dry);

    correction.wet          = wet_value.value;
    correction.dry          = dry_value.value;
    correction.has_gradient = false;

    VERBOSEF("tropospheric wet: %+.14f", correction.wet);
    VERBOSEF("tropospheric dry: %+.14f", correction.dry);

    // Slope of the interpolation, per metre north and east
    if (wet_value.has_slope && dry_value.has_slope) {
        auto lat_span = -delta_latitude;  // [deg], the rows run towards south
        auto lon_span = delta_longitude;  // [deg]
        auto cos_lat  = std::cos(llh.x);
        if (lat_span != 0.0 && lon_span != 0.0 && cos_lat > 1e-6) {
            // mean Earth radius
            constexpr double EARTH_RADIUS = 6371000.0;
            auto             deg_to_m_n   = EARTH_RADIUS * constant::DEG2RAD;
            auto             deg_to_m_e   = EARTH_RADIUS * cos_lat * constant::DEG2RAD;

            correction.dry_north    = dry_value.d_row / lat_span / deg_to_m_n;
            correction.dry_east     = dry_value.d_col / lon_span / deg_to_m_e;
            correction.wet_north    = wet_value.d_row / lat_span / deg_to_m_n;
            correction.wet_east     = wet_value.d_col / lon_span / deg_to_m_e;
            correction.has_gradient = true;
            VERBOSEF("tropospheric gradient dry: N %+.4e E %+.4e, wet: N %+.4e E %+.4e m/m",
                     correction.dry_north, correction.dry_east, correction.wet_north,
                     correction.wet_east);
        }
    }

    return GridStatus::Success;
}

namespace {

struct GridInterpolationName {
    GridInterpolation interpolation;
    char const*       name;
};

constexpr GridInterpolationName GRID_INTERPOLATION_NAMES[] = {
    {GridInterpolation::FourPoint, "four-point"},
    {GridInterpolation::ThreePoint, "three-point"},
    {GridInterpolation::Nearest, "nearest"},
};

}  // namespace

char const* grid_interpolation_name(GridInterpolation interpolation) NOEXCEPT {
    for (auto const& entry : GRID_INTERPOLATION_NAMES) {
        if (entry.interpolation == interpolation) return entry.name;
    }
    return "unknown";
}

bool parse_grid_interpolation(std::string const& name, GridInterpolation& interpolation) NOEXCEPT {
    for (auto const& entry : GRID_INTERPOLATION_NAMES) {
        if (name == entry.name) {
            interpolation = entry.interpolation;
            return true;
        }
    }
    return false;
}

std::string grid_interpolation_names() {
    std::string names;
    for (auto const& entry : GRID_INTERPOLATION_NAMES) {
        if (!names.empty()) names += '|';
        names += entry.name;
    }
    return names;
}

void GridData::print_grid() {
    // Calculate the maximum length of the indices and data values
    int max_index_length = 1;
    int max_data_length  = 1;

    // Print the header
    std::stringstream ss;
    ss << "  ";
    for (long x = 0; x <= number_of_steps_longitude; x++) {
        ss << std::setw(max_index_length) << std::hex << x << std::dec;
    }
    VERBOSEF("%s", ss.str().c_str());

    // Print the grid
    for (long y = 0; y <= number_of_steps_latitude; y++) {
        ss.str("");
        ss.clear();
        ss << std::setw(max_index_length) << y << "|";
        for (long x = 0; x <= number_of_steps_longitude; x++) {
            auto i          = y * (number_of_steps_longitude + 1) + x;
            auto grid_point = grid_points[static_cast<size_t>(i)];

            if (grid_point.valid) {
                if (grid_point.tropspheric_valid) {
                    if (grid_point.tropospheric_dry > 0.0) {
                        ss << std::setw(max_data_length) << "+";
                    } else {
                        ss << std::setw(max_data_length) << "-";
                    }
                } else {
                    ss << std::setw(max_data_length) << ".";
                }
            } else {
                ss << std::setw(max_data_length) << "*";
            }
        }
        VERBOSEF("%s", ss.str().c_str());
    }
}

}  // namespace tokoro
}  // namespace generator
