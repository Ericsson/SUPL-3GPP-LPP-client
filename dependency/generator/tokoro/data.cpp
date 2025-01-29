#include "data.hpp"
#include "constant.hpp"
#include "decode.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
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
#pragma GCC diagnostic pop

#include <asn.1/bit_string.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#define LOGLET_CURRENT_MODULE "tokoro/data"

namespace generator {
namespace tokoro {

bool OrbitCorrection::correction(ts::Tai time, Float3 eph_position, Float3 eph_velocity,
                                 Float3& result, Float3* output_radial, Float3* output_along,
                                 Float3* output_cross, double* output_delta) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto e_along = eph_velocity;
    if (!e_along.normalize()) {
        WARNF("failed to normalize e_along");
        return false;
    }
    VERBOSEF("e_along:   %+24.14f, %+24.14f, %+24.14f", e_along.x, e_along.y, e_along.z);

    auto e_cross = cross_product(eph_position, eph_velocity);
    if (!e_cross.normalize()) {
        WARNF("failed to normalize e_cross");
        return false;
    }
    VERBOSEF("e_cross:   %+24.14f, %+24.14f, %+24.14f", e_cross.x, e_cross.y, e_cross.z);

    auto e_radial = cross_product(e_along, e_cross);
    VERBOSEF("e_radial:  %+24.14f, %+24.14f, %+24.14f", e_radial.x, e_radial.y, e_radial.z);

    VERBOSEF("t:   %s", time.rtklib_time_string().c_str());
    VERBOSEF("t0:  %s", reference_time.rtklib_time_string().c_str());

    auto t_k = time.difference_seconds(reference_time);
    VERBOSEF("t_k: %+.14f", t_k);

    auto delta_at = delta + dot_delta * t_k;
    VERBOSEF("delta:     %+24.14f, %+24.14f, %+24.14f", delta.x, delta.y, delta.z);
    VERBOSEF("dot_delta: %+24.14f, %+24.14f, %+24.14f", dot_delta.x, dot_delta.y, dot_delta.z);
    VERBOSEF("delta_at:  %+24.14f, %+24.14f, %+24.14f", delta_at.x, delta_at.y, delta_at.z);

    auto x = e_radial.x * delta_at.x + e_along.x * delta_at.y + e_cross.x * delta_at.z;
    auto y = e_radial.y * delta_at.x + e_along.y * delta_at.y + e_cross.y * delta_at.z;
    auto z = e_radial.z * delta_at.x + e_along.z * delta_at.y + e_cross.z * delta_at.z;

    VERBOSEF("result:    %+24.14f, %+24.14f, %+24.14f", x, y, z);

    if (output_radial) *output_radial = e_radial;
    if (output_along) *output_along = e_along;
    if (output_cross) *output_cross = e_cross;
    if (output_delta) *output_delta = t_k;

    result = eph_position - Float3{x, y, z};
    return true;
}

double ClockCorrection::correction(ts::Tai time) const NOEXCEPT {
    FUNCTION_SCOPE();

    VERBOSEF("t:   %s", ts::Utc{time}.rtklib_time_string().c_str());
    VERBOSEF("t0:  %s", ts::Utc{reference_time}.rtklib_time_string().c_str());

    auto t_k = ts::Gps{time}.difference(ts::Gps{reference_time}).full_seconds();
    VERBOSEF("t_k: %+.14f", t_k);

    VERBOSEF("c:      %+24.14f, %+24.14f, %+24.14f", c0, c1, c2);

    auto r0 = c0;
    auto r1 = c1 * t_k;
    auto r2 = c2 * t_k * t_k;
    VERBOSEF("parts:  %+24.14f, %+24.14f, %+24.14f", r0, r1, r2);

    auto result = r0 + r1 + r2;
    VERBOSEF("result: %+24.14f", result);
    return result;
}

bool CorrectionPointSet::array_to_index(long array_index, long& index, bool& valid,
                                        Float3& llh) const NOEXCEPT {
    long array_count    = 0;
    long absolute_index = 0;
    for (long y = 0; y <= number_of_steps_latitude; y++) {
        for (long x = 0; x <= number_of_steps_longitude; x++) {
            auto i        = y * (number_of_steps_latitude + 1) + x;
            auto bit      = 1ULL << (64 - 1 - i);
            auto is_valid = (bitmask & bit) != 0;

            if (array_count == array_index) {
                index = absolute_index;
                valid = is_valid;
                llh.x = reference_point_latitude - static_cast<double>(y) * step_of_latitude;
                llh.y = reference_point_longitude + static_cast<double>(x) * step_of_longitude;
                llh.z = 0;
                return true;
            }

            // only valid grid points are included in the array
            if (is_valid) {
                array_count++;
            }
            absolute_index++;
        }
    }

    return false;
}

static double interpolate(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

GridPoint const* GridData::find_top_left(Float3 llh) const NOEXCEPT {
    FUNCTION_SCOPE();

    for (auto& grid_point : mGridPoints) {
        if (!grid_point.valid) {
            continue;
        }

        auto x0 = grid_point.position.x;
        auto y0 = grid_point.position.y;
        auto x1 = x0 - mDeltaLatitude;
        auto y1 = y0 + mDeltaLongitude;
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

    for (auto& grid_point : mGridPoints) {
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
        find_with_absolute_index(top_left->absolute_index + (mNumberOfStepsLongitude + 1));
    auto bottom_right =
        find_with_absolute_index(top_left->absolute_index + (mNumberOfStepsLongitude + 1) + 1);
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

bool GridData::ionospheric(SatelliteId sv_id, Float3 llh,
                           double& ionospheric_residual) const NOEXCEPT {
    FUNCTION_SCOPE();

    // if we're inside 4 points, bilinear interpolation
    GridPoint const* tl = nullptr;
    GridPoint const* tr = nullptr;
    GridPoint const* bl = nullptr;
    GridPoint const* br = nullptr;
    if (find_4_points(llh, tl, tr, bl, br)) {
        VERBOSEF("bilinear interpolation");

        if (!tl->has_ionospheric_residual(sv_id) || !tr->has_ionospheric_residual(sv_id) ||
            !bl->has_ionospheric_residual(sv_id) || !br->has_ionospheric_residual(sv_id)) {
            VERBOSEF("ionospheric correction not found");
            return false;
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

        ionospheric_residual = interpolate(interpolate(tl_value, bl_value, dx),
                                           interpolate(tr_value, br_value, dx), dy);
        VERBOSEF("ionospheric: %+.14f", ionospheric_residual);
        return true;
    }

    // TODO(ewasjon): support other interpolation methods
    return false;
}

bool GridData::tropospheric(Float3 llh, TroposphericCorrection& correction) const NOEXCEPT {
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
            return false;
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

        return true;
    }

    return false;
}

bool CorrectionData::tropospheric(SatelliteId sv_id, Float3 llh,
                                  TroposphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    if (mCorrectionPointSet == nullptr) {
        WARNF("tropospheric correction grid not available");
        return false;
    }

    auto grid_it = mGrid.find(sv_id.gnss());
    if (grid_it == mGrid.end()) {
        WARNF("tropospheric correction for satellite not found");
        return false;
    }

    auto& grid = grid_it->second;
    if (!grid.tropospheric(llh, correction)) {
        VERBOSEF("tropospheric correction not found");
        return false;
    }

    return true;
}

bool CorrectionData::ionospheric(SatelliteId sv_id, Float3 llh,
                                 IonosphericCorrection& correction) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto has_polynomial = false;
    auto has_gridded    = false;

    correction = {};

    auto it = mIonosphericPolynomial.find(sv_id);
    if (it != mIonosphericPolynomial.end()) {
        auto& polynomial = it->second;
        auto  latitude   = (llh.x * constant::RAD2DEG) - polynomial.reference_point_latitude;
        auto  longitude  = (llh.y * constant::RAD2DEG) - polynomial.reference_point_longitude;

        VERBOSEF("polynomial:");
        VERBOSEF("  c00: %.14f", polynomial.c00);
        VERBOSEF("  c01: %.14f", polynomial.c01);
        VERBOSEF("  c10: %.14f", polynomial.c10);
        VERBOSEF("  c11: %.14f", polynomial.c11);

        VERBOSEF("  px: %.14f", llh.x * constant::RAD2DEG);
        VERBOSEF("  rx: %.14f", polynomial.reference_point_latitude);
        VERBOSEF("  dx: %.14f", latitude);

        VERBOSEF("  py: %.14f", llh.y * constant::RAD2DEG);
        VERBOSEF("  ry: %.14f", polynomial.reference_point_longitude);
        VERBOSEF("  dy: %.14f", longitude);

        auto c00 = polynomial.c00;
        auto c01 = polynomial.c01 * latitude;
        auto c10 = polynomial.c10 * longitude;
        auto c11 = polynomial.c11 * latitude * longitude;

        VERBOSEF("    c00: %.14f", c00);
        VERBOSEF("    c01: %.14f", c01);
        VERBOSEF("    c10: %.14f", c10);
        VERBOSEF("    c11: %.14f", c11);

        has_polynomial                 = true;
        correction.polynomial_residual = c00 + c01 + c10 + c11;

        if (polynomial.quality_indicator_valid) {
            correction.quality_valid = true;
            correction.quality       = polynomial.quality_indicator;
        }
    }

    auto grid_it = mGrid.find(sv_id.gnss());
    if (grid_it != mGrid.end()) {
        if (grid_it->second.ionospheric(sv_id, llh, correction.grid_residual)) {
            has_gridded = true;
        }
    }

    return has_polynomial || has_gridded;
}

static SatelliteId::Gnss satellite_gnss_from_id(long gnss_id) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return SatelliteId::GPS;
    case GNSS_ID__gnss_id_galileo: return SatelliteId::GALILEO;
    case GNSS_ID__gnss_id_glonass: return SatelliteId::GLONASS;
    case GNSS_ID__gnss_id_bds: return SatelliteId::BEIDOU;
    default: UNREACHABLE();
    }
}

static SignalId::Gnss signal_gnss_from_id(long gnss_id) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return SignalId::GPS;
    case GNSS_ID__gnss_id_galileo: return SignalId::GALILEO;
    case GNSS_ID__gnss_id_glonass: return SignalId::GLONASS;
    case GNSS_ID__gnss_id_bds: return SignalId::BEIDOU;
    default: UNREACHABLE();
    }
}

static SignalId signal_id_from(SignalId::Gnss gnss, GNSS_SignalID_t& signal_id) {
    auto id = signal_id.gnss_SignalID;
    if (signal_id.ext1 && signal_id.ext1->gnss_SignalID_Ext_r15) {
        id = *signal_id.ext1->gnss_SignalID_Ext_r15;
    }

    return SignalId::from_lpp(gnss, id);
}

void CorrectionData::add_correction(long                                 gnss_id,
                                    GNSS_SSR_OrbitCorrections_r15 const* orbit) NOEXCEPT {
    if (!orbit) return;

    FUNCTION_SCOPE();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(orbit->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(orbit->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssrUpdateInterval_r15(orbit->ssrUpdateInterval_r15);
    if (update_interval > 1.0) {
        reference_time.add_seconds(update_interval * 0.5);
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = orbit->ssr_OrbitCorrectionList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r15.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("orbit correction with invalid satellite id");
            continue;
        }

        auto iod = static_cast<uint16_t>(helper::BitString::from(&satellite->iod_r15)->as_int64());

        auto radial      = decode::delta_radial_r15(satellite->delta_radial_r15);
        auto along_track = decode::delta_AlongTrack_r15(satellite->delta_AlongTrack_r15);
        auto cross_track = decode::delta_CrossTrack_r15(satellite->delta_CrossTrack_r15);

        auto dot_radial = decode::dot_delta_radial_r15(satellite->dot_delta_radial_r15);
        auto dot_along  = decode::dot_delta_AlongTrack_r15(satellite->dot_delta_AlongTrack_r15);
        auto dot_cross  = decode::dot_delta_CrossTrack_r15(satellite->dot_delta_CrossTrack_r15);

        auto& correction          = mOrbit[satellite_id];
        correction.iod            = iod;
        correction.reference_time = reference_time;
        correction.delta          = {radial, along_track, cross_track};
        correction.dot_delta      = {dot_radial, dot_along, dot_cross};

#ifdef DATA_TRACING
        datatrace::report_ssr_orbit_correction(reference_time, satellite_id.name(),
                                               correction.delta, correction.dot_delta, ssr_iod,
                                               static_cast<long>(iod));
#endif

        VERBOSEF("orbit: %3s %+f %+f %+f", satellite_id.name(), radial, along_track, cross_track);
    }
}

void CorrectionData::add_correction(long                                 gnss_id,
                                    GNSS_SSR_ClockCorrections_r15 const* clock) NOEXCEPT {
    if (!clock) return;

    FUNCTION_SCOPE();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(clock->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(clock->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssrUpdateInterval_r15(clock->ssrUpdateInterval_r15);
    if (update_interval > 1.0) {
        reference_time.add_seconds(update_interval * 0.5);
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = clock->ssr_ClockCorrectionList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r15.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("clock correction with invalid satellite id");
            continue;
        }

        auto c0 = decode::delta_Clock_C0_r15(satellite->delta_Clock_C0_r15);
        auto c1 = decode::delta_Clock_C1_r15(satellite->delta_Clock_C1_r15);
        auto c2 = decode::delta_Clock_C2_r15(satellite->delta_Clock_C2_r15);

        auto& clock_correction          = mClock[satellite_id];
        clock_correction.reference_time = reference_time;
        clock_correction.c0             = c0;
        clock_correction.c1             = c1;
        clock_correction.c2             = c2;

#ifdef DATA_TRACING
        datatrace::report_ssr_clock_correction(reference_time, satellite_id.name(), c0, c1, c2,
                                               ssr_iod);
#endif

        VERBOSEF("clock: %3s %+f %+f %+f", satellite_id.name(), c0, c1, c2);
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_CodeBias_r15 const* code_bias) NOEXCEPT {
    if (!code_bias) return;

    FUNCTION_SCOPE();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(code_bias->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(code_bias->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = code_bias->ssr_CodeBiasSatList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r15.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("code bias rejected: invalid satellite id (%ld)",
                  satellite->svID_r15.satellite_id);
            continue;
        }

        auto& signal_corrections = mSignal[satellite_id];

        auto& signal_list = satellite->ssr_CodeBiasSignalList_r15.list;
        for (int j = 0; j < signal_list.count; j++) {
            auto signal = signal_list.array[j];
            if (!signal) continue;

            auto signal_id = signal_id_from(signal_gnss, signal->signal_and_tracking_mode_ID_r15);
            mSignals[satellite_id].insert(signal_id);

            CodeBiasCorrection correction{};
            correction.bias                         = decode::codeBias_r15(signal->codeBias_r15);
            signal_corrections.code_bias[signal_id] = correction;

            VERBOSEF("code bias: %3s %-16s %+f", satellite_id.name(), signal_id.name(),
                     correction.bias);
        }
    }
}

void CorrectionData::add_correction(long                          gnss_id,
                                    GNSS_SSR_PhaseBias_r16 const* phase_bias) NOEXCEPT {
    if (!phase_bias) return;

    FUNCTION_SCOPE();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(phase_bias->iod_ssr_r16);
    auto epoch_time = decode::epochTime_r15(phase_bias->epochTime_r16);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = phase_bias->ssr_PhaseBiasSatList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r16.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("phase bias rejected: invalid satellite id (%ld)",
                  satellite->svID_r16.satellite_id);
            continue;
        }

        auto& signal_corrections = mSignal[satellite_id];

        auto& signal_list = satellite->ssr_PhaseBiasSignalList_r16.list;
        for (int j = 0; j < signal_list.count; j++) {
            auto signal = signal_list.array[j];
            if (!signal) continue;

            auto signal_id = signal_id_from(signal_gnss, signal->signal_and_tracking_mode_ID_r16);
            mSignals[satellite_id].insert(signal_id);

            PhaseBiasCorrection correction{};
            correction.bias                          = decode::phaseBias_r16(signal->phaseBias_r16);
            signal_corrections.phase_bias[signal_id] = correction;

            VERBOSEF("phase bias: %3s %-16s %+f", satellite_id.name(), signal_id.name(),
                     correction.bias);
        }
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16 const* stec,
                                    CorrectionPointSet const& correction_point_set) NOEXCEPT {
    if (!stec) return;

    FUNCTION_SCOPE();

    if (stec->correctionPointSetID_r16 != correction_point_set.set_id) {
        WARNF("correction point set id mismatch");
        return;
    }

    mCorrectionPointSet = &correction_point_set;

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(stec->iod_ssr_r16);
    auto epoch_time = decode::epochTime_r15(stec->epochTime_r16);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = stec->stec_SatList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r16.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("stec correction with invalid satellite id");
            continue;
        }

        auto stec_quality_indicator =
            decode::stecQualityIndicator_r16(satellite->stecQualityIndicator_r16);

        auto c00 = decode::stec_C00_r16(satellite->stec_C00_r16);
        auto c01 = decode::stec_C01_r16(satellite->stec_C01_r16);
        auto c10 = decode::stec_C10_r16(satellite->stec_C10_r16);
        auto c11 = decode::stec_C11_r16(satellite->stec_C11_r16);

        auto& poly                     = mIonosphericPolynomial[satellite_id];
        poly.c00                       = c00;
        poly.c01                       = c01;
        poly.c10                       = c10;
        poly.c11                       = c11;
        poly.reference_point_latitude  = correction_point_set.reference_point_latitude;
        poly.reference_point_longitude = correction_point_set.reference_point_longitude;

        poly.quality_indicator_valid = !stec_quality_indicator.invalid;
        poly.quality_indicator       = stec_quality_indicator.value;

#ifdef DATA_TRACING
        datatrace::Option<long>   quality_indiciator_class{};
        datatrace::Option<long>   quality_indicator_value{};
        datatrace::Option<double> quality_indicator{};
        if (!stec_quality_indicator.invalid) {
            quality_indicator        = stec_quality_indicator.value;
            quality_indiciator_class = stec_quality_indicator.cls;
            quality_indicator_value  = stec_quality_indicator.val;
        }

        datatrace::report_ssr_ionospheric_polynomial(
            epoch_time, satellite_id.name(), c00, c01, c10, c11,
            correction_point_set.reference_point_latitude,
            correction_point_set.reference_point_longitude, quality_indicator,
            quality_indiciator_class, quality_indicator_value, ssr_iod);
#endif

        VERBOSEF("stec: %3s %+f %+f %+f %+f", satellite_id.name(), c00, c01, c10, c11);
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16 const* grid,
                                    CorrectionPointSet const& correction_point_set) NOEXCEPT {
    if (!grid) return;

    FUNCTION_SCOPE();

    if (grid->correctionPointSetID_r16 != correction_point_set.set_id) {
        WARNF("correction point set id mismatch");
        return;
    }

    mCorrectionPointSet = &correction_point_set;

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(grid->iod_ssr_r16);
    auto epoch_time = decode::epochTime_r15(grid->epochTime_r16);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto grid_it = mGrid.find(satellite_gnss);
    if (grid_it == mGrid.end()) {
        auto& gnss_grid = mGrid[satellite_gnss];
        gnss_grid.init(correction_point_set);

        long array_index    = 0;
        long absolute_index = 0;
        for (long y = 0; y <= correction_point_set.number_of_steps_latitude; y++) {
            for (long x = 0; x <= correction_point_set.number_of_steps_longitude; x++) {
                auto i        = y * (correction_point_set.number_of_steps_latitude + 1) + x;
                auto bit      = 1ULL << (64 - 1 - i);
                auto is_valid = (correction_point_set.bitmask & bit) != 0;

                Float3 position{};
                position.x = correction_point_set.reference_point_latitude -
                             static_cast<double>(y) * correction_point_set.step_of_latitude;
                position.y = correction_point_set.reference_point_longitude +
                             static_cast<double>(x) * correction_point_set.step_of_longitude;
                gnss_grid.add_point(array_index, absolute_index, is_valid, position);

                // only valid grid points are included in the array
                if (is_valid) {
                    array_index++;
                }
                absolute_index++;
            }
        }
    }

    auto& grid_data = mGrid[satellite_gnss];
    auto& list      = grid->gridList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        auto grid_point = grid_data.point_from_array_index(i);
        if (!grid_point) {
            WARNF("grid point not found");
            continue;
        }

        VERBOSEF("grid: %ld/%ld %.14f %.14f", grid_point->array_index, grid_point->absolute_index,
                 grid_point->position.x, grid_point->position.y);

        if (element->tropospericDelayCorrection_r16) {
            auto& tropo = *element->tropospericDelayCorrection_r16;
            auto  dry =
                decode::tropoHydroStaticVerticalDelay_r16(tropo.tropoHydroStaticVerticalDelay_r16) +
                2.3;
            auto wet = decode::tropoWetVerticalDelay_r16(tropo.tropoWetVerticalDelay_r16) + 0.252;

            grid_point->tropspheric_valid = true;
            grid_point->tropospheric_wet  = wet;
            grid_point->tropospheric_dry  = dry;

#ifdef DATA_TRACING
            datatrace::report_ssr_tropospheric_grid(epoch_time, grid_point->absolute_index,
                                                    grid_point->position, wet, dry, ssr_iod);
#endif

            VERBOSEF("  wet: %+f dry: %+f", wet, dry);
        }

        if (element->stec_ResidualSatList_r16) {
            auto& sat_list = element->stec_ResidualSatList_r16->list;
            for (int j = 0; j < sat_list.count; j++) {
                auto satellite = sat_list.array[j];
                if (!satellite) continue;

                auto satellite_id =
                    SatelliteId::from_lpp(satellite_gnss, satellite->svID_r16.satellite_id);
                if (!satellite_id.is_valid()) {
                    WARNF("stec correction with invalid satellite id");
                    continue;
                }

                auto ionospheric =
                    decode::stecResidualCorrection_r16(satellite->stecResidualCorrection_r16);

                grid_point->ionospheric_valid                  = true;
                grid_point->ionospheric_residual[satellite_id] = ionospheric;

#ifdef DATA_TRACING
                datatrace::report_ssr_ionospheric_grid(epoch_time, grid_point->absolute_index,
                                                       grid_point->position, satellite_id.name(),
                                                       ionospheric, ssr_iod);
#endif

                VERBOSEF("  ionospheric: %3s %+f", satellite_id.name(), ionospheric);
            }
        }
    }
}

}  // namespace tokoro
}  // namespace generator
