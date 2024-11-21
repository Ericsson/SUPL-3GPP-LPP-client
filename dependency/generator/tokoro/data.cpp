#include "data.hpp"
#include "decode.hpp"
#include "ecef.hpp"
#include "wgs84.hpp"

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

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

namespace generator {
namespace tokoro {

bool OrbitCorrection::correction(ts::Tai time, Float3 eph_position, Float3 eph_velocity,
                                 Float3& result) const NOEXCEPT {
    VSCOPE_FUNCTION();

    auto e_along = eph_velocity;
    if (!e_along.normalize()) {
        WARNF("failed to normalize e_along");
        return false;
    }
    VERBOSEF("e_along: %24.14f, %24.14f, %24.14f", e_along.x, e_along.y, e_along.z);

    auto e_cross = cross_product(eph_position, eph_velocity);
    if (!e_cross.normalize()) {
        WARNF("failed to normalize e_cross");
        return false;
    }
    VERBOSEF("e_cross: %24.14f, %24.14f, %24.14f", e_cross.x, e_cross.y, e_cross.z);

    auto e_radial = cross_product(e_along, e_cross);
    VERBOSEF("e_radial: %24.14f, %24.14f, %24.14f", e_radial.x, e_radial.y, e_radial.z);

    VERBOSEF("time: %s", ts::Utc{time}.rtklib_time_string().c_str());
    VERBOSEF("reference_time: %s", ts::Utc{reference_time}.rtklib_time_string().c_str());
    auto t_k      = ts::Gps{time}.difference(ts::Gps{reference_time}).full_seconds();
    auto delta_at = delta + dot_delta * t_k;

    VERBOSEF("t_k: %24.14f", t_k);
    VERBOSEF("delta:     %24.14f, %24.14f, %24.14f", delta.x, delta.y, delta.z);
    VERBOSEF("dot_delta: %24.14f, %24.14f, %24.14f", dot_delta.x, dot_delta.y, dot_delta.z);
    VERBOSEF("delta_at:  %24.14f, %24.14f, %24.14f", delta_at.x, delta_at.y, delta_at.z);

    auto x = e_radial.x * delta_at.x + e_along.x * delta_at.y + e_cross.x * delta_at.z;
    auto y = e_radial.y * delta_at.x + e_along.y * delta_at.y + e_cross.y * delta_at.z;
    auto z = e_radial.z * delta_at.x + e_along.z * delta_at.y + e_cross.z * delta_at.z;

    VERBOSEF("result: %24.14f, %24.14f, %24.14f", x, y, z);

    result = eph_position - Float3{x, y, z};
    return true;
}

double ClockCorrection::correction(ts::Tai time) const NOEXCEPT {
    auto t_k = ts::Gps{time}.difference(ts::Gps{reference_time}).full_seconds();
    return c0 + c1 * t_k + c2 * t_k * t_k;
}

bool CorrectionPointSet::array_to_index(long array_index, long& index, bool& valid,
                                        Wgs84Position& position) const NOEXCEPT {
    long array_count    = 0;
    long absolute_index = 0;
    for (long y = 0; y <= number_of_steps_latitude; y++) {
        for (long x = 0; x <= number_of_steps_longitude; x++) {
            auto i        = y * (number_of_steps_latitude + 1) + x;
            auto bit      = 1ULL << (64 - 1 - i);
            auto is_valid = (bitmask & bit) != 0;

            if (array_count == array_index) {
                index      = absolute_index;
                valid      = is_valid;
                position.x = reference_point_latitude - static_cast<double>(y) * step_of_latitude;
                position.y = reference_point_longitude + static_cast<double>(x) * step_of_longitude;
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

GridPoint const* GridData::find_top_left(Wgs84Position position) const NOEXCEPT {
    VSCOPE_FUNCTION();

    for (auto& grid_point : mGridPoints) {
        if (!grid_point.valid) {
            continue;
        }

        auto x0 = grid_point.position.x;
        auto y0 = grid_point.position.y;
        auto x1 = x0 - mDeltaLatitude;
        auto y1 = y0 + mDeltaLongitude;
        VERBOSEF("latitude:  %+18.14f >= %+18.14f >= %+18.14f", x0, position.x, x1);
        VERBOSEF("longitude: %+18.14f <= %+18.14f <= %+18.14f", y0, position.y, y1);
        if (position.x <= x0 && position.x >= x1 && position.y >= y0 && position.y <= y1) {
            VERBOSEF("found: %ld/%ld", grid_point.array_index, grid_point.absolute_index);
            return &grid_point;
        }
    }

    VERBOSEF("top left not found");
    return nullptr;
}

GridPoint const* GridData::find_with_absolute_index(long absolute_index) const NOEXCEPT {
    VSCOPE_FUNCTION();

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

bool GridData::find_4_points(Wgs84Position position, GridPoint const*& tl, GridPoint const*& tr,
                             GridPoint const*& bl, GridPoint const*& br) const NOEXCEPT {
    VSCOPE_FUNCTION();

    auto top_left = find_top_left(position);
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

bool GridData::ionospheric(SatelliteId sv_id, Wgs84Position position,
                           double& ionospheric_residual) const NOEXCEPT {
    VSCOPE_FUNCTION();

    // if we're inside 4 points, bilinear interpolation
    GridPoint const* tl = nullptr;
    GridPoint const* tr = nullptr;
    GridPoint const* bl = nullptr;
    GridPoint const* br = nullptr;
    if (find_4_points(position, tl, tr, bl, br)) {
        VERBOSEF("bilinear interpolation");

        if (!tl->has_ionospheric_residual(sv_id) || !tr->has_ionospheric_residual(sv_id) ||
            !bl->has_ionospheric_residual(sv_id) || !br->has_ionospheric_residual(sv_id)) {
            WARNF("ionospheric correction not found");
            return false;
        }

        auto dx = (position.x - tl->position.x) / (br->position.x - tl->position.x);
        auto dy = (position.y - tl->position.y) / (br->position.y - tl->position.y);

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

bool GridData::tropospheric(Wgs84Position           position,
                            TroposphericCorrection& correction) const NOEXCEPT {
    VSCOPE_FUNCTION();

    // if we're inside 4 points, bilinear interpolation
    GridPoint const* tl = nullptr;
    GridPoint const* tr = nullptr;
    GridPoint const* bl = nullptr;
    GridPoint const* br = nullptr;
    if (find_4_points(position, tl, tr, bl, br)) {
        VERBOSEF("bilinear interpolation");

        if (!tl->has_tropospheric_data() || !tr->has_tropospheric_data() ||
            !bl->has_tropospheric_data() || !br->has_tropospheric_data()) {
            WARNF("tropospheric correction not found");
            return false;
        }

        auto dx = (position.x - tl->position.x) / (br->position.x - tl->position.x);
        auto dy = (position.y - tl->position.y) / (br->position.y - tl->position.y);

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

        return true;
    }

    return false;
}

bool CorrectionData::tropospheric(SatelliteId sv_id, EcefPosition ecef,
                                  TroposphericCorrection& correction) const NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mCorrectionPointSet == nullptr) {
        WARNF("tropospheric correction grid not available");
        return false;
    }

    auto position = ecef_to_wgs84(ecef);
    auto grid_it  = mGrid.find(sv_id.gnss());
    if (grid_it == mGrid.end()) {
        WARNF("tropospheric correction for satellite not found");
        return false;
    }

    auto& grid = grid_it->second;
    if (!grid.tropospheric(position, correction)) {
        WARNF("tropospheric correction not found");
        return false;
    }

    return true;
}

bool CorrectionData::ionospheric(SatelliteId sv_id, EcefPosition ecef,
                                 IonosphericCorrection& correction) const NOEXCEPT {
    VSCOPE_FUNCTION();

    auto position       = ecef_to_wgs84(ecef);
    auto has_polynomial = false;
    auto has_gridded    = false;

    correction = {};

    auto it = mIonosphericPolynomial.find(sv_id);
    if (it != mIonosphericPolynomial.end()) {
        auto& polynomial = it->second;
        auto  latitude   = position.x - polynomial.reference_point_latitude;
        auto  longitude  = position.y - polynomial.reference_point_longitude;

        VERBOSEF("polynomial:");
        VERBOSEF("  c00: %.14f", polynomial.c00);
        VERBOSEF("  c01: %.14f", polynomial.c01);
        VERBOSEF("  c10: %.14f", polynomial.c10);
        VERBOSEF("  c11: %.14f", polynomial.c11);

        VERBOSEF("  px: %.14f", position.x);
        VERBOSEF("  rx: %.14f", polynomial.reference_point_latitude);
        VERBOSEF("  dx: %.14f", latitude);

        VERBOSEF("  py: %.14f", position.y);
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
    }

    auto grid_it = mGrid.find(sv_id.gnss());
    if (grid_it != mGrid.end()) {
        if (grid_it->second.ionospheric(sv_id, position, correction.grid_residual)) {
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

    VSCOPE_FUNCTION();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto epoch_time = decode::epochTime_r15(orbit->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssrUpdateInterval_r15(orbit->ssrUpdateInterval_r15);
    if (update_interval > 1.0) {
        reference_time.add_seconds(update_interval * 0.5);
    }

    auto& list = orbit->ssr_OrbitCorrectionList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r15.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("orbit correction with invalid satellite id");
            continue;
        }

        auto radial      = decode::delta_radial_r15(satellite->delta_radial_r15);
        auto along_track = decode::delta_AlongTrack_r15(satellite->delta_AlongTrack_r15);
        auto cross_track = decode::delta_CrossTrack_r15(satellite->delta_CrossTrack_r15);

        auto dot_radial = decode::dot_delta_radial_r15(satellite->dot_delta_radial_r15);
        auto dot_along  = decode::dot_delta_AlongTrack_r15(satellite->dot_delta_AlongTrack_r15);
        auto dot_cross  = decode::dot_delta_CrossTrack_r15(satellite->dot_delta_CrossTrack_r15);

#if 0
        radial      = -radial;
        cross_track = -cross_track;

        dot_radial = -dot_radial;
        dot_cross  = -dot_cross;
#endif

#if 0
        radial      = -radial;
        dot_radial = -dot_radial;
#endif
#if 0
        along_track = -along_track;
        dot_along = -dot_along;
#endif
#if 0
        cross_track = -cross_track;
        dot_cross = -dot_cross;
#endif


        auto& correction          = mOrbit[satellite_id];
        correction.reference_time = reference_time;
        correction.delta          = {radial, along_track, cross_track};
        correction.dot_delta      = {dot_radial, dot_along, dot_cross};

        VERBOSEF("orbit: %3s %+f %+f %+f", satellite_id.name(), radial, along_track, cross_track);
    }
}

void CorrectionData::add_correction(long                                 gnss_id,
                                    GNSS_SSR_ClockCorrections_r15 const* clock) NOEXCEPT {
    if (!clock) return;

    VSCOPE_FUNCTION();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto epoch_time = decode::epochTime_r15(clock->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssrUpdateInterval_r15(clock->ssrUpdateInterval_r15);
    if (update_interval > 1.0) {
        reference_time.add_seconds(update_interval * 0.5);
    }

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

        VERBOSEF("clock: %3s %+f %+f %+f", satellite_id.name(), c0, c1, c2);
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_CodeBias_r15 const* code_bias) NOEXCEPT {
    if (!code_bias) return;

    VSCOPE_FUNCTION();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto epoch_time = decode::epochTime_r15(code_bias->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto& list = code_bias->ssr_CodeBiasSatList_r15.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r15.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("code bias rejected: invalid satellite id (%ld)", satellite->svID_r15.satellite_id);
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

    VSCOPE_FUNCTION();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto epoch_time = decode::epochTime_r15(phase_bias->epochTime_r16);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto& list = phase_bias->ssr_PhaseBiasSatList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r16.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("phase bias rejected: invalid satellite id (%ld)", satellite->svID_r16.satellite_id);
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

    VSCOPE_FUNCTION();

    if (stec->correctionPointSetID_r16 != correction_point_set.set_id) {
        WARNF("correction point set id mismatch");
        return;
    }

    mCorrectionPointSet = &correction_point_set;

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto epoch_time = decode::epochTime_r15(stec->epochTime_r16);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto& list = stec->stec_SatList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r16.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("stec correction with invalid satellite id");
            continue;
        }

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

        VERBOSEF("stec: %3s %+f %+f %+f %+f", satellite_id.name(), c00, c01, c10, c11);
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16 const* grid,
                                    CorrectionPointSet const& correction_point_set) NOEXCEPT {
    if (!grid) return;

    VSCOPE_FUNCTION();

    if (grid->correctionPointSetID_r16 != correction_point_set.set_id) {
        WARNF("correction point set id mismatch");
        return;
    }

    mCorrectionPointSet = &correction_point_set;

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto epoch_time = decode::epochTime_r15(grid->epochTime_r16);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto grid_it = mGrid.find(satellite_gnss);
    if (grid_it == mGrid.end()) {
        auto& grid = mGrid[satellite_gnss];
        grid.init(correction_point_set);

        long array_index    = 0;
        long absolute_index = 0;
        for (long y = 0; y <= correction_point_set.number_of_steps_latitude; y++) {
            for (long x = 0; x <= correction_point_set.number_of_steps_longitude; x++) {
                auto i        = y * (correction_point_set.number_of_steps_latitude + 1) + x;
                auto bit      = 1ULL << (64 - 1 - i);
                auto is_valid = (correction_point_set.bitmask & bit) != 0;

                Wgs84Position position{};
                position.x = correction_point_set.reference_point_latitude -
                             static_cast<double>(y) * correction_point_set.step_of_latitude;
                position.y = correction_point_set.reference_point_longitude +
                             static_cast<double>(x) * correction_point_set.step_of_longitude;
                grid.add_point(array_index, absolute_index, is_valid, position);

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

                VERBOSEF("  ionospheric: %3s %+f", satellite_id.name(), ionospheric);
            }
        }
    }
}

}  // namespace tokoro
}  // namespace generator
