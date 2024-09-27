#include "data.hpp"
#include "decode.hpp"
#include "generator.hpp"
#include "message.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <GNSS-ID.h>
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <GridElement-r16.h>
#include <STEC-ResidualSatElement-r16.h>
#include <STEC-ResidualSatList-r16.h>
#include <STEC-SatElement-r16.h>
#include <STEC-SatList-r16.h>
#include <TropospericDelayCorrection-r16.h>
#pragma GCC diagnostic pop

#include <algorithm>
#include <cmath>

namespace generator {
namespace spartn {

void CorrectionPointSet::calculate_grid_points() {
    internal_grid_points.clear();

    long relative_id = 0;
    for (long x = 0; x < numberOfStepsLatitude_r16 + 1; x++) {
        for (long y = 0; y < numberOfStepsLongitude_r16 + 1; y++) {
            auto      i = x * (numberOfStepsLongitude_r16 + 1) + y;
            GridPoint grid_point{};
            grid_point.id        = relative_id;
            grid_point.latitude  = reference_point_latitude - latitude_delta * x;
            grid_point.longitude = reference_point_longitude + longitude_delta * y;
            if (has_grid_point(i)) {
                relative_id++;
            } else {
                grid_point.id = -1;
            }
            internal_grid_points.push_back(grid_point);
        }
    }
}

std::vector<GridPoint> const& CorrectionPointSet::grid_points() const {
    return internal_grid_points;
}

uint32_t HpacSatellite::prn() const {
    // NOTE(ewasjon): 3GPP LPP defines PRN starting at 0 instead of 1.
    return static_cast<uint32_t>(id + 1);
}

void HpacSatellite::add_correction(STEC_SatElement_r16* new_stec) {
    if (!new_stec) return;
    stec = new_stec;
}

void HpacSatellite::add_correction(long grid_id, STEC_ResidualSatElement_r16* residual) {
    if (!residual) return;
    residuals[grid_id] = residual;
}

bool HpacSatellite::has_all_residuals(CorrectionPointSet const& cps) const {
    for (auto gp : cps.grid_points()) {
        if (residuals.find(gp.id) == residuals.end()) {
            return false;
        }
    }

    return true;
}

std::vector<HpacSatellite> HpacCorrections::satellites() const {
    std::unordered_map<long, HpacSatellite> satellites;

    if (stec) {
        auto& list = stec->stec_SatList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            auto  id        = element->svID_r16.satellite_id;
            auto& satellite = satellites[id];
            satellite.id    = id;
            satellite.add_correction(element);
        }
    }

    if (gridded) {
        // NOTE(ewasjon): This grid id (grid-point id) is not the absolute id. To get the absolute
        // id you need to reference the correction point set bitmask.
        long  grid_id = 0;
        auto& list    = gridded->gridList_r16.list;
        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (element) {
                if (element->stec_ResidualSatList_r16) {
                    auto& sat_list = element->stec_ResidualSatList_r16->list;
                    for (int j = 0; j < sat_list.count; j++) {
                        auto sat_element = sat_list.array[j];
                        if (!sat_element) continue;

                        auto  id        = sat_element->svID_r16.satellite_id;
                        auto& satellite = satellites[id];
                        satellite.id    = id;
                        satellite.add_correction(grid_id, sat_element);
                    }
                }
            }

            grid_id++;
        }
    }

    // Convert to vector
    std::vector<HpacSatellite> result;
    for (auto& kv : satellites) {
        result.push_back(kv.second);
    }

    // Sort by satellite id
    std::sort(result.begin(), result.end(), [](HpacSatellite const& a, HpacSatellite const& b) {
        return a.id < b.id;
    });

    return result;
}

GridElement_r16* HpacCorrections::find_grid_point(long grid_id) const {
    if (!gridded) return nullptr;
    auto& list = gridded->gridList_r16.list;
    if (grid_id < 0 || grid_id >= list.count) return nullptr;
    return list.array[grid_id];
}

void HpacData::set_ids(std::vector<uint16_t>& ids) const {
    for (auto& kvp : mKeyedCorrections) {
        auto set_id = kvp.first.set_id;
        if (std::find(ids.begin(), ids.end(), set_id) == ids.end()) {
            ids.push_back(set_id);
        }
    }
}

std::vector<uint16_t> CorrectionData::set_ids() const {
    std::vector<uint16_t> set_ids;
    for (auto& kvp : mHpacData) {
        kvp.second.set_ids(set_ids);
    }

    return set_ids;
}

std::vector<uint16_t> CorrectionData::iods() const {
    std::vector<uint16_t> iods;
    for (auto& kvp : mOcbData) {
        auto iod = kvp.first;
        if (std::find(iods.begin(), iods.end(), iod) == iods.end()) {
            iods.push_back(iod);
        }
    }

    for (auto& kvp : mHpacData) {
        auto iod = kvp.first;
        if (std::find(iods.begin(), iods.end(), iod) == iods.end()) {
            iods.push_back(iod);
        }
    }

    std::sort(iods.begin(), iods.end());
    return iods;
}

bool CorrectionData::find_gad_epoch_time(uint16_t iod, SpartnTime* epoch_time) const {
    SpartnTime time{};
    bool       found = false;

    auto hpac = mHpacData.find(iod);
    if (hpac != mHpacData.end()) {
        auto& hpac_data = hpac->second;
        for (auto& kvp : hpac_data.mKeyedCorrections) {
            auto& corrections = kvp.second;
            // NOTE(ewasjon): The GAD message is constellation-less, thus the epoch time that will
            // be used should be GPS time.
            if (corrections.gnss_id != 0) continue;
            if (!found || corrections.epoch_time > time) {
                time  = corrections.epoch_time;
                found = true;
            }
        }
    }

    if (found && epoch_time) {
        *epoch_time = time;
    }

    return found;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16* gridded) {
    if (!gridded) return;
    auto  iod  = static_cast<uint16_t>(gridded->iod_ssr_r16);
    auto& hpac = mHpacData[iod];

    auto epoch_time = spartn_time_from(gridded->epochTime_r16);
    auto set_id     = static_cast<uint16_t>(gridded->correctionPointSetID_r16);
    auto key        = HpacKey{set_id, gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections      = hpac.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.set_id     = set_id;
    corrections.epoch_time = epoch_time;
    corrections.gridded    = gridded;
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16* stec) {
    if (!stec) return;
    auto  iod  = static_cast<uint16_t>(stec->iod_ssr_r16);
    auto& hpac = mHpacData[iod];

    auto epoch_time = spartn_time_from(stec->epochTime_r16);
    auto set_id     = static_cast<uint16_t>(stec->correctionPointSetID_r16);
    auto key        = HpacKey{set_id, gnss_id, mGroupByEpochTime ? epoch_time.rounded_seconds : 0};

    auto& corrections      = hpac.mKeyedCorrections[key];
    corrections.gnss_id    = gnss_id;
    corrections.iod        = iod;
    corrections.set_id     = set_id;
    corrections.epoch_time = epoch_time;
    corrections.stec       = stec;
}

static bool within_range(double min, double max, double value) {
    if (value < min) return false;
    if (value > max) return false;
    return true;
}

struct StecParameters {
    int    equation_type;
    double c00;
    double c01;
    double c10;
    double c11;

    double reference_x;
    double reference_y;

    static double compute_residual(double x, double y, double c00, double c01, double c10,
                                   double c11, double rx, double ry) {
        return c00 + c01 * (x - rx) + c10 * (y - ry) + c11 * (x - rx) * (y - ry);
    }

    inline double evaluate_at_point(double x, double y) {
        return compute_residual(x, y, c00, c01, c10, c11, reference_x, reference_y);
    }
};

static bool compute_coefficient_size_indicator(StecParameters& parameters) {
    auto small_coefficient_size = true;

    // NOTE(ewasjon): There is no resolution difference between the small and large coefficent
    // blocks. Thus, we only need to check that we're inside the given intervals.
    if (parameters.equation_type >= 0) {
        if (!within_range(-81.88, 81.88, parameters.c00)) small_coefficient_size = false;
    }

    if (parameters.equation_type >= 1) {
        if (!within_range(-16.376, 16.376, parameters.c01)) small_coefficient_size = false;
        if (!within_range(-16.376, 16.376, parameters.c10)) small_coefficient_size = false;
    }

    if (parameters.equation_type >= 2) {
        if (!within_range(-8.190, 8.190, parameters.c11)) small_coefficient_size = false;
    }

    return !small_coefficient_size;
}

// LPP uses the first (north-west) grid point as the reference point for the ionosphereic delay
// polynomial. However, SPARTN uses the mid-point of the grid as the reference point. This means
// that all terms (except C00) will be incorrect.
static StecParameters compute_stec_parameters(CorrectionPointSet const&  correction_point_set,
                                              STEC_SatElement_r16 const& satellite,
                                              int equation_type, bool stec_transform,
                                              bool sign_flip_c00, bool sign_flip_c01,
                                              bool sign_flip_c10, bool sign_flip_c11) {
    auto c00 = decode::stec_C00_r16(satellite.stec_C00_r16);
    auto c01 = decode::stec_C01_r16(satellite.stec_C01_r16);
    auto c10 = decode::stec_C10_r16(satellite.stec_C10_r16);
    auto c11 = decode::stec_C11_r16(satellite.stec_C11_r16);

    if (sign_flip_c00) c00 = -c00;
    if (sign_flip_c01) c01 = -c01;
    if (sign_flip_c10) c10 = -c10;
    if (sign_flip_c11) c11 = -c11;

    auto c00_prime = c00;
    auto c01_prime = c01;
    auto c10_prime = c10;
    auto c11_prime = c11;

    auto x0 = correction_point_set.reference_point_latitude;
    auto y0 = correction_point_set.reference_point_longitude;
    auto dx = correction_point_set.latitude_grid_spacing();
    auto dy = correction_point_set.longitude_grid_spacing();
    auto hx = dx / 2;
    auto hy = dy / 2;
    auto mx = x0 - hx;
    auto my = y0 + hy;

    auto rx_prime = x0;
    auto ry_prime = y0;

    if (stec_transform) {
        // r = C00 + C01 * (x - x0) +
        //           C10 * (y - y0) +
        //           C11 * (x - x0) * (y - y0)
        // r = C00 + C01 * (x - mx - hx) +
        //           C10 * (y - my + hy) +
        //           C11 * (x - mx - hx) * (y - my + hy)
        // r = C00 + C01 * (-hx) +
        //           C01 * (x - mx) +
        //           C10 * (hy) +
        //           C10 * (y - my) +
        //           C11 * (x - mx) * (y - my + hy) +
        //           C11 * (-hx)  * (y - my + hy)
        // r = C00 + C01 * (-hx) + C10 * (hy) +
        //           C01 * (x - mx) +
        //           C10 * (y - my) +
        //           C11 * (x - mx) * (y - my) +
        //           C11 * (x - mx) * hy +
        //           C11 * (-hx) * (y - my) +
        //           C11 * (-hx) * (hy)
        // C00' = C00 + C01 * (-hx) + C10 * (hy) + C11 * (-hx) * (hy)
        // C01' = C01 + C11 * hy
        // C10' = C10 + C11 * hx
        // C11' = C11

        c00_prime = c00 - c01 * hx + c10 * hy - c11 * hx * hy;
        c01_prime = c01 + c11 * hy;
        c10_prime = c10 - c11 * hx;
        c11_prime = c11;

        rx_prime = mx;
        ry_prime = my;
#if SPARTN_DEBUG_PRINT
        printf("stec polynomial (%d):\n", equation_type);
        printf("  C00: %+.6f -> %+.6f\n", c00, c00_prime);
        if (equation_type >= 1) {
            printf("  C01: %+.6f -> %+.6f\n", c01, c01_prime);
            printf("  C10: %+.6f -> %+.6f\n", c10, c10_prime);
        }
        if (equation_type >= 2) {
            printf("  C11: %+.6f -> %+.6f\n", c11, c11_prime);
        }
#endif
        if (equation_type == 1 || equation_type == 2) {
#if SPARTN_DEBUG_PRINT
            for (auto gp : correction_point_set.grid_points()) {
                auto latitude  = gp.latitude;
                auto longitude = gp.longitude;
                auto lpp_r    = StecParameters::compute_residual(latitude, longitude, c00, c01, c10,
                                                                 c11, x0, y0);
                auto spartn_r = StecParameters::compute_residual(
                    latitude, longitude, c00_prime, c01_prime, c10_prime, c11_prime, mx, my);
                auto incorrect_r = StecParameters::compute_residual(latitude, longitude, c00, c01,
                                                                    c10, c11, mx, my);
                printf("    stec[%2ld] = lpp: %+9.6f, spartn: %+9.6f (%+9.6f), incorrect: %+9.6f "
                       "(%+9.6f)  (%.4f, %.4f)\n",
                       gp.id, lpp_r, spartn_r, lpp_r - spartn_r, incorrect_r, incorrect_r - lpp_r,
                       latitude, longitude);
            }
#endif
        }
    }

    return StecParameters{equation_type, c00_prime, c01_prime, c10_prime,
                          c11_prime,     rx_prime,  ry_prime};
}

static uint8_t compute_troposphere_block_type(CorrectionPointSet const& correction_point_set,
                                              HpacCorrections const&    corrections) {
    // If we're missing the troposphere correction, return 0=none
    if (!corrections.gridded) return 0;

    // NOTE(ewasjon): SPARTN supports three types of troposphere corrections:
    // 0 = none
    // 1 = polynomial
    // 2 = polynomial + grid
    //
    // Because 3GPP LPP doesn't have a polynomial, only values for each grid point, we can only
    // support type 0 and 2.

    // The tropospheric parameters can be shared between GNSS systems, which can mean that there
    // are cases where the tropospheric parameters are not available. Thus, only include it if we
    // at least have one grid point with tropospheric parameters.
    auto tropospheric_count = 0;
    auto grid_points        = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        auto element = corrections.find_grid_point(gp.id);
        if (element && element->tropospericDelayCorrection_r16) {
            tropospheric_count++;
        }
    }

    if (tropospheric_count == 0) {
        return 0;
    } else {
        return 2;
    }
}

static uint8_t compute_ionosphere_block_type(GNSS_SSR_STEC_Correction_r16 const*   stec,
                                             GNSS_SSR_GriddedCorrection_r16 const* gridded) {
    // If we're missing the ionosphere correction, return 0=none
    if (!stec) return 0;

    // We have a gridded correction, so we use both the polynomial and the grid
    if (gridded) {
        return 2;
    }

    // If we're missing the gridded correction, we only use the polynomial
    // for the ionosphere correction.
    return 1;
}

static double compute_average_zentith_delay(CorrectionPointSet& correction_point_set,
                                            HpacCorrections&    corrections) {
    double total_zenith_delay = 0.0;
    double count              = 0.0;

    // TODO(ewasjon): This can be done better by calculate the error based on rounding up or down.
    auto grid_points = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        auto element = corrections.find_grid_point(gp.id);
        if (element && element->tropospericDelayCorrection_r16) {
            auto& grid_point = *element->tropospericDelayCorrection_r16;
            auto residual = decode::tropoWetVerticalDelay_r16(grid_point.tropoWetVerticalDelay_r16);
            total_zenith_delay += residual;
            count++;
        }
    }

    if (count > 0.0) {
        auto average_zentith_delay = total_zenith_delay / count;
        return average_zentith_delay;
    } else {
        return 0.0;
    }
}

static double compute_average_hydrostatic_delay(CorrectionPointSet& correction_point_set,
                                                HpacCorrections&    corrections) {
    double total_hydrostatic_delay = 0.0;
    double count                   = 0.0;

    // TODO(ewasjon): This can be done better by calculate the error based on rounding up or down.
    auto grid_points = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        auto element = corrections.find_grid_point(gp.id);
        if (element && element->tropospericDelayCorrection_r16) {
            auto& grid_point        = *element->tropospericDelayCorrection_r16;
            auto  hydrostatic_delay = decode::tropoHydroStaticVerticalDelay_r16(
                grid_point.tropoHydroStaticVerticalDelay_r16);
            total_hydrostatic_delay += hydrostatic_delay;
            count++;
        }
    }

    if (count > 0.0) {
        auto average_hydrostatic_delay = total_hydrostatic_delay / count;
        return average_hydrostatic_delay;
    } else {
        return 0.0;
    }
}

struct TroposphereResidual {
    double residual;
    bool   invalid;
};

static std::vector<TroposphereResidual>
compute_troposphere_residuals(CorrectionPointSet& correction_point_set,
                              HpacCorrections& corrections, double average_zenith_delay,
                              double hydrostatic_delay_avg,
                              bool   add_hydrostatic_residual_to_wet_residual) {
    std::vector<TroposphereResidual> result;

#ifdef SPARTN_DEBUG_PRINT
    printf("  troposphere residuals:\n");
#endif

    auto grid_points = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        auto element = corrections.find_grid_point(gp.id);
        if (element && element->tropospericDelayCorrection_r16) {
            auto& grid_point = *element->tropospericDelayCorrection_r16;

            auto hydrostatic_delay = decode::tropoHydroStaticVerticalDelay_r16(
                grid_point.tropoHydroStaticVerticalDelay_r16);
            auto hydrostatic_residual = hydrostatic_delay - hydrostatic_delay_avg;
            auto wet_delay =
                decode::tropoWetVerticalDelay_r16(grid_point.tropoWetVerticalDelay_r16);
            auto wet_residual = wet_delay - average_zenith_delay;

            double total_residual = 0;
            if (add_hydrostatic_residual_to_wet_residual) {
                total_residual = hydrostatic_residual + wet_residual;
            } else {
                total_residual = wet_residual;
            }

#ifdef SPARTN_DEBUG_PRINT
            printf("    grid[%2ld] = %+.6f\n", gp.id, total_residual);
#endif
            result.push_back({total_residual, false});
        } else {
#ifdef SPARTN_DEBUG_PRINT
            printf("    grid[%2ld] = invalid\n", gp.id);
#endif
            result.push_back({0.0, true});
        }
    }

    return result;
}

static int
compute_troposphere_residual_field_size(std::vector<TroposphereResidual> const& residuals) {
    int residual_field_size = 0;
    for (auto tr : residuals) {
        if (tr.invalid) continue;
        if (within_range(-0.124, 0.124, tr.residual))
            residual_field_size = std::max<uint8_t>(residual_field_size, 0U);
        else
            residual_field_size = std::max<uint8_t>(residual_field_size, 1U);
    }
    return residual_field_size;
}

static void troposphere_data_block(MessageBuilder&     builder,
                                   CorrectionPointSet& correction_point_set,
                                   HpacCorrections& corrections, int sf042_override,
                                   int sf042_default, bool use_average_zenith_delay,
                                   bool calculate_sf051,
                                   bool add_hydrostatic_residual_to_wet_residual) {
    auto& data = *corrections.gridded;

    // NOTE(ewasjon): Use a polynomial of degree 0, as we don't have a polynomial in 3GPP
    // LPP. This will result in a constant value for the troposphere correction.
    builder.sf041(0 /* T_00 */);

    if (sf042_override >= 0) {
        uint8_t value = sf042_override > 7 ? 7 : static_cast<uint8_t>(sf042_override);
#ifdef SPARTN_DEBUG_PRINT
        printf("  sf042: %d [override]\n", value);
#endif
        builder.sf042_raw(value);
    } else if (data.troposphericDelayQualityIndicator_r16) {
        auto quality = decode::troposphericDelayQualityIndicator_r16(
            *data.troposphericDelayQualityIndicator_r16);
        if (quality.invalid) {
            uint8_t value = sf042_default < 0 ?
                                0 :
                                (sf042_default > 7 ? 7 : static_cast<uint8_t>(sf042_default));
            builder.sf042_raw(value);
#ifdef SPARTN_DEBUG_PRINT
            printf("  sf042: %d (%u) [default/invalid]\n", sf042_default, value);
#endif
        } else {
            auto value = builder.sf042(quality.value);
#ifdef SPARTN_DEBUG_PRINT
            printf("  sf042: %f (%u)\n", quality.value, value);
#endif
        }
    } else {
        uint8_t value =
            sf042_default < 0 ? 0 : (sf042_default > 7 ? 7 : static_cast<uint8_t>(sf042_default));
        builder.sf042_raw(value);
#ifdef SPARTN_DEBUG_PRINT
        printf("  sf042: %d (%u) [default/missing]\n", sf042_default, value);
#endif
    }

    // NOTE(ewasjon): SPARTN have an average hydrostatic delay for all grid points. 3GPP LPP only
    // have hydrostatic delay per grid point. Thus, we need to compute the average.
    auto hydrostatic_delay_avg =
        compute_average_hydrostatic_delay(correction_point_set, corrections);
    hydrostatic_delay_avg = builder.sf043(hydrostatic_delay_avg);

#ifdef SPARTN_DEBUG_PRINT
    printf("  hydrostatic_delay_avg: %f\n", hydrostatic_delay_avg);
#endif

    // NOTE(ewasjon): 3GPP LPP doesn't include a polynomial for the wet delay (zenith delay). Thus,
    // we can set this to a constant value of 0.0. We can also compute the average zenith delay for
    // all grid points, and subtract this from the zenith delay for each grid point. This will
    // reduce the precision needed.

    double average_zenith_delay = 0.0;
    if (use_average_zenith_delay) {
        average_zenith_delay = compute_average_zentith_delay(correction_point_set, corrections);
    }

    if (within_range(-0.252, 0.252, average_zenith_delay)) {
        builder.sf044(0);                                            // Small coefficient block
        average_zenith_delay = builder.sf045(average_zenith_delay);  // T_00
    } else if (within_range(-1.020, 1.020, average_zenith_delay)) {
        builder.sf044(1);                                            // Small coefficient block
        average_zenith_delay = builder.sf048(average_zenith_delay);  // T_00
    } else {
        builder.sf044(0);    // Small coefficient block
        builder.sf045(0.0);  // T_0
        average_zenith_delay = 0.0;
    }

#ifdef SPARTN_DEBUG_PRINT
    printf("  average_zenith_delay: %f\n", average_zenith_delay);
#endif

    // Compute the residuals
    auto residuals = compute_troposphere_residuals(correction_point_set, corrections,
                                                   average_zenith_delay, hydrostatic_delay_avg,
                                                   add_hydrostatic_residual_to_wet_residual);

    // Compute the residual field size for all grid points
    auto residual_field_size = 0;
    if (calculate_sf051) {
        residual_field_size = compute_troposphere_residual_field_size(residuals);
    } else {
        // If we're not calculating the residual field size, always use the largest size
        residual_field_size = 1;
    }

    builder.sf051(residual_field_size);

    for (auto tr : residuals) {
        if (tr.invalid) {
            if (residual_field_size == 0)
                builder.sf052_invalid();
            else
                builder.sf053_invalid();
        } else {
            if (residual_field_size == 0)
                builder.sf052(tr.residual);
            else
                builder.sf053(tr.residual);
        }
    }
}

static int compute_equation_type(std::vector<HpacSatellite>& satellites) {
    auto final_equation_type = 0;

    for (auto& satellite : satellites) {
        if (!satellite.stec) continue;
        auto& stec = *satellite.stec;

        auto equation_type = 0;  // C_00
        if (stec.stec_C01_r16 || stec.stec_C10_r16) {
            equation_type = 1;  // C_00, C_01, C_10
        }

        if (stec.stec_C11_r16) {
            equation_type = 2;  // C_00, C_01, C_10, C_11
        }

        if (equation_type > final_equation_type) {
            final_equation_type = equation_type;
        }
    }

    return final_equation_type;
}

static StecParameters
ionosphere_data_block_1(MessageBuilder& builder, CorrectionPointSet& correction_point_set,
                        HpacCorrections& corrections, HpacSatellite const& satellite,
                        int equation_type, int sf055_override, int sf055_default,
                        bool stec_transform, bool sign_flip_c00, bool sign_flip_c01,
                        bool sign_flip_c10, bool sign_flip_c11) {
    auto element = satellite.stec;

    if (sf055_override >= 0) {
        uint8_t value = sf055_override > 15 ? 15 : static_cast<uint8_t>(sf055_override);
        builder.sf055_raw(value);
#ifdef SPARTN_DEBUG_PRINT
        printf("  sf055: %d (%u) [override]\n", sf055_override, value);
#endif
    } else {
        auto q = decode::stecQualityIndicator_r16(element->stecQualityIndicator_r16);
        if (q.invalid) {
            uint8_t value = sf055_default < 0 ?
                                0 :
                                (sf055_default > 15 ? 15 : static_cast<uint8_t>(sf055_default));
            builder.sf055_raw(value);
#ifdef SPARTN_DEBUG_PRINT
            printf("  sf055: %d (%u) [default/invalid]\n", sf055_default, value);
#endif
        } else {
            auto value = builder.sf055(q.value);
#ifdef SPARTN_DEBUG_PRINT
            printf("  sf055: %f (%u)\n", q.value, value);
#endif
        }
    }

    auto stec_parameters =
        compute_stec_parameters(correction_point_set, *element, equation_type, stec_transform,
                                sign_flip_c00, sign_flip_c01, sign_flip_c10, sign_flip_c11);

    auto coefficient_size_indicator = compute_coefficient_size_indicator(stec_parameters);
    builder.sf056(coefficient_size_indicator);

    if (equation_type >= 0) {
        builder.ionosphere_coefficient_c00(coefficient_size_indicator, stec_parameters.c00);
    }

    if (equation_type >= 1) {
        builder.ionosphere_coefficient_c10_c01(coefficient_size_indicator, stec_parameters.c01);
        builder.ionosphere_coefficient_c10_c01(coefficient_size_indicator, stec_parameters.c10);
    }

    if (equation_type >= 2) {
        builder.ionosphere_coefficient_c11(coefficient_size_indicator, stec_parameters.c11);
    }

    return stec_parameters;
}

struct StecResidual {
    long   id;
    double residual;
    bool   invalid;
};

static std::vector<StecResidual> compute_stec_residuals(CorrectionPointSet& correction_point_set,
                                                        HpacSatellite&      satellite,
                                                        StecParameters&     stec_parameters,
                                                        StecMethod          stec_method,
                                                        bool sign_flip_stec_residuals) {
    std::vector<StecResidual> result;

    auto grid_points = correction_point_set.grid_points();
    for (auto gp : grid_points) {
        auto remaining_polynomial_residual = 0.0;
        if (stec_method == StecMethod::MoveToResiduals) {
            auto polynomial_residual = stec_parameters.evaluate_at_point(gp.latitude, gp.longitude);
            auto constant_residual   = stec_parameters.c00;
            remaining_polynomial_residual += (polynomial_residual - constant_residual);
        }

        auto it = satellite.residuals.find(gp.id);
        if (it == satellite.residuals.end()) {
            result.push_back({gp.id, remaining_polynomial_residual, true});
        } else {
            auto& element  = *it->second;
            auto  residual = decode::stecResidualCorrection_r16(element.stecResidualCorrection_r16);
            if (sign_flip_stec_residuals) residual = -residual;
            result.push_back({gp.id, residual + remaining_polynomial_residual, false});
        }
    }

    return result;
}

static uint8_t compute_residual_field_size(std::vector<StecResidual> const& residuals) {
    uint8_t residual_field_size = 0;
    for (auto& r : residuals) {
        auto residual = r.residual;
        if (within_range(-0.28, 0.28, residual))
            residual_field_size = std::max<uint8_t>(residual_field_size, 0U);
        else if (within_range(-2.52, 2.52, residual))
            residual_field_size = std::max<uint8_t>(residual_field_size, 1U);
        else if (within_range(-20.44, 20.44, residual))
            residual_field_size = std::max<uint8_t>(residual_field_size, 2U);
        else
            residual_field_size = std::max<uint8_t>(residual_field_size, 3U);
    }

    return residual_field_size;
}

static void ionosphere_data_block_2(MessageBuilder&     builder,
                                    CorrectionPointSet& correction_point_set,
                                    HpacSatellite& satellite, StecMethod stec_method,
                                    StecParameters stec_parameters, bool stec_invalid_to_zero,
                                    bool sign_flip_stec_residuals) {
    auto residuals = compute_stec_residuals(correction_point_set, satellite, stec_parameters,
                                            stec_method, sign_flip_stec_residuals);
    auto residual_field_size = compute_residual_field_size(residuals);
    builder.sf063(residual_field_size);

#ifdef SPARTN_DEBUG_PRINT
    printf("  residual_field_size=%d\n", residual_field_size);
#endif

    for (auto r : residuals) {
        // NOTE(ewasjon): If the residual is invalid and stec_invalid_to_zero is true, we want to
        // set the residual to 0.0. But if stec_method == StecMethod::MoveToResiduals, we want to
        // keep the polynomial residual, thus we use the r.residual value even if it's invalid.
        if (r.invalid && !stec_invalid_to_zero) {
            builder.ionosphere_residual_invalid(residual_field_size);
#ifdef SPARTN_DEBUG_PRINT
            printf("    grid[%2ld] = invalid\n", r.id);
#endif
        } else {
            auto residual         = r.residual;
            auto encoded_residual = builder.ionosphere_residual(residual_field_size, residual);
#ifdef SPARTN_DEBUG_PRINT
            printf("    grid[%2ld] = %+.4f (%+.4f)%s\n", r.id, encoded_residual,
                   residual - encoded_residual, r.invalid ? " [invalid]" : "");
#endif
        }
    }
}

static void ionosphere_data_block(MessageBuilder& builder, CorrectionPointSet& correction_point_set,
                                  HpacCorrections& corrections, long gnss_id, OcbCorrections* ocb,
                                  int ionosphere_block_type, int sf055_override, int sf055_default,
                                  StecMethod stec_method, bool stec_transform,
                                  bool filter_by_residuals, bool stec_invalid_to_zero,
                                  bool sign_flip_c00, bool sign_flip_c01, bool sign_flip_c10,
                                  bool sign_flip_c11, bool sign_flip_stec_residuals) {
    auto satellites = corrections.satellites();

    {
        // NOTE(ewasjon): Remove all satellites that does not have STEC corrections.
        // TODO(ewasjon): [low-priority] Add _WHY_ we're removing satellites without STEC
        auto it = satellites.begin();
        for (; it != satellites.end();) {
            if (!it->stec) {
#ifdef SPARTN_DEBUG_PRINT
                printf("  removed satellite=%u [stec]\n", it->prn());
#endif
                it = satellites.erase(it);
            } else if (ocb && !ocb->has_satellite(it->id)) {
#ifdef SPARTN_DEBUG_PRINT
                printf("  removed satellite=%u [ocb]\n", it->prn());
#endif
                it = satellites.erase(it);
            } else if (filter_by_residuals && !it->has_all_residuals(correction_point_set)) {
#ifdef SPARTN_DEBUG_PRINT
                printf("  removed satellite=%u [residuals]\n", it->prn());
#endif
                it = satellites.erase(it);
            } else {
                it++;
            }
        }
    }

    auto equation_type = compute_equation_type(satellites);
    switch (stec_method) {
    case StecMethod::Default: break;
    case StecMethod::DiscardC01C10C11:
    case StecMethod::MoveToResiduals: equation_type = 0; break;
    }

    builder.sf054(equation_type);
    builder.satellite_mask(gnss_id, satellites);

    for (auto& satellite : satellites) {
#ifdef SPARTN_DEBUG_PRINT
        assert(satellite.stec);
        printf("  satellite=%u\n", satellite.prn());
#endif

        auto stec_parameters =
            ionosphere_data_block_1(builder, correction_point_set, corrections, satellite,
                                    equation_type, sf055_override, sf055_default, stec_transform,
                                    sign_flip_c00, sign_flip_c01, sign_flip_c10, sign_flip_c11);

        if (ionosphere_block_type == 2) {
            ionosphere_data_block_2(builder, correction_point_set, satellite, stec_method,
                                    stec_parameters, stec_invalid_to_zero,
                                    sign_flip_stec_residuals);
        }
    }
}

void Generator::generate_hpac(uint16_t iod) {
    auto hpac_data = mCorrectionData->hpac(iod);
    if (!hpac_data) return;

    auto ocb_data = mCorrectionData->ocb(iod);

    std::vector<HpacCorrections*> messages;
    for (auto& kvp : hpac_data->mKeyedCorrections) {
        if (!mGpsSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_gps) continue;
        if (!mGlonassSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_glonass) continue;
        if (!mGalileoSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_galileo) continue;
        if (!mBeidouSupported && kvp.first.gnss_id == GNSS_ID__gnss_id_bds) continue;

        messages.push_back(&kvp.second);
    }

    std::sort(messages.begin(), messages.end(),
              [](HpacCorrections const* a, HpacCorrections const* b) {
                  return subtype_from_gnss_id(a->gnss_id) < subtype_from_gnss_id(b->gnss_id);
              });

    for (size_t message_id = 0; message_id < messages.size(); message_id++) {
        auto& corrections = *messages[message_id];
        auto  epoch_time  = corrections.epoch_time.rounded_seconds;
        auto  gnss_id     = corrections.gnss_id;
        auto  set_id      = corrections.set_id;

        auto cps_it = mCorrectionPointSets.find(set_id);
        if (cps_it == mCorrectionPointSets.end()) continue;
        auto& correction_point_set = *(cps_it->second.get());

        OcbCorrections* ocb_corrections = nullptr;
        if (ocb_data && mFilterByOcb) {
            auto ocb_key = OcbKey{gnss_id, epoch_time};
            if (!mGroupByEpochTime) {
                ocb_key.epoch_time = 0;
            }
            auto ocb_it = ocb_data->mKeyedCorrections.find(ocb_key);
            if (ocb_it != ocb_data->mKeyedCorrections.end()) {
                ocb_corrections = &ocb_it->second;
            }
        }

#ifdef SPARTN_DEBUG_PRINT
        printf("HPAC: time=%u, set=%hu, gnss=%ld, iod=%hu\n", epoch_time, set_id, gnss_id, iod);
        printf("  area_id=%u\n", correction_point_set.area_id);
#endif

        auto subtype = subtype_from_gnss_id(gnss_id);
        auto troposphere_block_type =
            compute_troposphere_block_type(correction_point_set, corrections);
        auto ionosphere_block_type =
            compute_ionosphere_block_type(corrections.stec, corrections.gridded);

        auto siou = iod;
        if (mIncreasingSiou) {
            siou = mSiouIndex;
        }

        MessageBuilder builder{1 /* HPAC */, subtype, epoch_time};
        builder.sf005(siou);
        builder.sf068(0);  // TODO(ewasjon): [low-priority] We could include AIOU in the
                           // correction point set, to handle overflow
        builder.sf069();
        builder.sf030(1);

        // Atmosphere block
        {
            // Area data block
            {
                builder.sf031(static_cast<uint8_t>(correction_point_set.area_id));
                builder.sf039(static_cast<uint8_t>(correction_point_set.grid_point_count));
                builder.sf040(troposphere_block_type);
                builder.sf040(ionosphere_block_type);
            }

            // Troposphere data block
            if (troposphere_block_type != 0) {
                // TODO(ewasjon): [low-priority] Expose this as a option
                auto calculate_sf051 = mComputeAverageZenithDelay;
                troposphere_data_block(builder, correction_point_set, corrections, mSf042Override,
                                       mSf042Default, mComputeAverageZenithDelay, calculate_sf051,
                                       mHydrostaticResidualInZenith);
            }

            // Ionosphere data block
            if (ionosphere_block_type != 0) {
                ionosphere_data_block(builder, correction_point_set, corrections, gnss_id,
                                      ocb_corrections, ionosphere_block_type, mSf055Override,
                                      mSf055Default, mStecMethod, mStecTranform, mFilterByResiduals,
                                      mStecInvalidToZero, mSignFlipC00, mSignFlipC01, mSignFlipC10,
                                      mSignFlipC11, mSignFlipStecResiduals);
            }
        }

        mMessages.push_back(builder.build());
    }
}

}  // namespace spartn
}  // namespace generator
