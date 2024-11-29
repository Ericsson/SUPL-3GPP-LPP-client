#include "generator.hpp"
#include "constant.hpp"
#include "coordinate.hpp"
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
      mGenerator(generator) {
    // Initialize the satellite vector to the maximum number of satellites
    // GPS: 32, GLONASS: 24, GALILEO: 36, BEIDOU: 35
    mSatellites.reserve(32 + 24 + 36 + 35);

    // Initialize the satellites
    initialize_satellites();
}

ReferenceStation::~ReferenceStation() NOEXCEPT = default;

void ReferenceStation::initialize_satellites() NOEXCEPT {
    VSCOPE_FUNCTION();

    // GPS
    if (mGenerateGps) {
        for (uint8_t i = 1; i <= 32; i++) {
            auto id = SatelliteId::from_gps_prn(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
    }

    // TODO: GLONASS
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
        for (uint8_t i = 1; i <= 35; i++) {
            auto id = SatelliteId::from_bds_prn(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
    }
}

void ReferenceStation::initialize_observation(Satellite& satellite, SignalId signal_id) NOEXCEPT {
    VSCOPE_FUNCTION();

    auto correction_data = *mGenerator.mCorrectionData;

    auto& observation = satellite.initialize_observation(signal_id);
    observation.compute_phase_bias(correction_data);
    observation.compute_code_bias(correction_data);
    observation.compute_tropospheric(mGroundPosition, correction_data);
    observation.compute_ionospheric(mGroundPosition, correction_data);

    if (mShapiroCorrection) observation.compute_shapiro();
    if (mEarthSolidTidesCorrection) observation.compute_earth_solid_tides();
    if (mPhaseWindupCorrection) observation.compute_phase_windup();
    if (mAntennaPhaseVariation) observation.compute_antenna_phase_variation();

    if (mTropoHeightCorrection) observation.compute_tropospheric_height();

    observation.compute_ranges();

    VERBOSEF("observation: c=%f, p=%f", observation.code_range(), observation.phase_range());

    if (!observation.has_phase_bias()) {
        WARNF("discarded: %s %s - no phase bias", satellite.id().name(), signal_id.name());
        observation.discard();
    } else if (!observation.has_code_bias()) {
        WARNF("discarded: %s %s - no code bias", satellite.id().name(), signal_id.name());
        observation.discard();
    }

    if (!observation.has_tropospheric()) {
        WARNF("discarded: %s %s - no tropospheric correction", satellite.id().name(),
              signal_id.name());
        observation.discard();
    }
    if (!observation.has_ionospheric()) {
        WARNF("discarded: %s %s - no ionospheric correction", satellite.id().name(),
              signal_id.name());
        observation.discard();
    }
}

bool ReferenceStation::generate(ts::Tai const& reception_time) NOEXCEPT {
    VSCOPE_FUNCTION();

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
    for (auto& satellite : mSatellites) {
        satellite.reset_observations();

        if (!satellite.enabled()) continue;
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
                continue;
            }

            initialize_observation(satellite, signal);
        }

        DEBUGF("satellite %s: %zu observations", satellite.id().name(),
               satellite.observations().size());
    }

    return true;
}

void ReferenceStation::build_rtcm_observation(Satellite const&         satellite,
                                              Observation const&       observation,
                                              RangeTimeDivision const& rtd,
                                              double                   reference_phase_range_rate,
                                              rtcm::Observations&      observations) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", observation.signal_id().name());

    auto code_range       = observation.code_range();
    auto phase_range      = observation.phase_range();
    auto phase_range_rate = observation.phase_range_rate();

    auto meter_to_cms   = 1.0e3 / constant::SPEED_OF_LIGHT;
    auto code_range_ms  = code_range * meter_to_cms;
    auto phase_range_ms = phase_range * meter_to_cms;

    auto delta_code_range_ms    = code_range_ms - rtd.used_range;
    auto delta_phase_range_ms   = phase_range_ms - rtd.used_range;
    auto delta_phase_range_rate = phase_range_rate - reference_phase_range_rate;

    VERBOSEF("%-15s code:  %+.14f (%+.14f)", observation.signal_id().name(), code_range_ms,
             delta_code_range_ms);
    VERBOSEF("%-15s phase: %+.14f (%+.14f)", observation.signal_id().name(), phase_range_ms,
             delta_phase_range_ms);
    VERBOSEF("%-15s phase rate: %+.14f (%+.14f)", observation.signal_id().name(), phase_range_rate,
             delta_phase_range_rate);

#if 0
        auto reconstructed =
            (rtd.integer_ms + rtd.rough_range + delta_code_range_ms) / meter_to_cms;
        VERBOSEF("code_range reconstructed: %.14f == %.14f (%.14f)", code_range, reconstructed,
                 code_range - reconstructed);
#endif

    rtcm::Signal signal{};
    signal.id                     = observation.signal_id();
    signal.satellite              = satellite.id();
    signal.fine_pseudo_range      = delta_code_range_ms;
    signal.fine_phase_range       = delta_phase_range_ms;
    signal.fine_phase_range_rate  = delta_phase_range_rate;
    signal.carrier_to_noise_ratio = 47.0;   // TODO(ewasjon): How do we choose this value?
    signal.lock_time              = 525.0;  // TODO: How do we determine this value?
    observations.signals.push_back(signal);
}

void ReferenceStation::build_rtcm_satellite(Satellite const&    satellite,
                                            rtcm::Observations& observations) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, observations=%zu", satellite.id().name(),
                     satellite.observations().size());

    auto average_code_range       = satellite.average_code_range();
    auto average_phase_range_rate = satellite.average_phase_range_rate();
    auto rtd                      = range_time_division(average_code_range);
    VERBOSEF("average_code_range:       %.4f", average_code_range);
    VERBOSEF("average_phase_range_rate: %.4f", average_phase_range_rate);
    VERBOSEF("rtd: %d %f %f %f", rtd.integer_ms, rtd.rough_range, rtd.used_range, rtd.unused_range);

    // Round average phase_range_rate to the nearest integer
    auto phase_range_rate = std::round(average_phase_range_rate);

    rtcm::Satellite rtcm{};
    rtcm.id                     = satellite.id();
    rtcm.integer_ms             = rtd.integer_ms;
    rtcm.rough_range            = rtd.rough_range;
    rtcm.rough_phase_range_rate = phase_range_rate;
    observations.satellites.push_back(rtcm);

    for (auto const& observation : satellite.observations()) {
        if (!observation.is_valid()) continue;
        build_rtcm_observation(satellite, observation, rtd, phase_range_rate, observations);
    }
}

std::vector<rtcm::Message> ReferenceStation::produce() NOEXCEPT {
    VSCOPE_FUNCTION();

    std::vector<rtcm::Message> messages;

    auto reference_station_id = 1902;
    auto msm_type             = 5;

    rtcm::ReferenceStation reference_station{};
    reference_station.reference_station_id          = reference_station_id;
    reference_station.x                             = mRtcmGroundPosition.x;
    reference_station.y                             = mRtcmGroundPosition.y;
    reference_station.z                             = mRtcmGroundPosition.z;
    reference_station.is_physical_reference_station = false;
    reference_station.antenna_height                = 0.0;

    messages.push_back(
        rtcm::generate_1006(reference_station, mGenerateGps, mGenerateGlo, mGenerateGal));

    if (mRtcmPhysicalGroundPositionSet) {
        auto physical_reference_station_id = 4095;
        if (reference_station_id > 1) {
            physical_reference_station_id = reference_station_id - 1;
        }

        rtcm::PhysicalReferenceStation physical_reference_station{};
        physical_reference_station.reference_station_id = physical_reference_station_id;
        physical_reference_station.x                    = mRtcmPhysicalGroundPosition.x;
        physical_reference_station.y                    = mRtcmPhysicalGroundPosition.y;
        physical_reference_station.z                    = mRtcmPhysicalGroundPosition.z;
        messages.push_back(rtcm::generate_1032(reference_station, physical_reference_station));
    }

    rtcm::CommonObservationInfo common{};
    common.reference_station_id = reference_station_id;
    common.clock_steering       = 1;

    rtcm::Observations msm_gps{};
    rtcm::Observations msm_glo{};
    rtcm::Observations msm_gal{};
    rtcm::Observations msm_bds{};

    msm_gps.time = mGenerationTime;
    msm_glo.time = mGenerationTime;
    msm_gal.time = mGenerationTime;
    msm_bds.time = mGenerationTime;

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
            rtcm::generate_msm(msm_type, last_msm, rtcm::GenericGnssId::GPS, common, msm_gps);
        messages.push_back(std::move(message));
    }

    if (will_generate_glo) {
        auto last_msm = !will_generate_gal && !will_generate_bds;
        auto message =
            rtcm::generate_msm(msm_type, last_msm, rtcm::GenericGnssId::GLONASS, common, msm_glo);
        messages.push_back(std::move(message));
    }

    if (will_generate_gal) {
        auto last_msm = !will_generate_bds;
        auto message =
            rtcm::generate_msm(msm_type, last_msm, rtcm::GenericGnssId::GALILEO, common, msm_gal);
        messages.push_back(std::move(message));
    }

    if (will_generate_bds) {
        auto message =
            rtcm::generate_msm(msm_type, true, rtcm::GenericGnssId::BEIDOU, common, msm_bds);
        messages.push_back(std::move(message));
    }

    return messages;
}

//
//
//

Generator::Generator() NOEXCEPT {}

Generator::~Generator() NOEXCEPT = default;

std::shared_ptr<ReferenceStation>
Generator::define_reference_station(ReferenceStationConfig const& config) NOEXCEPT {
    INFOF("define reference station:");
    INFOF("  ground position (itrf): (%f, %f, %f)", config.itrf_ground_position.x,
          config.itrf_ground_position.x, config.itrf_ground_position.x);
    INFOF("  ground position (rtcm): (%f, %f, %f)", config.rtcm_ground_position.x,
          config.rtcm_ground_position.x, config.rtcm_ground_position.x);
    INFOF("  gnss: %s%s%s%s", config.generate_gps ? "G" : "-", config.generate_glo ? "R" : "-",
          config.generate_gal ? "E" : "-", config.generate_bds ? "C" : "-");

    return std::make_shared<ReferenceStation>(*this, config);
}

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
        VERBOSEF("  reference_point_latitude:  %.14f",
                 correction_point_set.reference_point_latitude);
        VERBOSEF("  reference_point_longitude: %.14f",
                 correction_point_set.reference_point_longitude);
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
        // if (!mGenerateGps && gnss_id == GNSS_ID__gnss_id_gps) {
        //     VERBOSEF("skipping GPS");
        //     continue;
        // } else if (!mGenerateGal && gnss_id == GNSS_ID__gnss_id_galileo) {
        //     VERBOSEF("skipping Galileo");
        //     continue;
        // } else if (!mGenerateBds && gnss_id == GNSS_ID__gnss_id_bds) {
        //     VERBOSEF("skipping BeiDou");
        //     continue;
        // } else if (!mGenerateGlo && gnss_id == GNSS_ID__gnss_id_glonass) {
        //     VERBOSEF("skipping GLONASS");
        //     continue;
        // }

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

bool Generator::find_ephemeris(SatelliteId sv_id, ts::Tai const& time, uint16_t iode,
                               ephemeris::Ephemeris& eph) const NOEXCEPT {
    if (sv_id.gnss() == SatelliteId::Gnss::GPS) {
        auto it = mGpsEphemeris.find(sv_id);
        if (it == mGpsEphemeris.end()) return false;
        auto& list = it->second;

        auto gps_time = ts::Gps(time);
        for (auto& ephemeris : list) {
            VERBOSEF("searching: %s %4u %.0f %4u%s%s", sv_id.name(), ephemeris.week_number,
                     ephemeris.toe, ephemeris.iode, ephemeris.is_valid(gps_time) ? " [time]" : "",
                     ephemeris.iode == iode ? " [iode]" : "");
            if (!ephemeris.is_valid(gps_time)) continue;
            if (ephemeris.iode != iode) continue;
            eph = ephemeris::Ephemeris(ephemeris);
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
        for (auto& ephemeris : list) {
            VERBOSEF("searching: %s %4u %.0f %4u%s%s", sv_id.name(), ephemeris.week_number,
                     ephemeris.toe, ephemeris.iod_nav,
                     ephemeris.is_valid(gal_time) ? " [time]" : "",
                     ephemeris.iod_nav == iode ? " [iode]" : "");
            if (!ephemeris.is_valid(gal_time)) continue;
            if (ephemeris.iod_nav != iode) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            return true;
        }

        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::BEIDOU) {
        auto  it   = mBdsEphemeris.find(sv_id);
        auto& list = it->second;

        auto bds_time = ts::Bdt(time);
        for (auto& ephemeris : list) {
            VERBOSEF("searching: %s %4u %.0f %4u%s%s", sv_id.name(), ephemeris.week_number,
                     ephemeris.toe, ephemeris.iode, ephemeris.is_valid(bds_time) ? " [time]" : "",
                     ephemeris.iode == iode ? " [iode]" : "");
            if (!ephemeris.is_valid(bds_time)) continue;
            if (ephemeris.iode != iode) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            return true;
        }

        return false;
    }

    UNREACHABLE();
    return false;
}

void Generator::process_ephemeris(ephemeris::GpsEphemeris const& ephemeris) NOEXCEPT {
    auto satellite_id = SatelliteId::from_gps_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GPS %d", ephemeris.prn);
        return;
    }

    auto& list = mGpsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.compare(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s", satellite_id.name());
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    DEBUGF("ephemeris: %s", satellite_id.name());
}

void Generator::process_ephemeris(ephemeris::GalEphemeris const& ephemeris) NOEXCEPT {
    auto satellite_id = SatelliteId::from_gal_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GAL %d", ephemeris.prn);
        return;
    }

    auto& list = mGalEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.compare(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s", satellite_id.name());
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    DEBUGF("ephemeris: %s", satellite_id.name());
}

void Generator::process_ephemeris(ephemeris::BdsEphemeris const& ephemeris) NOEXCEPT {
    auto satellite_id = SatelliteId::from_bds_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: BDS %d", ephemeris.prn);
        return;
    }

    auto& list = mBdsEphemeris[satellite_id];

    // Check if the ephemeris is already in the list
    for (auto& eph : list) {
        if (eph.compare(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s", satellite_id.name());
            return;
        }
    }

    // Remove the oldest ephemeris if the list is full
    if (list.size() >= 10) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    DEBUGF("ephemeris: %s", satellite_id.name());
}

}  // namespace tokoro
}  // namespace generator
