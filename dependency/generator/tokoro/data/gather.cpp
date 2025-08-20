#include "constant.hpp"
#include "correction.hpp"
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

LOGLET_MODULE3(tokoro, data, gather);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(tokoro, data, gather)

namespace generator {
namespace tokoro {

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
        correction.ssr_iod        = static_cast<uint16_t>(ssr_iod);
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
        clock_correction.ssr_iod        = static_cast<uint16_t>(ssr_iod);
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
            correction.ssr_iod                      = static_cast<uint16_t>(ssr_iod);
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
            correction.ssr_iod                       = static_cast<uint16_t>(ssr_iod);
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

    auto& list = grid->gridList_r16.list;

    auto grid_it = mGrid.find(satellite_gnss);
    if (grid_it == mGrid.end() ||
        grid_it->second.mCorrectionPointSetId != correction_point_set.set_id) {
        auto& gnss_grid = mGrid[satellite_gnss];
        gnss_grid.init(correction_point_set);

        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            CorrectionPointInfo correction_point;
            if (!correction_point_set.array_to_index(i, &correction_point)) {
                WARNF("correction point set array to index failed: %d", i);
                correction_point.is_valid = false;
                gnss_grid.add_point(correction_point);
                continue;
            }
            DEBUGF("point: %s %2ld,%2ld, (%2ld,%2ld), %.14f, %.14f, %.14f",
                   correction_point.is_valid ? "OK " : "BAD", correction_point.array_index,
                   correction_point.absolute_index, correction_point.latitude_index,
                   correction_point.longitude_index, correction_point.position.x,
                   correction_point.position.y, correction_point.position.z);
            // xer_fprint(stdout, &asn_DEF_GridElement_r16, element);

            gnss_grid.add_point(correction_point);
        }
    }

    auto& grid_data = mGrid[satellite_gnss];
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

    grid_data.print_grid();
}

}  // namespace tokoro
}  // namespace generator
