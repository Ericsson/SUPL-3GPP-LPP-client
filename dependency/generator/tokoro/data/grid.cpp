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

bool GridData::find_4_points(Float3 llh, GridPoint const*& tl, GridPoint const*& tr,
                             GridPoint const*& bl, GridPoint const*& br) const NOEXCEPT {
    FUNCTION_SCOPE();

    if (delta_latitude <= 0.0 || delta_longitude <= 0.0) return false;

    auto row = (reference_latitude - llh.x * constant::RAD2DEG) / delta_latitude;
    auto col = (llh.y * constant::RAD2DEG - reference_longitude) / delta_longitude;
    VERBOSEF("grid position: row %+.6f col %+.6f", row, col);

    long rows[2];
    long cols[2];
    auto row_count = cell_candidates(row, number_of_steps_latitude, rows);
    auto col_count = cell_candidates(col, number_of_steps_longitude, cols);
    for (long r = 0; r < row_count; r++) {
        for (long c = 0; c < col_count; c++) {
            auto p_tl = point_at(rows[r], cols[c]);
            auto p_tr = point_at(rows[r], cols[c] + 1);
            auto p_bl = point_at(rows[r] + 1, cols[c]);
            auto p_br = point_at(rows[r] + 1, cols[c] + 1);
            if (!p_tl || !p_tr || !p_bl || !p_br) {
                VERBOSEF("cell %ld,%ld has a masked corner", rows[r], cols[c]);
                continue;
            }
            tl = p_tl;
            tr = p_tr;
            bl = p_bl;
            br = p_br;
            return true;
        }
    }

    VERBOSEF("no complete cell found");
    return false;
}

GridData::GridStatus GridData::ionospheric(SatelliteId sv_id, Float3 llh, ts::Tai const& now,
                                           double  max_age,
                                           double& ionospheric_residual) const NOEXCEPT {
    FUNCTION_SCOPE();

    GridPoint const* tl = nullptr;
    GridPoint const* tr = nullptr;
    GridPoint const* bl = nullptr;
    GridPoint const* br = nullptr;
    if (!find_4_points(llh, tl, tr, bl, br)) {
        return GridStatus::PositionOutsideGrid;
    }

    VERBOSEF("bilinear interpolation");

    if (!tl->has_ionospheric_residual(sv_id) || !tr->has_ionospheric_residual(sv_id) ||
        !bl->has_ionospheric_residual(sv_id) || !br->has_ionospheric_residual(sv_id)) {
        VERBOSEF("ionospheric correction not found");
        return GridStatus::MissingSatelliteData;
    }

    if (max_age > 0.0) {
        for (auto* corner : {tl, tr, bl, br}) {
            auto age = now - corner->ionospheric_residual_epoch.at(sv_id);
            if (age > max_age) {
                VERBOSEF("ionospheric grid point residual is too old (%.1fs > %.1fs limit)", age,
                         max_age);
                return GridStatus::StaleData;
            }
        }
    }

    auto dx = (llh.x * constant::RAD2DEG - tl->position.x) / (br->position.x - tl->position.x);
    auto dy = (llh.y * constant::RAD2DEG - tl->position.y) / (br->position.y - tl->position.y);

    VERBOSEF("dx: %+.14f", dx);
    VERBOSEF("dy: %+.14f", dy);

    auto tl_value = tl->ionospheric_residual.at(sv_id);
    auto tr_value = tr->ionospheric_residual.at(sv_id);
    auto bl_value = bl->ionospheric_residual.at(sv_id);
    auto br_value = br->ionospheric_residual.at(sv_id);

    VERBOSEF("tl: %ld/%ld: %+.14f", tl->array_index, tl->absolute_index, tl_value);
    VERBOSEF("tr: %ld/%ld: %+.14f", tr->array_index, tr->absolute_index, tr_value);
    VERBOSEF("bl: %ld/%ld: %+.14f", bl->array_index, bl->absolute_index, bl_value);
    VERBOSEF("br: %ld/%ld: %+.14f", br->array_index, br->absolute_index, br_value);

    ionospheric_residual =
        interpolate(interpolate(tl_value, bl_value, dx), interpolate(tr_value, br_value, dx), dy);
    VERBOSEF("ionospheric: %+.14f", ionospheric_residual);
    return GridStatus::Success;
}

GridData::GridStatus GridData::tropospheric(Float3 llh, ts::Tai const& now, double max_age,
                                            TroposphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    // if we're inside 4 points, bilinear interpolation
    GridPoint const* tl = nullptr;
    GridPoint const* tr = nullptr;
    GridPoint const* bl = nullptr;
    GridPoint const* br = nullptr;
    if (find_4_points(llh, tl, tr, bl, br)) {
        VERBOSEF("bilinear interpolation");

        if (!tl->has_tropospheric_data() || !tr->has_tropospheric_data() ||
            !bl->has_tropospheric_data() || !br->has_tropospheric_data()) {
            VERBOSEF("tropospheric correction not found");
            return GridStatus::MissingSatelliteData;
        }

        if (max_age > 0.0) {
            for (auto* corner : {tl, tr, bl, br}) {
                auto age = now - corner->tropospheric_epoch;
                if (age > max_age) {
                    VERBOSEF("tropospheric grid point is too old (%.1fs > %.1fs limit)", age,
                             max_age);
                    return GridStatus::StaleData;
                }
            }
        }

        auto dx = (llh.x * constant::RAD2DEG - tl->position.x) / (br->position.x - tl->position.x);
        auto dy = (llh.y * constant::RAD2DEG - tl->position.y) / (br->position.y - tl->position.y);

        VERBOSEF("dx: %+.14f", dx);
        VERBOSEF("dy: %+.14f", dy);

        auto tl_value_wet = tl->tropospheric_wet;
        auto tr_value_wet = tr->tropospheric_wet;
        auto bl_value_wet = bl->tropospheric_wet;
        auto br_value_wet = br->tropospheric_wet;

        VERBOSEF("tl wet: %ld/%ld: %+.14f", tl->array_index, tl->absolute_index, tl_value_wet);
        VERBOSEF("tr wet: %ld/%ld: %+.14f", tr->array_index, tr->absolute_index, tr_value_wet);
        VERBOSEF("bl wet: %ld/%ld: %+.14f", bl->array_index, bl->absolute_index, bl_value_wet);
        VERBOSEF("br wet: %ld/%ld: %+.14f", br->array_index, br->absolute_index, br_value_wet);

        auto tl_value_dry = tl->tropospheric_dry;
        auto tr_value_dry = tr->tropospheric_dry;
        auto bl_value_dry = bl->tropospheric_dry;
        auto br_value_dry = br->tropospheric_dry;

        VERBOSEF("tl dry: %ld/%ld: %+.14f", tl->array_index, tl->absolute_index, tl_value_dry);
        VERBOSEF("tr dry: %ld/%ld: %+.14f", tr->array_index, tr->absolute_index, tr_value_dry);
        VERBOSEF("bl dry: %ld/%ld: %+.14f", bl->array_index, bl->absolute_index, bl_value_dry);
        VERBOSEF("br dry: %ld/%ld: %+.14f", br->array_index, br->absolute_index, br_value_dry);

        correction.wet = interpolate(interpolate(tl_value_wet, bl_value_wet, dx),
                                     interpolate(tr_value_wet, br_value_wet, dx), dy);
        correction.dry = interpolate(interpolate(tl_value_dry, bl_value_dry, dx),
                                     interpolate(tr_value_dry, br_value_dry, dx), dy);

        VERBOSEF("tropospheric wet: %+.14f", correction.wet);
        VERBOSEF("tropospheric dry: %+.14f", correction.dry);

        // Slope of the bilinear interpolation, per metre north and east. dx runs from the top to
        // the bottom row (latitude), dy from the left to the right column (longitude).
        {
            auto lat_span = br->position.x - tl->position.x;  // [deg]
            auto lon_span = br->position.y - tl->position.y;  // [deg]
            auto cos_lat  = std::cos(llh.x);
            if (lat_span != 0.0 && lon_span != 0.0 && cos_lat > 1e-6) {
                // mean Earth radius
                constexpr double EARTH_RADIUS = 6371000.0;
                auto             deg_to_m_n   = EARTH_RADIUS * constant::DEG2RAD;
                auto             deg_to_m_e   = EARTH_RADIUS * cos_lat * constant::DEG2RAD;

                auto d_dx = [&](double tl_v, double tr_v, double bl_v, double br_v) {
                    return (1.0 - dy) * (bl_v - tl_v) + dy * (br_v - tr_v);
                };
                auto d_dy = [&](double tl_v, double tr_v, double bl_v, double br_v) {
                    return (1.0 - dx) * (tr_v - tl_v) + dx * (br_v - bl_v);
                };

                correction.dry_north =
                    d_dx(tl_value_dry, tr_value_dry, bl_value_dry, br_value_dry) / lat_span /
                    deg_to_m_n;
                correction.dry_east = d_dy(tl_value_dry, tr_value_dry, bl_value_dry, br_value_dry) /
                                      lon_span / deg_to_m_e;
                correction.wet_north =
                    d_dx(tl_value_wet, tr_value_wet, bl_value_wet, br_value_wet) / lat_span /
                    deg_to_m_n;
                correction.wet_east = d_dy(tl_value_wet, tr_value_wet, bl_value_wet, br_value_wet) /
                                      lon_span / deg_to_m_e;
                correction.has_gradient = true;
                VERBOSEF("tropospheric gradient dry: N %+.4e E %+.4e, wet: N %+.4e E %+.4e m/m",
                         correction.dry_north, correction.dry_east, correction.wet_north,
                         correction.wet_east);
            }
        }

        return GridStatus::Success;
    }

    return GridStatus::PositionOutsideGrid;
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
