#include "generator.hpp"
#include "constant.hpp"
#include "coordinate.hpp"
#include "data/correction.hpp"
#include "decode.hpp"
#include "models/helper.hpp"
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

#include <algorithm>
#include <map>
#include <math.h>
#include <unordered_map>

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/generator.hpp>
#include <generator/rtcm/rtk_data.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE(tokoro);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(tokoro)

namespace generator {
namespace tokoro {

ReferenceStation::ReferenceStation(Generator&                    generator,
                                   ReferenceStationConfig const& config) NOEXCEPT
    : mGroundPosition(config.itrf_ground_position),
      mRtcmGroundPosition(config.rtcm_ground_position),
      mRtcmPhysicalGroundPosition(config.rtcm_ground_position),
      mRtcmPhysicalGroundPositionSet(false),
      mGenerateGps(config.generate_gps),
      mGenerateGlo(config.generate_glo),
      mGenerateGal(config.generate_gal),
      mGenerateBds(config.generate_bds),
      mShapiroCorrection(true),
      mEarthSolidTidesCorrection(false),
      mPhaseWindupCorrection(false),
      mAntennaPhaseVariation(false),
      mTropoHeightCorrection(false),
      mElevationMask(10.0),
      mPhaseRangeRate(true),
      mRtcmReferenceStationId(1902),
      mRtcmMsmType(5),
      mNegativePhaseWindup(false),
      mGenerateRinex(false),
      mRequireCodeBias(true),
      mRequirePhaseBias(true),
      mRequireTropo(true),
      mRequireIono(true),
      mUseTroposphericModel(false),
      mUseIonosphericHeightCorrection(false),
      mRinexBuilder(ts::Utc::now().rinex_filename() + ".rnx", 3.04),
      mGenerator(generator) {
    // Initialize the satellite vector to the maximum number of satellites
    // GPS: 32, GLONASS: 24, GALILEO: 36, BEIDOU: 35
    mSatellites.reserve(32 + 24 + 36 + 35);

    // Initialize the satellites
    initialize_satellites();

    mGenerationTime = ts::Tai::now();
    mLastRinexEpoch = mGenerationTime;
}

ReferenceStation::~ReferenceStation() = default;

void ReferenceStation::initialize_satellites() NOEXCEPT {
    FUNCTION_SCOPE();

    // GPS
    if (mGenerateGps) {
        for (uint8_t i = 1; i <= 32; i++) {
            auto id = SatelliteId::from_gps_prn(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
    }

    // TODO(ewasjon): GLONASS
    if (mGenerateGlo) {
    }

    // GALILEO
    if (mGenerateGal) {
        for (uint8_t i = 1; i <= 36; i++) {
            auto id = SatelliteId::from_gal_prn(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
    }

    // BEIDOU
    if (mGenerateBds) {
        for (uint8_t i = 1; i <= 63; i++) {
            auto id = SatelliteId::from_bds_prn(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
    }
}

void ReferenceStation::initialize_observation(Satellite& satellite, SignalId signal_id) NOEXCEPT {
    FUNCTION_SCOPE();

    // Update lock tracking
    SatelliteSignalId ss_id{satellite.id(), signal_id};
    LockTime          lock_time{};
    if (mLockTime.find(ss_id) == mLockTime.end()) {
        lock_time.time    = mGenerationTime;
        lock_time.seconds = 0;
        lock_time.lost    = true;
    } else {
        lock_time.time    = mLockTime[ss_id];
        lock_time.seconds = mGenerationTime.difference_seconds(lock_time.time);
        lock_time.lost    = false;
    }

    auto correction_data = *mGenerator.mCorrectionData;
    auto antex           = mGenerator.mAntex.get();

    auto& observation = satellite.initialize_observation(signal_id);
    observation.update_lock_time(lock_time);
    observation.compute_phase_bias(correction_data);
    observation.compute_code_bias(correction_data);
    observation.compute_tropospheric(correction_data);
    observation.compute_ionospheric(correction_data);

    if (mAntennaPhaseVariation && antex) observation.compute_antenna_phase_variation(*antex);
    if (mTropoHeightCorrection) observation.compute_tropospheric_height();

    observation.set_negative_phase_windup(mNegativePhaseWindup);
    observation.set_require_code_bias(mRequireCodeBias);
    observation.set_require_phase_bias(mRequirePhaseBias);
    observation.set_require_tropo(mRequireTropo);
    observation.set_require_iono(mRequireIono);
    observation.set_use_tropospheric_model(mUseTroposphericModel);
    observation.set_use_ionospheric_height_correction(mUseIonosphericHeightCorrection);
    observation.compute_ranges();

    if (!observation.is_valid()) return;
    VERBOSEF("observation: c=%f, p=%f", observation.code_range(), observation.phase_range());
}

bool ReferenceStation::generate(ts::Tai const& reception_time) NOEXCEPT {
    FUNCTION_SCOPE();
    if (mGenerator.mCorrectionData == nullptr) {
        WARNF("no correction data available");
        return false;
    }

    mGenerationTime = reception_time;

    DEBUGF("generation time: %s", mGenerationTime.rtklib_time_string().c_str());
    DEBUGF("satellite count: %zu", mSatellites.size());

    // Update the satellites
    for (auto& satellite : mSatellites) {
        if (mSatelliteIncludeSet.size() > 0 &&
            mSatelliteIncludeSet.find(satellite.id()) == mSatelliteIncludeSet.end()) {
            WARNF("discarded: %s - not included", satellite.id().name());
            satellite.disable();
            continue;
        }

        satellite.update(mGenerationTime);
    }

    // Generate the observations
    std::unordered_set<SatelliteSignalId> active_signals;
    for (auto& satellite : mSatellites) {
        satellite.reset_observations();

        if (!satellite.enabled()) continue;

        satellite.compute_sun_position();
        if (mShapiroCorrection) satellite.compute_shapiro();
        if (mEarthSolidTidesCorrection) satellite.compute_earth_solid_tides();
        if (mPhaseWindupCorrection) satellite.compute_phase_windup();
        satellite.datatrace_report();

        if (mSatelliteIncludeSet.size() > 0 &&
            mSatelliteIncludeSet.find(satellite.id()) == mSatelliteIncludeSet.end()) {
            WARNF("discarded: %s - not included", satellite.id().name());
            satellite.disable();
            continue;
        }

        if (satellite.elevation() * constant::RAD2DEG < mElevationMask) {
            WARNF("discarded: %s - elevation mask (%.2f < %.2f)", satellite.id().name(),
                  satellite.elevation() * constant::RAD2DEG, mElevationMask);
            satellite.disable();
            continue;
        }

        auto signals = mGenerator.mCorrectionData->signals(satellite.id());
        if (!signals) continue;

        for (auto& signal : *signals) {
            if (mSignalIncludeSet.size() > 0 &&
                mSignalIncludeSet.find(signal) == mSignalIncludeSet.end()) {
                WARNF("discarded: %s %s - not included", satellite.id().name(), signal.name());
                continue;
            }

            if (signal.frequency() <= 1.0) {
                WARNF("discarded: %s %s - invalid frequency", satellite.id().name(), signal.name());
                continue;
            }

            initialize_observation(satellite, signal);
            satellite.remove_discarded_observations();
        }

        if (satellite.observations().size() == 0) {
            WARNF("discarded: %s - no valid observations", satellite.id().name());
            satellite.disable();
            continue;
        }

        for (auto const& observation : satellite.observations()) {
            if (!observation.is_valid()) continue;
            active_signals.insert(observation.ss_id());
        }

        DEBUGF("satellite %s: %zu observations", satellite.id().name(),
               satellite.observations().size());
    }

    // Update the lock time
    std::vector<SatelliteSignalId> lost_lock;
    for (auto& id : mLockTime) {
        if (active_signals.find(id.first) == active_signals.end()) {
            lost_lock.push_back(id.first);
        }
    }

    VERBOSEF("lli: active=%zu, lost=%zu, total=%zu", active_signals.size(), lost_lock.size(),
             mLockTime.size());
    for (auto& id : lost_lock) {
        VERBOSEF("lli -%s", id.to_string().c_str());
        mLockTime.erase(id);
    }

    for (auto& id : active_signals) {
        if (mLockTime.find(id) == mLockTime.end()) {
            VERBOSEF("lli +%s", id.to_string().c_str());
            mLockTime[id] = mGenerationTime;
        }
    }

    return true;
}

void ReferenceStation::build_rtcm_observation(Satellite const&         satellite,
                                              Observation const&       observation,
                                              RangeTimeDivision const& rtd,
                                              double                   reference_phase_range_rate,
                                              rtcm::Observations&      observations) NOEXCEPT {
    FUNCTION_SCOPEF("%s", observation.signal_id().name());

    auto code_range  = observation.code_range();
    auto phase_range = observation.phase_range();

    auto meter_to_cms   = 1.0e3 / constant::SPEED_OF_LIGHT;
    auto code_range_ms  = code_range * meter_to_cms;
    auto phase_range_ms = phase_range * meter_to_cms;

    auto delta_code_range_ms  = code_range_ms - rtd.used_range;
    auto delta_phase_range_ms = phase_range_ms - rtd.used_range;

    VERBOSEF("%-15s code:  %+.14f (%+.14f)", observation.signal_id().name(), code_range_ms,
             delta_code_range_ms);
    VERBOSEF("%-15s phase: %+.14f (%+.14f)", observation.signal_id().name(), phase_range_ms,
             delta_phase_range_ms);

    rtcm::Signal signal{};
    signal.id                     = observation.signal_id();
    signal.satellite              = satellite.id();
    signal.fine_pseudo_range      = delta_code_range_ms;
    signal.fine_phase_range       = delta_phase_range_ms;
    signal.carrier_to_noise_ratio = observation.carrier_to_noise_ratio();
    signal.lock_time              = observation.lock_time().seconds;

    if (mPhaseRangeRate) {
        auto phase_range_rate       = observation.phase_range_rate();
        auto delta_phase_range_rate = phase_range_rate - reference_phase_range_rate;
        VERBOSEF("%-15s phase rate: %+.14f (%+.14f)", observation.signal_id().name(),
                 phase_range_rate, delta_phase_range_rate);

        signal.fine_phase_range_rate = delta_phase_range_rate;
    }

    observations.signals.push_back(signal);
}

void ReferenceStation::build_rtcm_satellite(Satellite const&    satellite,
                                            rtcm::Observations& observations) NOEXCEPT {
    FUNCTION_SCOPEF("%s, observations=%zu", satellite.id().name(), satellite.observations().size());

    auto average_code_range = satellite.average_code_range();
    auto rtd                = range_time_division(average_code_range);
    VERBOSEF("avg_code_range:       %.4f (%d, %.4f)", rtd.used_range, rtd.integer_ms,
             rtd.rough_range);

    auto phase_range_rate = 0.0;
    if (mPhaseRangeRate) {
        // Round average phase_range_rate to the nearest integer
        auto avg_phase_range_rate = satellite.average_phase_range_rate();
        phase_range_rate          = std::round(avg_phase_range_rate);
        VERBOSEF("avg_phase_range_rate: %.4f (%f)", avg_phase_range_rate, phase_range_rate);
    }

    rtcm::Satellite rtcm{};
    rtcm.id          = satellite.id();
    rtcm.integer_ms  = rtd.integer_ms;
    rtcm.rough_range = rtd.rough_range;
    if (mPhaseRangeRate) {
        rtcm.rough_phase_range_rate = phase_range_rate;
    }
    observations.satellites.push_back(rtcm);

    for (auto const& observation : satellite.observations()) {
        if (!observation.is_valid()) continue;
        build_rtcm_observation(satellite, observation, rtd, phase_range_rate, observations);
    }
}

std::vector<rtcm::Message> ReferenceStation::produce() NOEXCEPT {
    FUNCTION_SCOPE();

    std::vector<rtcm::Message> messages;

    rtcm::ReferenceStation reference_station{};
    reference_station.reference_station_id          = mRtcmReferenceStationId;
    reference_station.x                             = mRtcmGroundPosition.x;
    reference_station.y                             = mRtcmGroundPosition.y;
    reference_station.z                             = mRtcmGroundPosition.z;
    reference_station.is_physical_reference_station = false;
    reference_station.antenna_height                = 0.0;

    messages.push_back(
        rtcm::generate_1006(reference_station, mGenerateGps, mGenerateGlo, mGenerateGal));

    if (mRtcmPhysicalGroundPositionSet) {
        auto physical_reference_station_id = 4095u;
        if (mRtcmReferenceStationId > 1) {
            physical_reference_station_id = mRtcmReferenceStationId - 1;
        }

        rtcm::PhysicalReferenceStation physical_reference_station{};
        physical_reference_station.reference_station_id = physical_reference_station_id;
        physical_reference_station.x                    = mRtcmPhysicalGroundPosition.x;
        physical_reference_station.y                    = mRtcmPhysicalGroundPosition.y;
        physical_reference_station.z                    = mRtcmPhysicalGroundPosition.z;
        messages.push_back(rtcm::generate_1032(reference_station, physical_reference_station));
    }

    rtcm::CommonObservationInfo common{};
    common.reference_station_id = mRtcmReferenceStationId;
    common.clock_steering       = 1;

    rtcm::Observations msm_gps{};
    rtcm::Observations msm_glo{};
    rtcm::Observations msm_gal{};
    rtcm::Observations msm_bds{};

    msm_gps.time = mGenerationTime;
    msm_glo.time = mGenerationTime;
    msm_gal.time = mGenerationTime;
    msm_bds.time = mGenerationTime;

    if (mGenerateRinex) {
        if (mLastRinexEpoch < mGenerationTime) {
            mRinexBuilder.set_antenna_position(mRtcmGroundPosition);
            mRinexBuilder.set_gps_support(mGenerateGps);
            mRinexBuilder.set_glo_support(mGenerateGlo);
            mRinexBuilder.set_gal_support(mGenerateGal);
            mRinexBuilder.set_bds_support(mGenerateBds);

            std::vector<SatelliteId> rinex_satellites;
            for (auto& satellite : mSatellites) {
                if (!satellite.enabled()) continue;
                if (!mGenerateGps && satellite.id().gnss() == SatelliteId::Gnss::GPS) continue;
                if (!mGenerateGlo && satellite.id().gnss() == SatelliteId::Gnss::GLONASS) continue;
                if (!mGenerateGal && satellite.id().gnss() == SatelliteId::Gnss::GALILEO) continue;
                if (!mGenerateBds && satellite.id().gnss() == SatelliteId::Gnss::BEIDOU) continue;
                rinex_satellites.push_back(satellite.id());
            }

            mRinexBuilder.epoch(mGenerationTime, rinex_satellites);

            std::unordered_map<format::rinex::ObservationType, double> rinex_observations;
            for (auto& satellite : mSatellites) {
                if (!satellite.enabled()) continue;
                if (!mGenerateGps && satellite.id().gnss() == SatelliteId::Gnss::GPS) continue;
                if (!mGenerateGlo && satellite.id().gnss() == SatelliteId::Gnss::GLONASS) continue;
                if (!mGenerateGal && satellite.id().gnss() == SatelliteId::Gnss::GALILEO) continue;
                if (!mGenerateBds && satellite.id().gnss() == SatelliteId::Gnss::BEIDOU) continue;
                rinex_observations.clear();

                std::unordered_set<SignalId> loss_signals;
                for (auto& observation : satellite.observations()) {
                    format::rinex::ObservationType type;
                    type.kind                = format::rinex::ObservationKind::Code;
                    type.signal_id           = observation.signal_id();
                    rinex_observations[type] = observation.code_range();

                    type.kind = format::rinex::ObservationKind::Phase;
                    rinex_observations[type] =
                        observation.phase_range() / observation.wave_length();

                    type.kind                = format::rinex::ObservationKind::SignalStrength;
                    rinex_observations[type] = observation.carrier_to_noise_ratio();

                    if (observation.lock_time().lost) {
                        loss_signals.insert(observation.signal_id());
                    }
                }

                mRinexBuilder.observations(satellite.id(), rinex_observations, loss_signals);
            }

            mLastRinexEpoch = mGenerationTime;
        }
    }

    if (mGenerateGps) {
        for (auto& satellite : mSatellites) {
            if (!satellite.enabled()) continue;
            if (satellite.id().gnss() != SatelliteId::Gnss::GPS) continue;
            build_rtcm_satellite(satellite, msm_gps);
        }
    }

    if (mGenerateGlo) {
        for (auto& satellite : mSatellites) {
            if (!satellite.enabled()) continue;
            if (satellite.id().gnss() != SatelliteId::Gnss::GLONASS) continue;
            build_rtcm_satellite(satellite, msm_glo);
        }
    }

    if (mGenerateGal) {
        for (auto& satellite : mSatellites) {
            if (!satellite.enabled()) continue;
            if (satellite.id().gnss() != SatelliteId::Gnss::GALILEO) continue;
            build_rtcm_satellite(satellite, msm_gal);
        }
    }

    if (mGenerateBds) {
        for (auto& satellite : mSatellites) {
            if (!satellite.enabled()) continue;
            if (satellite.id().gnss() != SatelliteId::Gnss::BEIDOU) continue;
            build_rtcm_satellite(satellite, msm_bds);
        }
    }

    DEBUGF("GPS: satellites=%2zu, signals=%2zu", msm_gps.satellites.size(), msm_gps.signals.size());
    DEBUGF("GLO: satellites=%2zu, signals=%2zu", msm_glo.satellites.size(), msm_glo.signals.size());
    DEBUGF("GAL: satellites=%2zu, signals=%2zu", msm_gal.satellites.size(), msm_gal.signals.size());
    DEBUGF("BDS: satellites=%2zu, signals=%2zu", msm_bds.satellites.size(), msm_bds.signals.size());
    auto will_generate_gps =
        mGenerateGps && msm_gps.satellites.size() > 0 && msm_gps.signals.size() > 0;
    auto will_generate_glo =
        mGenerateGlo && msm_glo.satellites.size() > 0 && msm_glo.signals.size() > 0;
    auto will_generate_gal =
        mGenerateGal && msm_gal.satellites.size() > 0 && msm_gal.signals.size() > 0;
    auto will_generate_bds =
        mGenerateBds && msm_bds.satellites.size() > 0 && msm_bds.signals.size() > 0;

    if (will_generate_gps) {
        auto last_msm = !will_generate_glo && !will_generate_gal && !will_generate_bds;
        auto message =
            rtcm::generate_msm(mRtcmMsmType, last_msm, rtcm::GenericGnssId::GPS, common, msm_gps);
        messages.push_back(std::move(message));
    }

    if (will_generate_glo) {
        auto last_msm = !will_generate_gal && !will_generate_bds;
        auto message  = rtcm::generate_msm(mRtcmMsmType, last_msm, rtcm::GenericGnssId::GLONASS,
                                           common, msm_glo);
        messages.push_back(std::move(message));
    }

    if (will_generate_gal) {
        auto last_msm = !will_generate_bds;
        auto message  = rtcm::generate_msm(mRtcmMsmType, last_msm, rtcm::GenericGnssId::GALILEO,
                                           common, msm_gal);
        messages.push_back(std::move(message));
    }

    if (will_generate_bds) {
        auto message =
            rtcm::generate_msm(mRtcmMsmType, true, rtcm::GenericGnssId::BEIDOU, common, msm_bds);
        messages.push_back(std::move(message));
    }

    return messages;
}

//
//
//

Generator::Generator() NOEXCEPT {
    FUNCTION_SCOPE();
    mIodConsistencyCheck                         = false;
    mUseReceptionTimeForOrbitAndClockCorrections = false;
    mUseOrbitCorrectionInIteration               = false;
    mIgnoreBitmask                               = false;
}

Generator::~Generator() = default;

std::shared_ptr<ReferenceStation>
Generator::define_reference_station(ReferenceStationConfig const& config) NOEXCEPT {
    FUNCTION_SCOPE();
    INFOF("define reference station:");
    INFOF("  ground position (itrf): (%f, %f, %f)", config.itrf_ground_position.x,
          config.itrf_ground_position.y, config.itrf_ground_position.z);
    INFOF("  ground position (rtcm): (%f, %f, %f)", config.rtcm_ground_position.x,
          config.rtcm_ground_position.y, config.rtcm_ground_position.z);
    INFOF("  gnss: %s%s%s%s", config.generate_gps ? "G" : "-", config.generate_glo ? "R" : "-",
          config.generate_gal ? "E" : "-", config.generate_bds ? "C" : "-");

    return std::make_shared<ReferenceStation>(*this, config);
}

bool Generator::process_lpp(LPP_Message const& lpp_message) NOEXCEPT {
    FUNCTION_SCOPE();

    if (!lpp_message.lpp_MessageBody) return false;

    auto& body = *lpp_message.lpp_MessageBody;
    if (body.present != LPP_MessageBody_PR_c1) return false;
    if (body.choice.c1.present != LPP_MessageBody__c1_PR_provideAssistanceData) return false;

    auto& pad = body.choice.c1.choice.provideAssistanceData;
    if (pad.criticalExtensions.present != ProvideAssistanceData__criticalExtensions_PR_c1)
        return false;
    if (pad.criticalExtensions.choice.c1.present !=
        ProvideAssistanceData__criticalExtensions__c1_PR_provideAssistanceData_r9)
        return false;

    auto& message = pad.criticalExtensions.choice.c1.choice.provideAssistanceData_r9;
    find_correction_point_set(message);

    if (!mCorrectionPointSet) {
        WARNF("no correction point set found");
        return false;
    }

    mCorrectionData = std::unique_ptr<CorrectionData>(new CorrectionData());
    find_corrections(message);

    mLastCorrectionDataTime = mCorrectionData->latest_correction_time();
    return true;
}

void Generator::find_correction_point_set(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT {
    FUNCTION_SCOPE();
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

        DEBUGF("correction point set:");
        DEBUGF("  set_id: %u", correction_point_set.set_id);
        DEBUGF("  reference_point_latitude:  %.14f", correction_point_set.reference_point_latitude);
        DEBUGF("  reference_point_longitude: %.14f",
               correction_point_set.reference_point_longitude);
        DEBUGF("  step_of_latitude:  %.14f", correction_point_set.step_of_latitude);
        DEBUGF("  step_of_longitude: %.14f", correction_point_set.step_of_longitude);

        uint64_t bitmask = 0;
        if (array.bitmaskOfGrids_r16) {
            for (size_t i = 0; i < array.bitmaskOfGrids_r16->size; i++) {
                bitmask <<= 8;
                bitmask |= static_cast<uint64_t>(array.bitmaskOfGrids_r16->buf[i]);
            }
            bitmask >>= array.bitmaskOfGrids_r16->bits_unused;
            DEBUGF("  bitmask: %ld bytes, %d bits, 0x%016lX", array.bitmaskOfGrids_r16->size,
                   array.bitmaskOfGrids_r16->bits_unused, bitmask);
        } else {
            bitmask = 0xFFFFFFFFFFFFFFFF;
        }
        correction_point_set.bitmask = bitmask;

        auto cps_ptr = new CorrectionPointSet(correction_point_set);

        DEBUGF("  grid_point_count: %u", grid_point_count);
        for (long i = 0; i < 64; i++) {
            CorrectionPointInfo cpi{};
            if (cps_ptr->array_to_index(i, &cpi)) {
                DEBUGF("    %2ld: %2ld/%2ld %ld/%ld %s %+18.14f %+18.14f %+18.14f", i,
                       cpi.absolute_index, cpi.array_index, cpi.latitude_index, cpi.longitude_index,
                       cpi.is_valid ? "ok" : "--", cpi.position.x, cpi.position.y, cpi.position.z);
            } else {
                DEBUGF("    %2ld: invalid", i);
            }
        }

        if (mIgnoreBitmask) {
            NOTICEF("ignoring correction point bitmask");
            cps_ptr->bitmask = 0xFFFFFFFFFFFFFFFF;
        }

        mCorrectionPointSet.reset(cps_ptr);
    } else {
        // TODO(ewasjon): [low-priority] Support list of correction points
        WARNF("unsupported correction point type");
    }
}

void Generator::find_corrections(ProvideAssistanceData_r9_IEs const& message) NOEXCEPT {
    FUNCTION_SCOPE();
    if (!message.a_gnss_ProvideAssistanceData) return;
    if (!message.a_gnss_ProvideAssistanceData->gnss_GenericAssistData) return;

    ASSERT(mCorrectionData, "correction data not initialized");
    ASSERT(mCorrectionPointSet, "correction point set not initialized");

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

bool Generator::find_ephemeris(SatelliteId sv_id, ts::Tai const& time, uint16_t iod,
                               ephemeris::Ephemeris& eph) const NOEXCEPT {
    FUNCTION_SCOPE();
    if (sv_id.gnss() == SatelliteId::Gnss::GPS) {
        auto it = mGpsEphemeris.find(sv_id);
        if (it == mGpsEphemeris.end()) return false;
        auto& list = it->second;

        auto gps_time = ts::Gps(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                     ephemeris.is_valid(gps_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(gps_time)) continue;
            if (ephemeris.lpp_iod != iod && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::GLONASS) {
        // TODO:
        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::GALILEO) {
        auto it = mGalEphemeris.find(sv_id);
        if (it == mGalEphemeris.end()) return false;
        auto& list = it->second;

        auto gal_time = ts::Gst(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iod_nav,
                     ephemeris.is_valid(gal_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(gal_time)) continue;
            if (ephemeris.lpp_iod != iod && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::BEIDOU) {
        auto it = mBdsEphemeris.find(sv_id);
        if (it == mBdsEphemeris.end()) return false;
        auto& list = it->second;

        auto bds_time = ts::Bdt(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                     ephemeris.is_valid(bds_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(bds_time)) continue;
            if (ephemeris.lpp_iod != iod && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        return false;
    }

    UNREACHABLE();
    return false;
}

void Generator::process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_gps_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GPS %d", ephemeris.prn);
        return;
    }

    auto& list = mGpsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GpsEphemeris const& a, ephemeris::GpsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

void Generator::process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_gal_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GAL %d", ephemeris.prn);
        return;
    }

    auto& list = mGalEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GalEphemeris const& a, ephemeris::GalEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

void Generator::process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_bds_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: BDS %d", ephemeris.prn);
        return;
    }

    auto& list = mBdsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::BdsEphemeris const& a, ephemeris::BdsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

}  // namespace tokoro
}  // namespace generator
