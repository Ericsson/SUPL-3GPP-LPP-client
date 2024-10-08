#include "generator.hpp"
#include "constant.hpp"
#include "data.hpp"
#include "decode.hpp"
#include "helper.hpp"
#include "observation.hpp"
#include "satellite.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <A-GNSS-ProvideAssistanceData.h>
#include <GNSS-CommonAssistData.h>
#include <GNSS-GenericAssistData.h>
#include <GNSS-GenericAssistDataElement.h>
#include <GNSS-SSR-CorrectionPoints-r16.h>
#include <GNSS-SSR-OrbitCorrections-r15.h>
#include <LPP-Message.h>
#include <LPP-MessageBody.h>
#include <SSR-CodeBiasSatElement-r15.h>
#include <SSR-CodeBiasSignalElement-r15.h>
#include <SSR-PhaseBiasSatElement-r16.h>
#include <SSR-PhaseBiasSignalElement-r16.h>
#pragma GCC diagnostic pop

#include <map>
#include <math.h>
#include <unordered_map>

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/generator.hpp>
#include <generator/rtcm/rtk_data.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

namespace generator {
namespace tokoro {

Generator::Generator() NOEXCEPT {}

Generator::~Generator() NOEXCEPT = default;

void Generator::process_lpp(LPP_Message const& lpp_message) NOEXCEPT {
    VSCOPE_FUNCTION();

    if (!lpp_message.lpp_MessageBody) return;

    auto& body = *lpp_message.lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return;

    auto& pad = body.choice.c1.choice.provideAssistanceData;
    if (pad.criticalExtensions.present != ProvideAssistanceData__criticalExtensions_PR_c1) return;
    if (pad.criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return;

    auto& message = pad.criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    find_correction_point_set(message);

    if (!mCorrectionPointSet) {
        WARNF("no correction point set found");
        return;
    }

    mCorrectionData = std::unique_ptr<CorrectionData>(new CorrectionData());
    find_corrections(message);

    mLastCorrectionDataTime = mCorrectionData->latest_correction_time();
}

void Generator::find_correction_point_set(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT {
    if (!message.a_gnss_ProvideAssistanceData) return;
    if (!message.a_gnss_ProvideAssistanceData->gnss_CommonAssistData) return;

    auto& cad = *message.a_gnss_ProvideAssistanceData->gnss_CommonAssistData;
    if (!cad.ext2) return;
    if (!cad.ext2->gnss_SSR_CorrectionPoints_r16) return;

    auto& ssr = *cad.ext2->gnss_SSR_CorrectionPoints_r16;
    if (ssr.correctionPoints_r16.present ==
        GNSS_SSR_CorrectionPoints_r16__correctionPoints_r16_PR_arrayOfCorrectionPoints_r16) {
        auto& array                   = ssr.correctionPoints_r16.choice.arrayOfCorrectionPoints_r16;
        auto  correction_point_set_id = static_cast<uint16_t>(ssr.correctionPointSetID_r16);

        auto reference_point_latitude =
            decode::referencePointLatitude_r16(array.referencePointLatitude_r16);
        auto reference_point_longitude =
            decode::referencePointLongitude_r16(array.referencePointLongitude_r16);
        auto number_of_steps_latitude =
            decode::numberOfStepsLatitude_r16(array.numberOfStepsLatitude_r16);
        auto number_of_steps_longitude =
            decode::numberOfStepsLongitude_r16(array.numberOfStepsLongitude_r16);
        auto step_of_latitude  = decode::stepOfLatitude_r16(array.stepOfLatitude_r16);
        auto step_of_longitude = decode::stepOfLongitude_r16(array.stepOfLongitude_r16);

        auto grid_point_count = (number_of_steps_longitude + 1) * (number_of_steps_latitude + 1);

        CorrectionPointSet correction_point_set{};
        correction_point_set.set_id                    = correction_point_set_id;
        correction_point_set.grid_point_count          = grid_point_count;
        correction_point_set.reference_point_latitude  = reference_point_latitude;
        correction_point_set.reference_point_longitude = reference_point_longitude;
        correction_point_set.number_of_steps_latitude  = number_of_steps_latitude;
        correction_point_set.number_of_steps_longitude = number_of_steps_longitude;
        correction_point_set.step_of_latitude          = step_of_latitude;
        correction_point_set.step_of_longitude         = step_of_longitude;

        VERBOSEF("correction point set:");
        VERBOSEF("  set_id: %u", correction_point_set.set_id);
        VERBOSEF("  reference_point_latitude:  %.14f", correction_point_set.reference_point_latitude);
        VERBOSEF("  reference_point_longitude: %.14f", correction_point_set.reference_point_longitude);
        VERBOSEF("  step_of_latitude:  %.14f", correction_point_set.step_of_latitude);
        VERBOSEF("  step_of_longitude: %.14f", correction_point_set.step_of_longitude);

        uint64_t bitmask = 0;
        if (array.bitmaskOfGrids_r16) {
            for (size_t i = 0; i < array.bitmaskOfGrids_r16->size; i++) {
                bitmask <<= 8;
                bitmask |= static_cast<uint64_t>(array.bitmaskOfGrids_r16->buf[i]);
            }
            bitmask >>= array.bitmaskOfGrids_r16->bits_unused;
#ifdef SPARTN_DEBUG_PRINT
            printf(" bitmask: %ld bytes, %d bits, 0x%016lX\n", array.bitmaskOfGrids_r16->size,
                   array.bitmaskOfGrids_r16->bits_unused, bitmask);
#endif
        } else {
            bitmask = 0xFFFFFFFFFFFFFFFF;
        }
        correction_point_set.bitmask = bitmask;

        mCorrectionPointSet.reset(new CorrectionPointSet(correction_point_set));
    } else {
        // TODO(ewasjon): [low-priority] Support list of correction points
    }
}

void Generator::find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT {
    if (!message.a_gnss_ProvideAssistanceData) return;
    if (!message.a_gnss_ProvideAssistanceData->gnss_GenericAssistData) return;

    ASSERT(mCorrectionData, "correction data not initialized");
    ASSERT(mCorrectionPointSet, "correction point set not initialized");

    auto& gad = *message.a_gnss_ProvideAssistanceData->gnss_GenericAssistData;
    for (int i = 0; i < gad.list.count; i++) {
        auto element = gad.list.array[i];
        if (!element) continue;

        auto gnss_id = element->gnss_ID.gnss_id;
        if (!mGenerateGps && gnss_id == GNSS_ID__gnss_id_gps) {
            VERBOSEF("skipping GPS");
            continue;
        } else if (!mGenerateGal && gnss_id == GNSS_ID__gnss_id_galileo) {
            VERBOSEF("skipping Galileo");
            continue;
        } else if (!mGenerateBds && gnss_id == GNSS_ID__gnss_id_bds) {
            VERBOSEF("skipping BeiDou");
            continue;
        } else if (!mGenerateGlo && gnss_id == GNSS_ID__gnss_id_glonass) {
            VERBOSEF("skipping GLONASS");
            continue;
        }

        if (gnss_id != GNSS_ID__gnss_id_gps && gnss_id != GNSS_ID__gnss_id_galileo &&
            gnss_id != GNSS_ID__gnss_id_bds && gnss_id != GNSS_ID__gnss_id_glonass) {
            WARNF("unsupported GNSS ID: %d", gnss_id);
            continue;
        }

        if (element->ext2) {
            mCorrectionData->add_correction(gnss_id, element->ext2->gnss_SSR_OrbitCorrections_r15);
            mCorrectionData->add_correction(gnss_id, element->ext2->gnss_SSR_ClockCorrections_r15);
            mCorrectionData->add_correction(gnss_id, element->ext2->gnss_SSR_CodeBias_r15);
        }

        if (element->ext3) {
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_PhaseBias_r16);
            // mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_URA_r16);
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_STEC_Correction_r16,
                                            *mCorrectionPointSet);
            mCorrectionData->add_correction(gnss_id, element->ext3->gnss_SSR_GriddedCorrection_r16,
                                            *mCorrectionPointSet);
        }
    }
}

void Generator::create_satellites() NOEXCEPT {
    mSatellites.clear();

    if (mGenerateGps) {
        for (auto& kvp : mGpsEphemeris) {
            create_satellite(kvp.first);
        }
    }

    if (mGenerateGal) {
        for (auto& kvp : mGalEphemeris) {
            create_satellite(kvp.first);
        }
    }

    if (mGenerateBds) {
        for (auto& kvp : mBdsEphemeris) {
            create_satellite(kvp.first);
        }
    }
}

bool Generator::find_ephemeris(SatelliteId sv_id, ephemeris::Ephemeris& eph) NOEXCEPT {
    if (sv_id.gnss() == SatelliteId::Gnss::GPS) {
        auto it = mGpsEphemeris.find(sv_id);
        if (it != mGpsEphemeris.end()) {
            eph = ephemeris::Ephemeris(it->second);
            return true;
        }
    } else if (sv_id.gnss() == SatelliteId::Gnss::GALILEO) {
        auto it = mGalEphemeris.find(sv_id);
        if (it != mGalEphemeris.end()) {
            eph = ephemeris::Ephemeris(it->second);
            return true;
        }
    } else if (sv_id.gnss() == SatelliteId::Gnss::BEIDOU) {
        auto it = mBdsEphemeris.find(sv_id);
        if (it != mBdsEphemeris.end()) {
            eph = ephemeris::Ephemeris(it->second);
            return true;
        }
    }

    UNREACHABLE();
    return false;
}

void Generator::create_satellite(SatelliteId sv_id) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", sv_id.name());
    auto it = mSatellites.find(sv_id);
    if (it != mSatellites.end()) {
        VERBOSEF("satellite already exists");
        return;
    }

    if (mSatelliteIncludeSet.size() > 0 &&
        mSatelliteIncludeSet.find(sv_id) == mSatelliteIncludeSet.end()) {
        VERBOSEF("satellite not included");
        return;
    }

    ephemeris::Ephemeris eph{};
    if (!find_ephemeris(sv_id, eph)) {
        WARNF("ephemeris not found");
        return;
    }

    if (!eph.is_valid(mTimeReception)) {
        WARNF("ephemeris outside validity period");
        return;
    }

    auto satellite =
        std::unique_ptr<Satellite>(new Satellite(sv_id, eph, mTimeReception, mEcefLocation));

    satellite->find_orbit_correction(*mCorrectionData);
    satellite->find_clock_correction(*mCorrectionData);
    if (!satellite->compute_true_position()) {
        WARNF("failed to compute true position");
        return;
    }

    if (!satellite->compute_azimuth_and_elevation()) {
        WARNF("failed to compute azimuth and elevation");
        return;
    }

    if (satellite->elevation() < 15.0 * constant::DEG2RAD) {
        WARNF("rejected: elevation too low (%.2f)", satellite->elevation() * constant::RAD2DEG);
        return;
    }

    mSatellites[sv_id] = std::move(satellite);
}

void Generator::process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    auto satellite_id = SatelliteId::from_gps_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id");
        return;
    }

    DEBUGF("ephemeris: %s", satellite_id.name());
    mGpsEphemeris[satellite_id] = ephemeris;
}

void Generator::process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    auto satellite_id = SatelliteId::from_gal_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id");
        return;
    }

    DEBUGF("ephemeris: %s", satellite_id.name());
    mGalEphemeris[satellite_id] = ephemeris;
}

void Generator::process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    auto satellite_id = SatelliteId::from_bds_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id");
        return;
    }

    DEBUGF("ephemeris: %s", satellite_id.name());
    mBdsEphemeris[satellite_id] = ephemeris;
}

bool Generator::generate_observation(Satellite const& satellite, SignalId signal_id) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", satellite.id().name(), signal_id.name());
    if (mSignalIncludeSet.size() > 0 &&
        mSignalIncludeSet.find(signal_id) == mSignalIncludeSet.end()) {
        VERBOSEF("signal not included");
        return false;
    }

    Observation observation{satellite, signal_id, mEcefLocation};
    observation.compute_phase_bias(*mCorrectionData);
    observation.compute_code_bias(*mCorrectionData);
    observation.compute_tropospheric(mEcefLocation, *mCorrectionData);
    observation.compute_ionospheric(mEcefLocation, *mCorrectionData);

    VERBOSEF("observation: p=%f, c=%f", observation.pseudorange(), observation.carrier_cycle());

    if (!observation.has_phase_bias()) {
        WARNF("discarded (no phase bias)");
        return false;
    } else if (!observation.has_code_bias()) {
        WARNF("discarded (no code bias)");
        return false;
    }

    if (!observation.has_tropospheric()) {
        WARNF("missing tropospheric correction");
    }
    if (!observation.has_ionospheric()) {
        WARNF("missing ionospheric correction");
    }

    mObservations.push_back(observation);
    return true;
}

void Generator::build_rtcm_satellite(Satellite const&    satellite,
                                     rtcm::Observations& observations) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", satellite.id().name());

    auto satellite_observation_count = 0;
    for (auto const& observation : mObservations) {
        if (observation.sv_id() != satellite.id()) continue;
        satellite_observation_count++;
    }

    if (satellite_observation_count == 0) {
        VERBOSEF("no observations for satellite %s", satellite.id().name());
        return;
    }

    auto rtd = range_time_division(satellite.pseudorange());
    VERBOSEF("rtd:");
    VERBOSEF("  integer_ms:   %d", rtd.integer_ms);
    VERBOSEF("  rough_range:  %.14f", rtd.rough_range);
    VERBOSEF("  used_range:   %.14f", rtd.used_range);
    VERBOSEF("  unused_range: %.14f", rtd.unused_range);

    rtcm::Satellite rtcm{};
    rtcm.id          = satellite.id();
    rtcm.integer_ms  = rtd.integer_ms;
    rtcm.rough_range = rtd.rough_range;
    observations.satellites.push_back(rtcm);

    for (auto const& observation : mObservations) {
        if (observation.sv_id() != satellite.id()) continue;

        if (observation.signal_id().gnss() == SignalId::Gnss::GPS) {
            observations.gps_observation_count++;
        } else if (observation.signal_id().gnss() == SignalId::Gnss::GALILEO) {
            observations.galileo_observation_count++;
        } else if (observation.signal_id().gnss() == SignalId::Gnss::BEIDOU) {
            observations.beidou_observation_count++;
        } else if (observation.signal_id().gnss() == SignalId::Gnss::GLONASS) {
            observations.glonass_observation_count++;
        }

        auto meter_to_cms   = 1.0e3 / constant::SPEED_OF_LIGHT;
        auto code_range_ms  = observation.pseudorange() * meter_to_cms;
        auto phase_range_ms = observation.carrier_cycle() * meter_to_cms;

        VERBOSEF("code_range: %.14f", code_range_ms);
        VERBOSEF("phase_range: %.14f", phase_range_ms);

        auto delta_code_range_ms  = code_range_ms - rtd.used_range;
        auto delta_phase_range_ms = phase_range_ms - rtd.used_range;

        VERBOSEF("delta_code_range: %.14f", delta_code_range_ms);
        VERBOSEF("delta_phase_range: %.14f", delta_phase_range_ms);

        rtcm::Signal signal{};
        signal.id                     = observation.signal_id();
        signal.satellite              = satellite.id();
        signal.fine_pseudo_range      = delta_code_range_ms;
        signal.fine_phase_range       = delta_phase_range_ms;
        signal.carrier_to_noise_ratio = 47.0;   // TODO(ewasjon): How do we choose this value?
        signal.lock_time              = 525.0;  // TODO: How do we determine this value?
        observations.signals.push_back(signal);
    }
}

void Generator::build_rtcm_observations(rtcm::Observations& observations) NOEXCEPT {
    VSCOPE_FUNCTION();

    for (auto& kvp : mSatellites) {
        build_rtcm_satellite(*kvp.second.get(), observations);
    }
}

void Generator::build_rtcm_messages(std::vector<rtcm::Message>& messages) NOEXCEPT {
    VSCOPE_FUNCTION();

    if (mObservations.empty()) {
        WARNF("no observations available");
        return;
    }

    rtcm::Observations observations{};
    observations.time = mTimeReception;
    build_rtcm_observations(observations);

    DEBUGF("satellites: %zu, signals: %zu", observations.satellites.size(),
           observations.signals.size());

    rtcm::ReferenceStation reference_station{};
    reference_station.reference_station_id = 1;
    reference_station.x                    = mEcefLocation.x;
    reference_station.y                    = mEcefLocation.y;
    reference_station.z                    = mEcefLocation.z;

    messages.push_back(rtcm::generate_1005(reference_station, true, false, false));

    rtcm::CommonObservationInfo common{};
    common.reference_station_id = 1;

    if (mGenerateGps && observations.gps_observation_count > 0) {
        auto gps_message =
            rtcm::generate_msm(7, true, rtcm::GenericGnssId::GPS, common, observations);
        messages.push_back(std::move(gps_message));
    }

    if (mGenerateGal && observations.galileo_observation_count > 0) {
        auto gal_message =
            rtcm::generate_msm(7, true, rtcm::GenericGnssId::GALILEO, common, observations);
        messages.push_back(std::move(gal_message));
    }

    if (mGenerateBds && observations.beidou_observation_count > 0) {
        auto bds_message =
            rtcm::generate_msm(7, true, rtcm::GenericGnssId::BEIDOU, common, observations);
        messages.push_back(std::move(bds_message));
    }
}

std::vector<rtcm::Message> Generator::generate(ts::Tai const& time_reception,
                                               EcefPosition   position_reception) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, (%f, %f, %f)", ts::Utc(time_reception).rtklib_time_string().c_str(),
                     position_reception.x, position_reception.y, position_reception.z);

    std::vector<rtcm::Message> messages{};
    if (!mCorrectionData) return messages;

    mEcefLocation  = position_reception;
    mWgs84Location = ecef_to_wgs84(position_reception);
    mTimeReception = time_reception;
    DEBUGF("location: ecef  (%f, %f, %f)", mEcefLocation.x, mEcefLocation.y, mEcefLocation.z);
    DEBUGF("location: wgs84 (%f, %f, %f)", mWgs84Location.x * constant::RAD2DEG,
           mWgs84Location.y * constant::RAD2DEG, mWgs84Location.z);

    create_satellites();
    DEBUGF("%lu satellites available", mSatellites.size());

    // generate observations
    mObservations.clear();
    for (auto& kvp : mSatellites) {
        auto signals = mCorrectionData->signals(kvp.first);
        if (!signals) continue;

        for (auto& signal : *signals) {
            generate_observation(*kvp.second.get(), signal);
        }
    }

    // generate RTCM messages
    build_rtcm_messages(messages);

    return messages;
}

}  // namespace tokoro
}  // namespace generator
