#include "constant.hpp"
#include "correction.hpp"
#include "decode.hpp"

#ifdef ENABLE_TOKORO_SNAPSHOT
#include <generator/tokoro/snapshot.hpp>
#endif

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

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE3(tokoro, data, gather);
#undef LOGLET_CURRENT_MODULE
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
    auto epoch_time = decode::epoch_time_r15(orbit->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssr_update_interval_r15(orbit->ssrUpdateInterval_r15);
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
        auto along_track = decode::delta_along_track_r15(satellite->delta_AlongTrack_r15);
        auto cross_track = decode::delta_cross_track_r15(satellite->delta_CrossTrack_r15);

        auto dot_radial = decode::dot_delta_radial_r15(satellite->dot_delta_radial_r15);
        auto dot_along  = decode::dot_delta_along_track_r15(satellite->dot_delta_AlongTrack_r15);
        auto dot_cross  = decode::dot_delta_cross_track_r15(satellite->dot_delta_CrossTrack_r15);

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
    auto epoch_time = decode::epoch_time_r15(clock->epochTime_r15);
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssr_update_interval_r15(clock->ssrUpdateInterval_r15);
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

        auto c0 = decode::delta_clock_c0_r15(satellite->delta_Clock_C0_r15);
        auto c1 = decode::delta_clock_c1_r15(satellite->delta_Clock_C1_r15);
        auto c2 = decode::delta_clock_c2_r15(satellite->delta_Clock_C2_r15);

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
    auto epoch_time = decode::epoch_time_r15(code_bias->epochTime_r15);
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
            correction.bias                         = decode::code_bias_r15(signal->codeBias_r15);
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
    auto epoch_time = decode::epoch_time_r15(phase_bias->epochTime_r16);
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
            correction.ssr_iod = static_cast<uint16_t>(ssr_iod);
            correction.bias    = decode::phase_bias_r16(signal->phaseBias_r16);
            signal_corrections.phase_bias[signal_id] = correction;

            VERBOSEF("phase bias: %3s %-16s %+f", satellite_id.name(), signal_id.name(),
                     correction.bias);
        }
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_STEC_Correction_r16 const* stec,
                                    CorrectionPointSet const& cps) NOEXCEPT {
    if (!stec) return;

    FUNCTION_SCOPE();

    if (stec->correctionPointSetID_r16 != cps.set_id) {
        WARNF("correction point set id mismatch");
        return;
    }

    correction_point_set = &cps;

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto epoch_time = decode::epoch_time_r15(stec->epochTime_r16);
#ifdef DATA_TRACING
    auto ssr_iod = decode::iod_ssr_r16(stec->iod_ssr_r16);
#endif
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
            decode::stec_quality_indicator_r16(satellite->stecQualityIndicator_r16);

        auto c00 = decode::stec_c00_r16(satellite->stec_C00_r16);
        auto c01 = decode::stec_c01_r16(satellite->stec_C01_r16);
        auto c10 = decode::stec_c10_r16(satellite->stec_C10_r16);
        auto c11 = decode::stec_c11_r16(satellite->stec_C11_r16);

        auto& poly                     = mIonosphericPolynomial[satellite_id];
        poly.c00                       = c00;
        poly.c01                       = c01;
        poly.c10                       = c10;
        poly.c11                       = c11;
        poly.reference_point_latitude  = correction_point_set->reference_point_latitude;
        poly.reference_point_longitude = correction_point_set->reference_point_longitude;

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
            correction_point_set->reference_point_latitude,
            correction_point_set->reference_point_longitude, quality_indicator,
            quality_indiciator_class, quality_indicator_value, ssr_iod);
#endif

        VERBOSEF("stec: %3s %+f %+f %+f %+f", satellite_id.name(), c00, c01, c10, c11);
    }
}

void CorrectionData::add_correction(long gnss_id, GNSS_SSR_GriddedCorrection_r16 const* grid,
                                    CorrectionPointSet const& cps) NOEXCEPT {
    if (!grid) return;

    FUNCTION_SCOPE();

    if (grid->correctionPointSetID_r16 != cps.set_id) {
        WARNF("correction point set id mismatch");
        return;
    }

    this->correction_point_set = &cps;

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto epoch_time = decode::epoch_time_r15(grid->epochTime_r16);
#ifdef DATA_TRACING
    auto ssr_iod = decode::iod_ssr_r16(grid->iod_ssr_r16);
#endif
    if (epoch_time.timestamp().full_seconds() > mLatestCorrectionTime.timestamp().full_seconds()) {
        mLatestCorrectionTime = epoch_time;
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = grid->gridList_r16.list;

    auto grid_it = mGrid.find(satellite_gnss);
    if (grid_it == mGrid.end() ||
        grid_it->second.correction_point_set_id != correction_point_set->set_id) {
        auto& gnss_grid = mGrid[satellite_gnss];
        gnss_grid.init(*correction_point_set);

        for (int i = 0; i < list.count; i++) {
            auto element = list.array[i];
            if (!element) continue;

            CorrectionPointInfo correction_point;
            if (!correction_point_set->array_to_index(i, &correction_point)) {
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
            auto  dry   = decode::tropo_hydro_static_vertical_delay_r16(
                           tropo.tropoHydroStaticVerticalDelay_r16) +
                       2.3;
            auto wet =
                decode::tropo_wet_vertical_delay_r16(tropo.tropoWetVerticalDelay_r16) + 0.252;

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

                auto ionospheric = decode::stec_residual_correction_r16(*satellite);

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

#ifdef ENABLE_TOKORO_SNAPSHOT
void CorrectionData::snapshot(std::vector<SnapshotOrbitCorrection>&       orbit_corrections,
                              std::vector<SnapshotClockCorrection>&       clock_corrections,
                              std::vector<SnapshotCodeBias>&              code_biases,
                              std::vector<SnapshotPhaseBias>&             phase_biases,
                              std::vector<SnapshotIonosphericPolynomial>& ionospheric_polynomials,
                              std::vector<SnapshotGridData>& grid_data) const NOEXCEPT {
    orbit_corrections.clear();
    clock_corrections.clear();
    code_biases.clear();
    phase_biases.clear();
    ionospheric_polynomials.clear();
    grid_data.clear();

    for (auto const& pair : mOrbit) {
        SnapshotOrbitCorrection oc;
        oc.gnss            = static_cast<int32_t>(pair.first.gnss());
        oc.prn             = static_cast<int32_t>(pair.first.prn().value);
        oc.radial          = pair.second.delta.x;
        oc.along_track     = pair.second.delta.y;
        oc.cross_track     = pair.second.delta.z;
        oc.dot_radial      = pair.second.dot_delta.x;
        oc.dot_along_track = pair.second.dot_delta.y;
        oc.dot_cross_track = pair.second.dot_delta.z;
        oc.iod             = pair.second.iod;
        orbit_corrections.push_back(oc);
    }

    for (auto const& pair : mClock) {
        SnapshotClockCorrection cc;
        cc.gnss = static_cast<int32_t>(pair.first.gnss());
        cc.prn  = static_cast<int32_t>(pair.first.prn().value);
        cc.c0   = pair.second.c0;
        cc.c1   = pair.second.c1;
        cc.c2   = pair.second.c2;
        clock_corrections.push_back(cc);
    }

    for (auto const& pair : mSignal) {
        for (auto const& code_bias : pair.second.code_bias) {
            SnapshotCodeBias cb;
            cb.gnss   = static_cast<int32_t>(pair.first.gnss());
            cb.prn    = static_cast<int32_t>(pair.first.prn().value);
            cb.signal = static_cast<int32_t>(code_bias.first.lpp_id());
            cb.bias   = code_bias.second.bias;
            code_biases.push_back(cb);
        }

        for (auto const& phase_bias : pair.second.phase_bias) {
            SnapshotPhaseBias pb;
            pb.gnss   = static_cast<int32_t>(pair.first.gnss());
            pb.prn    = static_cast<int32_t>(pair.first.prn().value);
            pb.signal = static_cast<int32_t>(phase_bias.first.lpp_id());
            pb.bias   = phase_bias.second.bias;
            phase_biases.push_back(pb);
        }
    }

    for (auto const& pair : mIonosphericPolynomial) {
        SnapshotIonosphericPolynomial sip;
        sip.gnss                      = static_cast<int32_t>(pair.first.gnss());
        sip.prn                       = static_cast<int32_t>(pair.first.prn().value);
        sip.c00                       = pair.second.c00;
        sip.c01                       = pair.second.c01;
        sip.c10                       = pair.second.c10;
        sip.c11                       = pair.second.c11;
        sip.reference_point_latitude  = pair.second.reference_point_latitude;
        sip.reference_point_longitude = pair.second.reference_point_longitude;
        sip.quality_indicator         = pair.second.quality_indicator;
        sip.quality_indicator_valid   = pair.second.quality_indicator_valid;
        ionospheric_polynomials.push_back(sip);
    }

    for (auto const& grid_pair : mGrid) {
        SnapshotGridData sgd;
        sgd.gnss = static_cast<int32_t>(grid_pair.first);

        for (auto const& point : grid_pair.second.grid_points) {
            if (!point.valid) continue;

            SnapshotGridPoint sgp;
            sgp.latitude_index  = point.latitude_index;
            sgp.longitude_index = point.longitude_index;
            sgp.latitude        = point.position.x;
            sgp.longitude       = point.position.y;
            sgp.height          = point.position.z;
            sgp.has_troposphere = point.tropspheric_valid;
            sgp.troposphere_wet = point.tropospheric_wet;
            sgp.troposphere_dry = point.tropospheric_dry;

            for (auto const& iono : point.ionospheric_residual) {
                SnapshotIonosphereResidual sir;
                sir.gnss     = static_cast<int32_t>(iono.first.gnss());
                sir.prn      = static_cast<int32_t>(iono.first.prn().value);
                sir.residual = iono.second;
                sgp.ionosphere_residuals.push_back(sir);
            }

            sgd.grid_points.push_back(sgp);
        }

        if (!sgd.grid_points.empty()) {
            grid_data.push_back(sgd);
        }
    }
}

void CorrectionData::load_snapshot(
    std::vector<SnapshotOrbitCorrection> const&       orbit_corrections,
    std::vector<SnapshotClockCorrection> const&       clock_corrections,
    std::vector<SnapshotCodeBias> const&              code_biases,
    std::vector<SnapshotPhaseBias> const&             phase_biases,
    std::vector<SnapshotIonosphericPolynomial> const& ionospheric_polynomials,
    std::vector<SnapshotGridData> const&              grid_data) NOEXCEPT {
    FUNCTION_SCOPE();
    mOrbit.clear();
    mClock.clear();
    mSignal.clear();
    mIonosphericPolynomial.clear();
    mGrid.clear();

    {
        VERBOSEF("loading %ld orbit corrections:", orbit_corrections.size());
        VERBOSE_INDENT_SCOPE();
        for (auto const& oc : orbit_corrections) {
            auto sat_id = SatelliteId::from_lpp(static_cast<SatelliteId::Gnss>(oc.gnss), oc.prn);
            OrbitCorrection orbit;
            orbit.delta.x     = oc.radial;
            orbit.delta.y     = oc.along_track;
            orbit.delta.z     = oc.cross_track;
            orbit.dot_delta.x = oc.dot_radial;
            orbit.dot_delta.y = oc.dot_along_track;
            orbit.dot_delta.z = oc.dot_cross_track;
            orbit.iod         = oc.iod;
            VERBOSEF("%3s %+f %+f %+f %+f %+f %+f %+f", sat_id.name(), orbit.delta.x, orbit.delta.y,
                     orbit.delta.z, orbit.dot_delta.x, orbit.dot_delta.y, orbit.dot_delta.z,
                     orbit.iod);
            mOrbit[sat_id] = orbit;
        }
    }

    {
        VERBOSEF("loading %ld clock corrections:", clock_corrections.size());
        VERBOSE_INDENT_SCOPE();
        for (auto const& cc : clock_corrections) {
            auto sat_id = SatelliteId::from_lpp(static_cast<SatelliteId::Gnss>(cc.gnss), cc.prn);
            ClockCorrection clock;
            clock.c0 = cc.c0;
            clock.c1 = cc.c1;
            clock.c2 = cc.c2;
            VERBOSEF("%3s %+f %+f %+f", sat_id.name(), clock.c0, clock.c1, clock.c2);
            mClock[sat_id] = clock;
        }
    }

    {
        VERBOSEF("loading %ld code biases:", code_biases.size());
        VERBOSE_INDENT_SCOPE();
        for (auto const& cb : code_biases) {
            auto sat_id    = SatelliteId::from_lpp(static_cast<SatelliteId::Gnss>(cb.gnss), cb.prn);
            auto signal_id = SignalId::from_lpp(static_cast<SignalId::Gnss>(cb.gnss), cb.signal);
            if (!signal_id.is_valid()) continue;

            CodeBiasCorrection code_bias;
            code_bias.bias = cb.bias;
            VERBOSEF("%3s %s %+f", sat_id.name(), signal_id.name(), code_bias.bias);
            mSignal[sat_id].code_bias[signal_id] = code_bias;
            mSignals[sat_id].insert(signal_id);
        }
    }

    {
        VERBOSEF("loading %ld phase biases:", phase_biases.size());
        VERBOSE_INDENT_SCOPE();
        for (auto const& pb : phase_biases) {
            auto sat_id    = SatelliteId::from_lpp(static_cast<SatelliteId::Gnss>(pb.gnss), pb.prn);
            auto signal_id = SignalId::from_lpp(static_cast<SignalId::Gnss>(pb.gnss), pb.signal);
            if (!signal_id.is_valid()) continue;

            PhaseBiasCorrection phase_bias;
            phase_bias.bias = pb.bias;
            VERBOSEF("%3s %s %+f", sat_id.name(), signal_id.name(), phase_bias.bias);
            mSignal[sat_id].phase_bias[signal_id] = phase_bias;
            mSignals[sat_id].insert(signal_id);
        }
    }

    {
        VERBOSEF("loading %ld ionospheric polynomials:", ionospheric_polynomials.size());
        VERBOSE_INDENT_SCOPE();
        for (auto const& sip : ionospheric_polynomials) {
            auto sat_id = SatelliteId::from_lpp(static_cast<SatelliteId::Gnss>(sip.gnss), sip.prn);

            IonosphericPolynomial poly;
            poly.c00                       = sip.c00;
            poly.c01                       = sip.c01;
            poly.c10                       = sip.c10;
            poly.c11                       = sip.c11;
            poly.reference_point_latitude  = sip.reference_point_latitude;
            poly.reference_point_longitude = sip.reference_point_longitude;
            poly.quality_indicator         = sip.quality_indicator;
            poly.quality_indicator_valid   = sip.quality_indicator_valid;

            VERBOSEF("%3s c00=%+f c01=%+f c10=%+f c11=%+f ref=(%.6f,%.6f)", sat_id.name(), poly.c00,
                     poly.c01, poly.c10, poly.c11, poly.reference_point_latitude,
                     poly.reference_point_longitude);

            mIonosphericPolynomial[sat_id] = poly;
        }
    }

    {
        VERBOSEF("loading %ld grid data:", grid_data.size());
        VERBOSE_INDENT_SCOPE();

        for (auto const& sgd : grid_data) {
            auto      gnss = static_cast<SatelliteId::Gnss>(sgd.gnss);
            GridData& grid = mGrid[gnss];

            if (correction_point_set) {
                grid.init(*correction_point_set);
            }

            VERBOSEF("GNSS %d: %ld points", sgd.gnss, sgd.grid_points.size());
            for (auto const& sgp : sgd.grid_points) {
                GridPoint point;
                point.valid             = true;
                point.latitude_index    = sgp.latitude_index;
                point.longitude_index   = sgp.longitude_index;
                point.position.x        = sgp.latitude;
                point.position.y        = sgp.longitude;
                point.position.z        = sgp.height;
                point.tropspheric_valid = sgp.has_troposphere;
                point.tropospheric_wet  = sgp.troposphere_wet;
                point.tropospheric_dry  = sgp.troposphere_dry;
                point.ionospheric_valid = !sgp.ionosphere_residuals.empty();

                // Calculate absolute index: row * width + col
                point.absolute_index =
                    sgp.latitude_index * (grid.number_of_steps_longitude + 1) + sgp.longitude_index;
                point.array_index = point.absolute_index;

                VERBOSEF("  %3d %3d %+f %+f %+f (%d %+f %+f) %zu abs=%ld", sgp.latitude_index,
                         sgp.longitude_index, sgp.latitude, sgp.longitude, sgp.height,
                         sgp.has_troposphere, sgp.troposphere_wet, sgp.troposphere_dry,
                         sgp.ionosphere_residuals.size(), point.absolute_index);

                for (auto const& sir : sgp.ionosphere_residuals) {
                    auto sat_id =
                        SatelliteId::from_lpp(static_cast<SatelliteId::Gnss>(sir.gnss), sir.prn);
                    VERBOSEF("    %3s %+f", sat_id.name(), sir.residual);
                    point.ionospheric_residual[sat_id] = sir.residual;
                }

                // Place at correct index instead of push_back
                if (point.absolute_index >= 0 &&
                    point.absolute_index < static_cast<long>(grid.grid_points.size())) {
                    grid.grid_points[static_cast<size_t>(point.absolute_index)] = point;
                }
            }
        }
    }
}
#endif

}  // namespace tokoro
}  // namespace generator
