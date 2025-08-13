#include "correction.hpp"

#include <generator/tokoro/coordinate.hpp>
#include <loglet/loglet.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-SSR-ClockCorrections-r15.h>
#include <GNSS-SSR-CodeBias-r15.h>
#include <GNSS-SSR-GriddedCorrection-r16.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <GNSS-SSR-PhaseBias-r16.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <GridElement-r16.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <ProvideAssistanceData-r9-IEs.h>
#include <ProvideAssistanceData.h>
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
#include "decode.hpp"

#if DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE2(idokeido, corr);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, corr)

namespace idokeido {

static uint8_t gnss_from_satellite_id(SatelliteId id) NOEXCEPT {
    switch (id.gnss()) {
    case SatelliteId::Gnss::GPS: return 0;
    case SatelliteId::Gnss::GLONASS: return 1;
    case SatelliteId::Gnss::GALILEO: return 2;
    case SatelliteId::Gnss::BEIDOU: return 3;
    default: UNREACHABLE();
    }
}

static uint8_t gnss_from_id(long gnss_id) NOEXCEPT {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return 0;
    case GNSS_ID__gnss_id_galileo: return 2;
    case GNSS_ID__gnss_id_glonass: return 1;
    case GNSS_ID__gnss_id_bds: return 3;
    default: UNREACHABLE();
    }
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

void CorrectionCache::add_correction(long                                 gnss_id,
                                     GNSS_SSR_ClockCorrections_r15 const* correction) NOEXCEPT {
    if (!correction) return;
    FUNCTION_SCOPE();

    auto gnss           = gnss_from_id(gnss_id);
    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(correction->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(correction->epochTime_r15);

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssrUpdateInterval_r15(correction->ssrUpdateInterval_r15);
    if (update_interval > 1.0) {
        reference_time.add_seconds(update_interval * 0.5);
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = correction->ssr_ClockCorrectionList_r15.list;
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

        auto& sc       = get_or_create_satellite_correction(satellite_id);
        sc.clock_valid = true;
        sc.clock       = ClockCorrection{
                  .reference_time = reference_time,
                  .c0             = c0,
                  .c1             = c1,
                  .c2             = c2,
        };

#ifdef DATA_TRACING
        datatrace::report_ssr_clock_correction(reference_time, satellite_id.name(), c0, c1, c2,
                                               ssr_iod);
#endif

        VERBOSEF("clock: %4u %3s %+f %+f %+f", ssr_iod, satellite_id.name(), c0, c1, c2);
    }
}

void CorrectionCache::add_correction(long                                 gnss_id,
                                     GNSS_SSR_OrbitCorrections_r15 const* correction) NOEXCEPT {
    if (!correction) return;
    FUNCTION_SCOPE();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(correction->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(correction->epochTime_r15);

    auto reference_time  = epoch_time;
    auto update_interval = decode::ssrUpdateInterval_r15(correction->ssrUpdateInterval_r15);
    if (update_interval > 1.0) {
        reference_time.add_seconds(update_interval * 0.5);
    }

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = correction->ssr_OrbitCorrectionList_r15.list;
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

        auto& sc      = get_or_create_satellite_correction(satellite_id);
        sc.orbit[iod] = OrbitCorrection{
            .reference_time = reference_time,
            .iod            = iod,
            .delta          = {radial, along_track, cross_track},
            .dot_delta      = {dot_radial, dot_along, dot_cross},
        };

#ifdef DATA_TRACING
        auto delta     = Float3{radial, along_track, cross_track};
        auto dot_delta = Float3{dot_radial, dot_along, dot_cross};
        datatrace::report_ssr_orbit_correction(reference_time, satellite_id.name(), delta,
                                               dot_delta, ssr_iod, static_cast<long>(iod));
#endif

        VERBOSEF("orbit: %u4 %3s %+f %+f %+f [eph iod: %u]", ssr_iod, satellite_id.name(), radial,
                 along_track, cross_track, iod);
    }
}

void CorrectionCache::add_correction(long                         gnss_id,
                                     GNSS_SSR_CodeBias_r15 const* code_bias) NOEXCEPT {
    if (!code_bias) return;
    FUNCTION_SCOPE();

    auto gnss           = gnss_from_id(gnss_id);
    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(code_bias->iod_ssr_r15);
    auto epoch_time = decode::epochTime_r15(code_bias->epochTime_r15);

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

        auto& sc = get_or_create_satellite_correction(satellite_id);

        auto& signal_list = satellite->ssr_CodeBiasSignalList_r15.list;
        for (int j = 0; j < signal_list.count; j++) {
            auto signal = signal_list.array[j];
            if (!signal) continue;

            auto signal_id = signal_id_from(signal_gnss, signal->signal_and_tracking_mode_ID_r15);
            auto value     = decode::codeBias_r15(signal->codeBias_r15);
            sc.code_bias[signal_id] = value;

            VERBOSEF("code bias: %4u %3s %-16s %+f", ssr_iod, satellite_id.name(), signal_id.name(),
                     value);
        }
    }
}

void CorrectionCache::add_correction(long                         gnss_id,
                                     GNSS_SSR_PhaseBias_r16 const* correction) NOEXCEPT {
    if (!correction) return;
    FUNCTION_SCOPE();

    auto satellite_gnss = satellite_gnss_from_id(gnss_id);
    auto signal_gnss    = signal_gnss_from_id(gnss_id);

    auto ssr_iod    = decode::iod_ssr_r16(correction->iod_ssr_r16);
    auto epoch_time = decode::epochTime_r15(correction->epochTime_r16);

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = correction->ssr_PhaseBiasSatList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto satellite = list.array[i];
        if (!satellite) continue;

        auto satellite_id = SatelliteId::from_lpp(satellite_gnss, satellite->svID_r16.satellite_id);
        if (!satellite_id.is_valid()) {
            WARNF("phase bias rejected: invalid satellite id (%ld)",
                  satellite->svID_r16.satellite_id);
            continue;
        }

        auto& sc = get_or_create_satellite_correction(satellite_id);

        auto& signal_list = satellite->ssr_PhaseBiasSignalList_r16.list;
        for (int j = 0; j < signal_list.count; j++) {
            auto signal = signal_list.array[j];
            if (!signal) continue;

            auto signal_id = signal_id_from(signal_gnss, signal->signal_and_tracking_mode_ID_r16);
            auto value     = decode::phaseBias_r16(signal->phaseBias_r16);
            sc.phase_bias[signal_id] = value;

            VERBOSEF("phase bias: %4u %3s %-16s %+f", ssr_iod, satellite_id.name(), signal_id.name(),
                     value);
        }
    }
}

void CorrectionCache::add_correction(long                                gnss_id,
                                     GNSS_SSR_STEC_Correction_r16 const* correction) NOEXCEPT {
    if (!correction) return;
    FUNCTION_SCOPE();

    auto gnss           = gnss_from_id(gnss_id);
    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto cps_id = static_cast<uint16_t>(correction->correctionPointSetID_r16);
    auto cps    = get_correction_point_set(cps_id);
    if (!cps) {
        DEBUGF("correction point set not found: %u", cps_id);
        return;
    }

    auto ssr_iod    = decode::iod_ssr_r16(correction->iod_ssr_r16);
    auto epoch_time = decode::epochTime_r15(correction->epochTime_r16);

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    auto& list = correction->stec_SatList_r16.list;
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

        auto& poly                   = cps->ionospheric_polynomials[satellite_id];
        poly.c00                     = c00;
        poly.c01                     = c01;
        poly.c10                     = c10;
        poly.c11                     = c11;
        poly.reference_point         = cps->reference_point;
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
            cps->reference_point.x() * constant::r2d, cps->reference_point.y() * constant::r2d,
            quality_indicator, quality_indiciator_class, quality_indicator_value, ssr_iod);
#endif

        VERBOSEF("stec: %3s %+f %+f %+f %+f", satellite_id.name(), c00, c01, c10, c11);
    }
}

void CorrectionCache::add_correction(long                                  gnss_id,
                                     GNSS_SSR_GriddedCorrection_r16 const* correction) NOEXCEPT {
    if (!correction) return;
    FUNCTION_SCOPE();

    auto gnss           = gnss_from_id(gnss_id);
    auto satellite_gnss = satellite_gnss_from_id(gnss_id);

    auto cps_id = static_cast<uint16_t>(correction->correctionPointSetID_r16);
    auto cps    = get_correction_point_set(cps_id);
    if (!cps) {
        DEBUGF("correction point set not found: %u", cps_id);
        return;
    }

    auto ssr_iod    = decode::iod_ssr_r16(correction->iod_ssr_r16);
    auto epoch_time = decode::epochTime_r15(correction->epochTime_r16);

    VERBOSEF("epoch: %s", epoch_time.rtklib_time_string().c_str());

    std::vector<GridElement_r16*> grid_elements;

    auto& list = correction->gridList_r16.list;
    for (int i = 0; i < list.count; i++) {
        auto element = list.array[i];
        if (!element) continue;
        grid_elements.push_back(element);
    }

    for (long i = 0; i < cps->point_count; i++) {
        auto& point = cps->points[i];
        if (!point.valid) continue;                               // skip invalid
        if (point.array_index < 0) continue;                      // skip invalid
        if (point.array_index >= grid_elements.size()) continue;  // skip invalid

        auto element = grid_elements[point.array_index];

        VERBOSEF("grid: %ld/%ld %+3.14f %+3.14f", point.array_index, point.absolute_index,
                 point.position.x() * constant::r2d, point.position.y() * constant::r2d);

        if (element->tropospericDelayCorrection_r16) {
            auto& tropo = *element->tropospericDelayCorrection_r16;
            auto  dry =
                decode::tropoHydroStaticVerticalDelay_r16(tropo.tropoHydroStaticVerticalDelay_r16) +
                2.3;
            auto wet = decode::tropoWetVerticalDelay_r16(tropo.tropoWetVerticalDelay_r16) + 0.252;

            point.has_tropospheric = true;
            point.tropospheric_wet = wet;
            point.tropospheric_dry = dry;

#ifdef DATA_TRACING
            datatrace::report_ssr_tropospheric_grid(epoch_time, point.absolute_index,
                                                    Float3{point.position.x() * constant::r2d,
                                                           point.position.y() * constant::r2d,
                                                           point.position.z()},
                                                    wet, dry, ssr_iod);
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

                point.has_ionospheric                    = true;
                point.ionospheric_residual[satellite_id] = ionospheric;

#ifdef DATA_TRACING
                datatrace::report_ssr_ionospheric_grid(epoch_time, point.absolute_index,
                                                       Float3{point.position.x() * constant::r2d,
                                                              point.position.y() * constant::r2d,
                                                              point.position.z()},
                                                       satellite_id.name(), ionospheric, ssr_iod);
#endif

                VERBOSEF("  ionospheric: %3s %+f", satellite_id.name(), ionospheric);
            }
        }
    }
}

void CorrectionCache::process(LPP_Message const& message) NOEXCEPT {
    FUNCTION_SCOPE();

    if (!message.lpp_MessageBody) return;

    auto& body = *message.lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return;

    auto& pad = body.choice.c1.choice.provideAssistanceData;
    if (pad.criticalExtensions.present != ProvideAssistanceData__criticalExtensions_PR_c1) return;
    if (pad.criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return;

    auto& pad_message = pad.criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    find_corrections(pad_message);
}

void CorrectionCache::find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT {
    FUNCTION_SCOPE();
    if (!message.a_gnss_ProvideAssistanceData) return;
    if (!message.a_gnss_ProvideAssistanceData->gnss_GenericAssistData) return;

    auto& gad = *message.a_gnss_ProvideAssistanceData->gnss_GenericAssistData;
    for (int i = 0; i < gad.list.count; i++) {
        auto element = gad.list.array[i];
        if (!element) continue;

        auto gnss_id = element->gnss_ID.gnss_id;
        if (gnss_id != GNSS_ID__gnss_id_gps && gnss_id != GNSS_ID__gnss_id_galileo &&
            gnss_id != GNSS_ID__gnss_id_bds && gnss_id != GNSS_ID__gnss_id_glonass) {
            WARNF("unsupported GNSS ID: %d", gnss_id);
            continue;
        }

        if (element->ext2) {
            add_correction(gnss_id, element->ext2->gnss_SSR_OrbitCorrections_r15);
            add_correction(gnss_id, element->ext2->gnss_SSR_ClockCorrections_r15);
            add_correction(gnss_id, element->ext2->gnss_SSR_CodeBias_r15);
        }

        if (element->ext3) {
            add_correction(gnss_id, element->ext3->gnss_SSR_PhaseBias_r16);
            // add_correction(gnss_id, element->ext3->gnss_SSR_URA_r16);
            add_correction(gnss_id, element->ext3->gnss_SSR_STEC_Correction_r16);
            add_correction(gnss_id, element->ext3->gnss_SSR_GriddedCorrection_r16);
        }
    }
}

SatelliteCorrection& CorrectionCache::get_or_create_satellite_correction(SatelliteId id) NOEXCEPT {
    auto it = mSatelliteCorrections.find(id);
    if (it == mSatelliteCorrections.end()) {
        auto new_correction = std::make_unique<SatelliteCorrection>();
        it                  = mSatelliteCorrections.emplace(id, std::move(new_correction)).first;
        return *it->second;
    } else {
        return *it->second;
    }
}

SatelliteCorrection const* CorrectionCache::satellite_correction(SatelliteId id) const NOEXCEPT {
    auto it = mSatelliteCorrections.find(id);
    if (it == mSatelliteCorrections.end()) return nullptr;
    return it->second.get();
}

CorrectionPointSet const* CorrectionCache::correction_point_set(Vector3 const& llh) const NOEXCEPT {
    std::vector<CorrectionPointSet const*> correction_point_sets;
    for(auto& [id, point_set] : mCorrectionPointSets) {
        if (point_set->contains(llh)) {
            correction_point_sets.push_back(point_set.get());
        }
    }

    // Sort by updated by
    std::sort(correction_point_sets.begin(), correction_point_sets.end(), 
              [](CorrectionPointSet const* a, CorrectionPointSet const* b) {
                  return a->updated_at < b->updated_at;
              });

    if (correction_point_sets.empty()) return nullptr;
    return correction_point_sets.back();
}

CorrectionPointSet& CorrectionCache::create_correction_point_set(uint16_t id) NOEXCEPT {
    auto it = mCorrectionPointSets.find(id);
    if (it == mCorrectionPointSets.end()) {
        auto new_correction = std::make_unique<CorrectionPointSet>();
        it                  = mCorrectionPointSets.emplace(id, std::move(new_correction)).first;
        return *it->second;
    } else {
        return *it->second;
    }
}

CorrectionPointSet* CorrectionCache::get_correction_point_set(uint16_t id) NOEXCEPT {
    auto it = mCorrectionPointSets.find(id);
    if (it == mCorrectionPointSets.end()) return nullptr;
    return it->second.get();
}

}  // namespace idokeido
