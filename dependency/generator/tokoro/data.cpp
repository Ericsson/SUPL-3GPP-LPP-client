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

bool CorrectionPointSet::array_to_index(long array_index, GridIndex& grid_index) const NOEXCEPT {
    long array_count = 0;
    for (long y = 0; y <= number_of_steps_latitude; y++) {
        for (long x = 0; x <= number_of_steps_longitude; x++) {
            if (array_count == array_index) {
                grid_index.x = x;
                grid_index.y = y;
                grid_index.i = x * (number_of_steps_latitude + 1) + y;
                return true;
            }
            array_count++;
        }
    }

    return false;
}

bool CorrectionPointSet::position_to_index(Wgs84Position position,
                                           GridIndex&    grid_index) const NOEXCEPT {
    VSCOPE_FUNCTION();

    VERBOSEF("position: %f, %f", position.x, position.y);
    VERBOSEF("reference_point: %f, %f", reference_point_latitude, reference_point_longitude);

    auto delta_latitude  = reference_point_latitude - position.x;
    auto delta_longitude = position.y - reference_point_longitude;
    VERBOSEF("delta_latitude: %f", delta_latitude);
    VERBOSEF("delta_longitude: %f", delta_longitude);

    auto x  = delta_longitude / step_of_longitude;
    auto y  = delta_latitude / step_of_latitude;
    auto ix = static_cast<long>(x);
    auto iy = static_cast<long>(y);

    VERBOSEF("x: %f (%ld)", x, ix);
    VERBOSEF("y: %f (%ld)", y, iy);

    if (ix < 0 || ix > number_of_steps_longitude || iy < 0 || iy > number_of_steps_latitude) {
        return false;
    }

    grid_index.x = ix;
    grid_index.y = iy;
    grid_index.i = grid_index.x * (number_of_steps_latitude + 1) + grid_index.y;
    return true;
}

Wgs84Position CorrectionPointSet::grid_point_position(GridIndex index) const NOEXCEPT {
    Wgs84Position position{};
    position.x = reference_point_latitude - static_cast<double>(index.y) * step_of_latitude;
    position.y = reference_point_longitude + static_cast<double>(index.x) * step_of_longitude;
    return position;
}

static double interpolate(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
}

bool TroposphereGrid::interpolate_4_points(GridIndex top_left_index, GridIndex top_right_index,
                                           GridIndex bottom_left_index,
                                           GridIndex bottom_right_index, Wgs84Position position,
                                           TroposphericCorrection& correction) const NOEXCEPT {
    VSCOPE_FUNCTION();

    VERBOSEF("top left:     (%ld, %ld)", top_left_index.x, top_left_index.y);
    VERBOSEF("top right:    (%ld, %ld)", top_right_index.x, top_right_index.y);
    VERBOSEF("bottom left:  (%ld, %ld)", bottom_left_index.x, bottom_left_index.y);
    VERBOSEF("bottom right: (%ld, %ld)", bottom_right_index.x, bottom_right_index.y);

    auto top_left_it     = points.find(top_left_index);
    auto top_right_it    = points.find(top_right_index);
    auto bottom_left_it  = points.find(bottom_left_index);
    auto bottom_right_it = points.find(bottom_right_index);
    if (top_left_it == points.end() || top_right_it == points.end() ||
        bottom_left_it == points.end() || bottom_right_it == points.end()) {
        WARNF("tropospheric correction grid points not found");
        return false;
    }

    auto& top_left     = top_left_it->second;
    auto& top_right    = top_right_it->second;
    auto& bottom_left  = bottom_left_it->second;
    auto& bottom_right = bottom_right_it->second;

    auto dx = (position.y - top_left.position.y) / (bottom_right.position.y - top_left.position.y);
    auto dy = (position.x - top_left.position.x) / (bottom_right.position.x - top_left.position.x);
    VERBOSEF("dx: %+f", dx);
    VERBOSEF("dy: %+f", dy);

    VERBOSEF("wet grid: %+8.5f %+8.5f", top_left.tropospheric.wet, top_right.tropospheric.wet);
    VERBOSEF("          %+8.5f %+8.5f", bottom_left.tropospheric.wet,
             bottom_right.tropospheric.wet);
    auto wet = interpolate(
        interpolate(top_left.tropospheric.wet, top_right.tropospheric.wet, dx),
        interpolate(bottom_left.tropospheric.wet, bottom_right.tropospheric.wet, dx), dy);
    VERBOSEF("wet: %+f", wet);

    VERBOSEF("dry grid: %+8.5f %+8.5f", top_left.tropospheric.dry, top_right.tropospheric.dry);
    VERBOSEF("          %+8.5f %+8.5f", bottom_left.tropospheric.dry,
             bottom_right.tropospheric.dry);
    auto dry = interpolate(
        interpolate(top_left.tropospheric.dry, top_right.tropospheric.dry, dx),
        interpolate(bottom_left.tropospheric.dry, bottom_right.tropospheric.dry, dx), dy);
    VERBOSEF("dry: %+f", dry);

    correction.wet = wet;
    correction.dry = dry;
    return true;
}

bool IonosphereGrid::interpolate_4_points(GridIndex top_left_index, GridIndex top_right_index,
                                          GridIndex bottom_left_index, GridIndex bottom_right_index,
                                          Wgs84Position position,
                                          double&       correction) const NOEXCEPT {
    VSCOPE_FUNCTION();

    VERBOSEF("top left:     (%ld, %ld)", top_left_index.x, top_left_index.y);
    VERBOSEF("top right:    (%ld, %ld)", top_right_index.x, top_right_index.y);
    VERBOSEF("bottom left:  (%ld, %ld)", bottom_left_index.x, bottom_left_index.y);
    VERBOSEF("bottom right: (%ld, %ld)", bottom_right_index.x, bottom_right_index.y);

    auto top_left_it     = points.find(top_left_index);
    auto top_right_it    = points.find(top_right_index);
    auto bottom_left_it  = points.find(bottom_left_index);
    auto bottom_right_it = points.find(bottom_right_index);

    if (top_left_it == points.end() || top_right_it == points.end() ||
        bottom_left_it == points.end() || bottom_right_it == points.end()) {
        WARNF("ionospheric correction grid points not found");
        return false;
    }

    auto& top_left     = top_left_it->second;
    auto& top_right    = top_right_it->second;
    auto& bottom_left  = bottom_left_it->second;
    auto& bottom_right = bottom_right_it->second;

    auto dx = (position.y - top_left.position.y) / (bottom_right.position.y - top_left.position.y);
    auto dy = (position.x - top_left.position.x) / (bottom_right.position.x - top_left.position.x);
    VERBOSEF("dx: %+f", dx);
    VERBOSEF("dy: %+f", dy);

    VERBOSEF("grid: %+8.5f %+8.5f", top_left.ionospheric, top_right.ionospheric);
    VERBOSEF("      %+8.5f %+8.5f", bottom_left.ionospheric, bottom_right.ionospheric);
    correction =
        interpolate(interpolate(top_left.ionospheric, top_right.ionospheric, dx),
                    interpolate(bottom_left.ionospheric, bottom_right.ionospheric, dx), dy);
    VERBOSEF("correction: %+f", correction);

    return true;
}

bool CorrectionData::tropospheric(SatelliteId sv_id, EcefPosition ecef,
                                  TroposphericCorrection& correction) const NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mCorrectionPointSet == nullptr) {
        WARNF("tropospheric correction grid not available");
        return false;
    }

    auto position = ecef_to_wgs84(ecef);
    auto grid_it  = mTroposphereGrid.find(sv_id.gnss());
    if (grid_it == mTroposphereGrid.end()) {
        WARNF("tropospheric correction for satellite not found");
        return false;
    }

    GridIndex top_left_index{};
    if (!mCorrectionPointSet->position_to_index(position, top_left_index)) {
        WARNF("tropospheric correction grid point not found");
        return false;
    }

    auto top_right_index    = mCorrectionPointSet->next_longitude(top_left_index);
    auto bottom_left_index  = mCorrectionPointSet->next_latitude(top_left_index);
    auto bottom_right_index = mCorrectionPointSet->next_longitude(bottom_left_index);

    auto& grid = grid_it->second;
    return grid.interpolate_4_points(top_left_index, top_right_index, bottom_left_index,
                                     bottom_right_index, position, correction);
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
        auto& polynomial               = it->second;
        auto  latitude                 = position.x - polynomial.reference_point_latitude;
        auto  longitude                = position.y - polynomial.reference_point_longitude;
        has_polynomial                 = true;
        correction.polynomial_residual = polynomial.c00 + polynomial.c01 * latitude +
                                         polynomial.c10 * longitude +
                                         polynomial.c11 * latitude * longitude;
    }

    auto grid_it = mIonosphereGrid.find(sv_id);
    if (grid_it != mIonosphereGrid.end()) {
        auto& grid = grid_it->second;

        GridIndex top_left_index{};
        if (mCorrectionPointSet->position_to_index(position, top_left_index)) {
            auto top_right_index    = mCorrectionPointSet->next_longitude(top_left_index);
            auto bottom_left_index  = mCorrectionPointSet->next_latitude(top_left_index);
            auto bottom_right_index = mCorrectionPointSet->next_longitude(bottom_left_index);

            has_gridded =
                grid.interpolate_4_points(top_left_index, top_right_index, bottom_left_index,
                                          bottom_right_index, position, correction.grid_residual);
        } else {
            WARNF("ionospheric correction grid point not found");
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
            WARNF("code bias with invalid satellite id");
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
            WARNF("code bias with invalid satellite id");
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

    auto& list = grid->gridList_r16.list;

    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;

        GridIndex index{};
        if (!correction_point_set.array_to_index(i, index)) {
            WARNF("failed to convert array index to grid index");
            continue;
        }

        if (element->tropospericDelayCorrection_r16) {
            auto& tropo = *element->tropospericDelayCorrection_r16;
            auto  wet =
                decode::tropoHydroStaticVerticalDelay_r16(tropo.tropoHydroStaticVerticalDelay_r16) +
                2.3;
            auto dry = decode::tropoWetVerticalDelay_r16(tropo.tropoWetVerticalDelay_r16) + 0.252;

            auto& tgrid                 = mTroposphereGrid[satellite_gnss];
            auto& grid_point            = tgrid.points[index];
            grid_point.position         = correction_point_set.grid_point_position(index);
            grid_point.tropospheric.wet = wet;
            grid_point.tropospheric.dry = dry;

            VERBOSEF("grid: (%02ld,%02ld) wet: %+f dry: %+f", index.x, index.y, wet, dry);
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

                auto& grid_point       = mIonosphereGrid[satellite_id].points[index];
                grid_point.position    = correction_point_set.grid_point_position(index);
                grid_point.ionospheric = ionospheric;

                VERBOSEF("grid: (%02ld,%02ld) ionospheric: %+f", index.x, index.y, ionospheric);
            }
        }
    }
}

}  // namespace tokoro
}  // namespace generator
