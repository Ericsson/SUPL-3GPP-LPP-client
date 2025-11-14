#include "grid.hpp"
#include "constant.hpp"
#include "decode.hpp"
#include "ionosphere.hpp"
#include "troposphere.hpp"

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
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
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

GridPoint const* GridData::find_top_left(Float3 llh) const NOEXCEPT {
    FUNCTION_SCOPE();

    for (auto& grid_point : grid_points) {
        if (!grid_point.valid) {
            continue;
        }

        auto x0 = grid_point.position.x;
        auto y0 = grid_point.position.y;
        auto x1 = x0 - delta_latitude;
        auto y1 = y0 + delta_longitude;
        VERBOSEF("latitude:  %+18.14f >= %+18.14f >= %+18.14f", x0, llh.x * constant::RAD2DEG, x1);
        VERBOSEF("longitude: %+18.14f <= %+18.14f <= %+18.14f", y0, llh.y * constant::RAD2DEG, y1);
        if (llh.x * constant::RAD2DEG <= x0 && llh.x * constant::RAD2DEG >= x1 &&
            llh.y * constant::RAD2DEG >= y0 && llh.y * constant::RAD2DEG <= y1) {
            VERBOSEF("found: %ld/%ld", grid_point.array_index, grid_point.absolute_index);
            return &grid_point;
        }
    }

    VERBOSEF("top left not found");
    return nullptr;
}

GridPoint const* GridData::find_with_absolute_index(long absolute_index) const NOEXCEPT {
    FUNCTION_SCOPE();

    for (auto& grid_point : grid_points) {
        if (!grid_point.valid) {
            continue;
        }

        if (grid_point.absolute_index == absolute_index) {
            return &grid_point;
        }
    }

    VERBOSEF("absolute index not found: %ld", absolute_index);
    return nullptr;
}

bool GridData::find_4_points(Float3 llh, GridPoint const*& tl, GridPoint const*& tr,
                             GridPoint const*& bl, GridPoint const*& br) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto top_left = find_top_left(llh);
    if (top_left == nullptr) {
        VERBOSEF("top left not found");
        return false;
    }

    auto top_right = find_with_absolute_index(top_left->absolute_index + 1);
    auto bottom_left =
        find_with_absolute_index(top_left->absolute_index + (number_of_steps_longitude + 1));
    auto bottom_right =
        find_with_absolute_index(top_left->absolute_index + (number_of_steps_longitude + 1) + 1);
    if (top_right == nullptr || bottom_left == nullptr || bottom_right == nullptr) {
        VERBOSEF("4 points not found");
        return false;
    }

    tl = top_left;
    tr = top_right;
    bl = bottom_left;
    br = bottom_right;
    return true;
}

GridData::GridStatus GridData::ionospheric(SatelliteId sv_id, Float3 llh,
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

GridData::GridStatus GridData::tropospheric(Float3                  llh,
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
