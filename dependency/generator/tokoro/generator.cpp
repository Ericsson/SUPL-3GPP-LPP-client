#include "generator.hpp"
#include "constant.hpp"
#include "coordinate.hpp"
#include "data/correction.hpp"
#include "decode.hpp"
#include "models/frame_transform.hpp"
#include "models/geoid.hpp"
#include "models/helper.hpp"
#include "models/troposphere_models.hpp"
#include "observation.hpp"
#include "reference_ellipsoid.hpp"
#include "satellite.hpp"

#ifdef ENABLE_TOKORO_SNAPSHOT
#include <generator/tokoro/snapshot.hpp>
#endif
#include <msgpack/msgpack.hpp>

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
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
EXTERNAL_WARNINGS_POP

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <unordered_map>

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/generator.hpp>
#include <generator/rtcm/rtk_data.hpp>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE(tokoro);
#undef LOGLET_CURRENT_MODULE
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
      mGenerateQzs(config.generate_qzs),
      mShapiroCorrection(true),
      mEarthSolidTidesCorrection(false),
      mPhaseWindupCorrection(false),
      mAntennaPhaseVariation(false),
      mTropoHeightCorrection(false),
      mTropoHydrostaticDelta(true),
      mElevationMask(10.0),
      mPhaseRangeRate(true),
      mRtcmReferenceStationId(1902),
      mRtcmMsmType(5),
      mNegativePhaseWindup(false),
      mNegativeCorrectionSign(false),
      mPhaseAlignment(false),
      mAntennaDescriptor(),
      mReceiverDescriptor(),
#ifdef INCLUDE_FORMAT_RINEX
      mGenerateRinex(false),
#endif
      mRequireCodeBias(true),
      mRequirePhaseBias(true),
      mRequireTropo(true),
      mRequireIono(true),
      mUseTroposphericModel(false),
      mUseIonosphericHeightCorrection(false),
#ifdef INCLUDE_FORMAT_RINEX
      mRinexBuilder(ts::Utc::now().rinex_filename() + ".rnx", 3.04),
#endif
      mGenerator(generator) {
    // GPS: 32, GLONASS: 24, Galileo: 36, BeiDou: 63, QZSS: 10
    mSatellites.reserve(32 + 24 + 36 + 63 + 10);

    // Initialize the satellites
    initialize_satellites();

    mGenerationTime = ts::Tai::now();
#ifdef INCLUDE_FORMAT_RINEX
    mLastRinexEpoch = mGenerationTime;
#endif
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

    // GLONASS
    if (mGenerateGlo) {
        for (uint8_t i = 1; i <= 24; i++) {
            auto id = SatelliteId::from_glo_slot(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
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

    // QZSS
    if (mGenerateQzs) {
        for (uint8_t i = 193; i <= 202; i++) {
            auto id = SatelliteId::from_qzs_prn(i);
            ASSERT(id.is_valid(), "invalid satellite id");
            mSatellites.emplace_back(id, mGroundPosition, mGenerator);
        }
    }
}

static CONSTEXPR int64_t PHASE_AMBIGUITY_LIMIT = 1000;

// Lock time [s] at which both the MSM4/5 (DF402, >= 524.288 s) and MSM6/7 (DF407, >= 67108.864 s)
// lock time indicators are saturated.
static CONSTEXPR double LOCK_TIME_MAX_SECONDS = 67108.864;

static uint64_t splitmix64(uint64_t x) NOEXCEPT {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

// Deterministic for a given seed, signal and loss count (independent of iteration order), so a
// run can be reproduced and a recreated reference station draws the same ambiguity.
int64_t ReferenceStation::random_phase_ambiguity(SatelliteSignalId ss_id) const NOEXCEPT {
    auto key = (static_cast<uint64_t>(ss_id.satellite().absolute_id()) << 32) ^
               static_cast<uint64_t>(ss_id.signal().absolute_id());
    auto count_it = mLossCount.find(ss_id);
    auto count    = count_it != mLossCount.end() ? count_it->second : 0u;
    auto hash     = splitmix64(mAmbiguitySeed ^ splitmix64(key) ^ splitmix64(count + 1ULL));
    auto range    = static_cast<uint64_t>(2 * PHASE_AMBIGUITY_LIMIT + 1);
    return static_cast<int64_t>(hash % range) - PHASE_AMBIGUITY_LIMIT;
}

// The IODs are compared with this many bits: GPS and QZSS compare the 8 bits of the IODE, the IOD
// of Galileo is the 10 bit IODnav, the IOD of GLONASS is the 7 bit tb and the IOD of BeiDou are
// the 11 MSB of toe.
static int iod_modulus(SatelliteId id) {
    if (id.is_gps() || id.is_qzss()) return 256;
    if (id.is_glonass()) return 128;
    if (id.is_galileo()) return 1024;
    return 2048;
}

// wanted - actual, wrapped to [-modulus / 2, modulus / 2)
static int iod_offset(SatelliteId id, uint16_t wanted, uint16_t actual) {
    auto modulus = iod_modulus(id);
    auto offset  = (static_cast<int>(wanted) % modulus) - (static_cast<int>(actual) % modulus);
    offset       = ((offset % modulus) + modulus) % modulus;
    if (offset >= modulus / 2) offset -= modulus;
    return offset;
}

DiagFile& ReferenceStation::diag_file(SatelliteId satellite_id, SignalId signal_id) NOEXCEPT {
    SatelliteSignalId ss_id{satellite_id, signal_id};
    auto              it = mDiagFiles.find(ss_id);
    if (it != mDiagFiles.end()) return it->second;

    auto sv_char = satellite_id.to_string()[0];
    char prn_buf[8];
    snprintf(prn_buf, sizeof(prn_buf), "%c%02d", sv_char,
             static_cast<int>(satellite_id.lpp_id().value) + 1);
    auto sig_str = DiagFile::rtcm_signal_name(satellite_id, signal_id);
    auto path =
        mDiagOutputDir + "/" + sv_char + "/" + std::string(prn_buf) + "/" + sig_str + ".diag";
    auto& file = mDiagFiles[ss_id];
    file.open(path, satellite_id, signal_id);
    return file;
}

void ReferenceStation::write_unavailable_signals(Satellite const& satellite) NOEXCEPT {
    if (!mGenerateDiag) return;
    auto reason = satellite.disable_reason();
    if (!reason || strcmp(reason, "not_included") == 0) return;

    auto signals = mGenerator.mCorrectionData->signals(satellite.id());
    if (!signals) return;

    for (auto const& signal : *signals) {
        if (mSignalIncludeSet.size() > 0 &&
            mSignalIncludeSet.find(signal) == mSignalIncludeSet.end()) {
            continue;
        }
        if (signal.frequency() <= 1.0) continue;

        auto    msm_id = signal.as_msm();
        DiagRow row{};
        row.prn =
            satellite.id().lpp_id().valid ? static_cast<int>(satellite.id().lpp_id().value) + 1 : 0;
        row.sig_id         = msm_id.valid ? static_cast<int>(msm_id.value) : 0;
        row.frequency_mhz  = signal.frequency() * 1.0e-3;
        row.valid          = false;
        row.discard_reason = std::string("satellite_") + reason;
        row.discard_detail =
            satellite.disable_detail().empty() ?
                std::string("satellite_") + reason + ":unknown" :
                std::string("satellite_") + reason + ":" + satellite.disable_detail();
        diag_file(satellite.id(), signal).write(mGenerationTime, row);
    }
}

void ReferenceStation::initialize_observation(Satellite& satellite, SignalId signal_id,
                                              char const* diag_discard_reason,
                                              std::string diag_discard_detail) NOEXCEPT {
    FUNCTION_SCOPE();

    auto const& correction_data = *mGenerator.mCorrectionData;

    // Update lock tracking
    SatelliteSignalId ss_id{satellite.id(), signal_id};

    // A changed phaseDiscontinuityIndicator_r16 from the SSR provider means the phase
    // ambiguity/lock arc is no longer continuous, even though the signal never left
    // the active set - restart the lock the same way a freshly (re)acquired signal
    // would be treated.
    auto discontinuity_changed = false;
    if (auto* phase_bias = correction_data.phase_bias(satellite.id(), signal_id)) {
        auto counter_it = mLastPhaseDiscontinuityCounter.find(ss_id);
        if (counter_it != mLastPhaseDiscontinuityCounter.end() &&
            counter_it->second != phase_bias->discontinuity) {
            discontinuity_changed = true;
        }
        mLastPhaseDiscontinuityCounter[ss_id] = phase_bias->discontinuity;
    }

    if (discontinuity_changed) {
        // Keep N within +-PHASE_AMBIGUITY_LIMIT cycles. The carrier phase is encoded relative
        // to the rough range in a +-2^-8 ms (~1171 m) field (DF401/DF406); the limit leaves
        // ample room below that for N*wavelength (<= ~0.26 m/cycle, so <= ~260 m) on top of
        // the code remainder (<~150 m) and phase-code differences. Wrapping is still an
        // integer-cycle jump, i.e. indistinguishable from another slip.
        auto& ambiguity = mPhaseAmbiguity[ss_id];
        ambiguity += 1;
        if (ambiguity > PHASE_AMBIGUITY_LIMIT) ambiguity = -PHASE_AMBIGUITY_LIMIT;
    }

    // A signal without a lock entry has just appeared: at startup (first epoch with
    // observations) or as a rise (rising satellite, or a signal that was lost earlier).
    auto is_new     = mLockTime.find(ss_id) == mLockTime.end();
    auto start_mode = mStarted ? mLockRise : mLockStart;
    if (is_new) {
        auto mode = mAmbiguityRandomization;
        auto randomize =
            mStarted ?
                (mode == AmbiguityRandomization::Rise || mode == AmbiguityRandomization::Both) :
                (mode == AmbiguityRandomization::Startup || mode == AmbiguityRandomization::Both);
        if (randomize) {
            auto ambiguity         = random_phase_ambiguity(ss_id);
            mPhaseAmbiguity[ss_id] = ambiguity;
            VERBOSEF("ambiguity %s %s: %+lld", mStarted ? "rise" : "startup",
                     ss_id.to_string().c_str(), static_cast<long long>(ambiguity));
        }
    }

    LockTime lock_time{};
    if (is_new && !discontinuity_changed && start_mode == LockStart::Max) {
        lock_time.time    = mGenerationTime + ts::Timestamp{-LOCK_TIME_MAX_SECONDS};
        lock_time.seconds = LOCK_TIME_MAX_SECONDS;
        lock_time.lost    = false;
        mLockTime[ss_id]  = lock_time.time;
        VERBOSEF("lock %s %s: max", mStarted ? "rise" : "startup", ss_id.to_string().c_str());
    } else if (discontinuity_changed || is_new) {
        lock_time.time    = mGenerationTime;
        lock_time.seconds = 0;
        lock_time.lost    = true;
        // Persist the reset immediately so later generations compute elapsed lock
        // time relative to the discontinuity, not the stale pre-discontinuity start.
        mLockTime[ss_id] = mGenerationTime;
    } else {
        lock_time.time    = mLockTime[ss_id];
        lock_time.seconds = mGenerationTime.difference_seconds(lock_time.time);
        lock_time.lost    = false;
    }

#ifdef INCLUDE_FORMAT_ANTEX
    auto antex = mGenerator.mAntex.get();
#endif

    auto& observation = satellite.initialize_observation(signal_id);
    observation.set_tropospheric_selection(mTroposphereModel, mMappingFunction,
                                           mTropoHydrostaticDelta,
                                           mGenerator.mTroposphereData.get());
    observation.compute_tropospheric_mapping();
    observation.update_lock_time(lock_time);
    {
        auto ambiguity_it = mPhaseAmbiguity.find(ss_id);
        observation.set_phase_ambiguity(
            ambiguity_it != mPhaseAmbiguity.end() ? ambiguity_it->second : 0);
    }
    observation.compute_phase_bias(correction_data);
    observation.compute_code_bias(correction_data);
    observation.compute_tropospheric(correction_data);
    observation.compute_ionospheric(correction_data);

#ifdef INCLUDE_FORMAT_ANTEX
    if (mAntennaPhaseVariation && antex) observation.compute_antenna_phase_variation(*antex);
#endif
    if (mTropoHeightCorrection) observation.compute_tropospheric_height();

    observation.set_negative_phase_windup(mNegativePhaseWindup);
    observation.set_negative_correction_sign(mNegativeCorrectionSign);
    observation.set_ssr_generation_consistency_check(mGenerator.mSsrGenerationConsistencyCheck);
    observation.set_require_code_bias(mRequireCodeBias);
    observation.set_require_phase_bias(mRequirePhaseBias);
    observation.set_require_tropo(mRequireTropo);
    observation.set_require_iono(mRequireIono);
    observation.set_use_tropospheric_model(mUseTroposphericModel);
    observation.compute_tropospheric_model();
    observation.set_tropospheric_gradient(mGradientSource, mGradientMapping, mGradientConstantSet,
                                          mGradientConstantNorth, mGradientConstantEast);
    observation.compute_tropospheric_gradient();
    observation.set_use_ionospheric_height_correction(mUseIonosphericHeightCorrection);

    if (mGenerateDiag) {
        observation.set_diag_file(&diag_file(satellite.id(), signal_id));
    }

    if (diag_discard_reason) {
        observation.set_diag_discard_reason(diag_discard_reason);
        observation.set_diag_discard_detail(std::move(diag_discard_detail));
    }

    if (mGenerateDiag) {
        auto previous = mPhaseDiscontinuity.find(ss_id);
        if (previous != mPhaseDiscontinuity.end())
            observation.set_previous_phase_discontinuity(previous->second);
    }

    observation.compute_ranges();

    if (mGenerateDiag) {
        if (observation.has_phase_bias())
            mPhaseDiscontinuity[ss_id] = observation.phase_discontinuity();
        else
            mPhaseDiscontinuity.erase(ss_id);
    }

    if (!observation.is_valid()) return;
    VERBOSEF("observation: c=%f, p=%f", observation.code_range(), observation.phase_range());
}

void ReferenceStation::write_tropospheric_diag(Satellite const& satellite) NOEXCEPT {
    FUNCTION_SCOPE();

    // only satellites above the horizon with a position
    if (!satellite.has_orbit_correction() || !satellite.has_clock_correction()) return;
    auto const& state = satellite.current_state();
    if (!(state.true_range > 0.0) || !(state.true_elevation > 0.0)) return;

    auto models   = all_troposphere_models();
    auto mappings = all_mapping_functions();

    // the sources of the zenith delays: the SSR delays and every model
    std::vector<std::string> zenith_names;
    zenith_names.push_back("ssr");
    for (auto model : models) {
        zenith_names.push_back(troposphere_model_name(model));
    }
    std::vector<std::string> mapping_names;
    for (auto mapping : mappings) {
        mapping_names.push_back(mapping_function_name(mapping));
    }
    auto                     gradient_sources  = all_gradient_sources();
    auto                     gradient_mappings = all_gradient_mappings();
    std::vector<std::string> gradient_source_names;
    for (auto source : gradient_sources) {
        gradient_source_names.push_back(gradient_source_name(source));
    }
    std::vector<std::string> gradient_mapping_names;
    for (auto mapping : gradient_mappings) {
        gradient_mapping_names.push_back(gradient_mapping_name(mapping));
    }

    auto it = mTropoDiagFiles.find(satellite.id());
    if (it == mTropoDiagFiles.end()) {
        auto sv_char = satellite.id().to_string()[0];
        char prn_buf[8];
        snprintf(prn_buf, sizeof(prn_buf), "%c%02d", sv_char,
                 static_cast<int>(satellite.id().lpp_id().value) + 1);
        auto path = mDiagOutputDir + "/" + sv_char + "/" + std::string(prn_buf) + "/tropo.diag";
        mTropoDiagFiles[satellite.id()].open(path, satellite.id(), zenith_names, mapping_names,
                                             gradient_source_names, gradient_mapping_names);
        it = mTropoDiagFiles.find(satellite.id());
    }

    auto            llh          = ecef_to_llh(mGroundPosition, ellipsoid::gWgs84);
    auto            geoid_height = Geoid::height(llh.x, llh.y, Geoid::Model::EMBEDDED);
    TroposphereSite site{state.reception_time, llh, geoid_height};
    auto const*     data = mGenerator.mTroposphereData.get();

    TropoDiagRow row{};
    row.prn =
        satellite.id().lpp_id().valid ? static_cast<int>(satellite.id().lpp_id().value) + 1 : 0;
    row.elevation_deg = state.true_elevation * constant::RAD2DEG;
    row.azimuth_deg   = state.true_azimuth * constant::RAD2DEG;

    // zenith delays, source 0 is SSR
    row.zenith.resize(zenith_names.size());
    row.gradient.resize(gradient_sources.size());
    for (size_t i = 0; i < gradient_sources.size(); i++) {
        if (gradient_sources[i] == GradientSource::Constant && mGradientConstantSet) {
            row.gradient[i] = {true, mGradientConstantNorth, mGradientConstantEast};
        }
    }
    {
        TroposphericCorrection ssr{};
        if (mGenerator.mCorrectionData->tropospheric(satellite.id(), llh, ssr)) {
            auto hydrostatic_scale = 1.0;
            auto wet_scale         = 1.0;
            auto scaled            = true;
            if (mTropoHeightCorrection) {
                scaled = height_scaling(site, hydrostatic_scale, wet_scale);
            }
            if (scaled) {
                row.zenith[0].valid       = true;
                row.zenith[0].hydrostatic = ssr.dry * hydrostatic_scale;
                row.zenith[0].wet         = ssr.wet * wet_scale;

                double north = 0.0;
                double east  = 0.0;
                if (ssr_grid_gradient(ssr, hydrostatic_scale, wet_scale, north, east)) {
                    for (size_t i = 0; i < gradient_sources.size(); i++) {
                        if (gradient_sources[i] == GradientSource::SsrGrid) {
                            row.gradient[i] = {true, north, east};
                        }
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < models.size(); i++) {
        ZenithDelays delays{};
        if (zenith_delays(models[i], data, site, delays)) {
            row.zenith[i + 1] = {true, delays.hydrostatic, delays.wet};
        }
    }

    // mapping functions
    row.mapping.resize(mappings.size());
    for (size_t i = 0; i < mappings.size(); i++) {
        MappingValues values{};
        if (mapping_values(mappings[i], data, site, state.true_elevation, mTropoHydrostaticDelta,
                           values)) {
            row.mapping[i] = {true, values.hydrostatic, values.wet};
        }
    }

    // every combination
    row.slant.resize(zenith_names.size() * mappings.size());
    for (size_t s = 0; s < zenith_names.size(); s++) {
        for (size_t m = 0; m < mappings.size(); m++) {
            if (!row.zenith[s].valid || !row.mapping[m].valid) continue;
            auto& slant = row.slant[s * mappings.size() + m];
            slant.valid = true;
            slant.value = row.zenith[s].hydrostatic * row.mapping[m].hydrostatic +
                          row.zenith[s].wet * row.mapping[m].wet;
        }
    }

    // the delay that is applied: SSR if available, otherwise the selected model if enabled
    size_t mapping_index = 0;
    for (size_t m = 0; m < mappings.size(); m++) {
        if (mappings[m] == mMappingFunction) mapping_index = m;
    }
    size_t source_index = 0;
    if (!row.zenith[0].valid && mUseTroposphericModel) {
        for (size_t i = 0; i < models.size(); i++) {
            if (models[i] == mTroposphereModel) source_index = i + 1;
        }
    }

    // gradient mapping functions, `macmillan` uses m_h of the selected mapping function
    row.gradient_mapping.resize(gradient_mappings.size());
    for (size_t g = 0; g < gradient_mappings.size(); g++) {
        auto const& selected = row.mapping[mapping_index];
        if (gradient_mappings[g] == GradientMapping::MacMillan && !selected.valid) continue;
        row.gradient_mapping[g] = {
            true, gradient_mapping_value(gradient_mappings[g], state.true_elevation,
                                         selected.valid ? selected.hydrostatic : 0.0)};
    }

    // gradient term of every combination of source and mapping function
    row.gradient_delay.resize(gradient_sources.size() * gradient_mappings.size());
    for (size_t s = 0; s < gradient_sources.size(); s++) {
        for (size_t g = 0; g < gradient_mappings.size(); g++) {
            if (!row.gradient[s].valid || !row.gradient_mapping[g].valid) continue;
            auto& term = row.gradient_delay[s * gradient_mappings.size() + g];
            term.valid = true;
            term.value = gradient_delay(row.gradient_mapping[g].value, state.true_azimuth,
                                        row.gradient[s].north, row.gradient[s].east);
        }
    }

    if (source_index != 0 || row.zenith[0].valid) {
        auto const& applied = row.slant[source_index * mappings.size() + mapping_index];
        if (applied.valid) {
            row.has_applied = true;
            row.applied     = applied.value;

            // the gradient of the selected source and mapping function
            for (size_t s = 0; s < gradient_sources.size(); s++) {
                for (size_t g = 0; g < gradient_mappings.size(); g++) {
                    auto const& term = row.gradient_delay[s * gradient_mappings.size() + g];
                    if (gradient_sources[s] == mGradientSource &&
                        gradient_mappings[g] == mGradientMapping && term.valid) {
                        row.applied_gradient = term.value;
                        row.applied += term.value;
                    }
                }
            }
        }
    }

    it->second.write(mGenerationTime, row);
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
            satellite.disable("not_included");
            continue;
        }

        satellite.update(mGenerationTime);
    }

    // Generate the observations
    std::unordered_set<SatelliteSignalId> active_signals;
    for (auto& satellite : mSatellites) {
        satellite.reset_observations();

        if (!satellite.enabled()) {
            TRACEF("discarded: %s - disabled", satellite.id().name());
            write_unavailable_signals(satellite);
            continue;
        }

        satellite.compute_sun_position();
        if (mShapiroCorrection) satellite.compute_shapiro();
        if (mEarthSolidTidesCorrection) satellite.compute_earth_solid_tides();
        if (mPhaseWindupCorrection) satellite.compute_phase_windup();
        satellite.datatrace_report();

        if (mSatelliteIncludeSet.size() > 0 &&
            mSatelliteIncludeSet.find(satellite.id()) == mSatelliteIncludeSet.end()) {
            WARNF("discarded: %s - not included", satellite.id().name());
            satellite.disable("not_included");
            continue;
        }

        bool        elevation_masked = false;
        std::string elevation_detail;
        if (satellite.elevation() * constant::RAD2DEG < mElevationMask) {
            WARNF("discarded: %s - elevation mask (%.2f < %.2f)", satellite.id().name(),
                  satellite.elevation() * constant::RAD2DEG, mElevationMask);
            elevation_masked = true;
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "elevation=%.2fdeg,mask=%.2fdeg",
                     satellite.elevation() * constant::RAD2DEG, mElevationMask);
            elevation_detail = buffer;
        }

        auto signals = mGenerator.mCorrectionData->signals(satellite.id());
        if (!signals) {
            WARNF("discarded: %s - no signals", satellite.id().name());
            satellite.disable("no_signals", "no_bias_data");
            continue;
        }

        // why the signals were discarded: reason -> count
        size_t                     signal_count = 0;
        std::map<std::string, int> discard_tally;
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

            initialize_observation(satellite, signal, elevation_masked ? "elevation_mask" : nullptr,
                                   elevation_detail);
            signal_count++;
            if (!satellite.observations().empty() && !satellite.observations().back().is_valid()) {
                auto const& reason = satellite.observations().back().discard_reason();
                discard_tally[reason.empty() ? "unknown" : reason]++;
            }
            satellite.remove_discarded_observations();
        }

        if (elevation_masked) {
            satellite.disable("elevation_mask", elevation_detail);
            continue;
        }

        if (satellite.observations().size() == 0) {
            WARNF("discarded: %s - no valid observations", satellite.id().name());
            std::string detail = "signals=" + std::to_string(signal_count);
            if (!discard_tally.empty()) {
                detail += "(";
                for (auto const& entry : discard_tally) {
                    if (detail.back() != '(') detail += ',';
                    detail += entry.first + ":" + std::to_string(entry.second);
                }
                detail += ")";
            }
            satellite.disable("no_valid_observations", detail);
            continue;
        }

        for (auto const& observation : satellite.observations()) {
            if (!observation.is_valid()) continue;
            active_signals.insert(observation.ss_id());
        }

        DEBUGF("satellite %s: %zu observations", satellite.id().name(),
               satellite.observations().size());
    }

    // Write per-satellite diagnostic
    if (mGenerateDiag) {
        for (auto const& satellite : mSatellites) {
            auto it = mSatDiagFiles.find(satellite.id());
            if (it == mSatDiagFiles.end()) {
                auto sv_char = satellite.id().to_string()[0];
                char prn_buf[8];
                snprintf(prn_buf, sizeof(prn_buf), "%c%02d", sv_char,
                         static_cast<int>(satellite.id().lpp_id().value) + 1);
                auto path =
                    mDiagOutputDir + "/" + sv_char + "/" + std::string(prn_buf) + "/satellite.diag";
                mSatDiagFiles[satellite.id()].open(path, satellite.id());
                it = mSatDiagFiles.find(satellite.id());
            }

            SatDiagRow row{};
            row.prn = satellite.id().lpp_id().valid ?
                          static_cast<int>(satellite.id().lpp_id().value) + 1 :
                          0;

            // Orbit SSR
            if (satellite.has_orbit_correction()) {
                auto const& orb      = satellite.orbit_correction();
                row.orbit_radial     = orb.delta.x;
                row.orbit_along      = orb.delta.y;
                row.orbit_cross      = orb.delta.z;
                row.orbit_radial_dot = orb.dot_delta.x;
                row.orbit_along_dot  = orb.dot_delta.y;
                row.orbit_cross_dot  = orb.dot_delta.z;
                row.orbit_iod        = orb.iod;
                row.orbit_ssr_iod    = orb.ssr_iod;
                row.has_orbit_ssr    = true;
                row.orbit_epoch.set(orb.epoch_time, mGenerationTime);
            }

            // Clock SSR
            if (satellite.has_clock_correction()) {
                auto const& clk   = satellite.clock_correction_data();
                row.clock_c0      = clk.c0;
                row.clock_c1      = clk.c1;
                row.clock_c2      = clk.c2;
                row.clock_ssr_iod = clk.ssr_iod;
                row.has_clock_ssr = true;
                row.clock_epoch.set(clk.epoch_time, mGenerationTime);
            }

            // Gridded (troposphere / ionosphere residual) assistance data time
            ts::Tai grid_epoch;
            if (mGenerator.mCorrectionData->grid_epoch_time(satellite.id().gnss(), grid_epoch)) {
                row.grid_epoch.set(grid_epoch, mGenerationTime);
            }

            // Iono polynomial
            auto iono_poly = mGenerator.mCorrectionData->ionospheric_polynomial(satellite.id());
            if (iono_poly) {
                row.iono_c00      = iono_poly->c00;
                row.iono_c01      = iono_poly->c01;
                row.iono_c10      = iono_poly->c10;
                row.iono_c11      = iono_poly->c11;
                row.iono_ref_lat  = iono_poly->reference_point_latitude;
                row.iono_ref_lon  = iono_poly->reference_point_longitude;
                row.has_iono_poly = true;
                if (iono_poly->quality_indicator_valid) {
                    row.iono_quality     = iono_poly->quality_indicator;
                    row.has_iono_quality = true;
                }
                row.iono_epoch.set(iono_poly->epoch_time, mGenerationTime);
                if (iono_poly->ssr_iod_valid) {
                    row.has_iono_ssr_iod = true;
                    row.iono_ssr_iod     = iono_poly->ssr_iod;
                }
            }

            {
                uint16_t grid_ssr_iod = 0;
                if (mGenerator.mCorrectionData->grid_ssr_iod(satellite.id().gnss(), grid_ssr_iod)) {
                    row.has_grid_ssr_iod = true;
                    row.grid_ssr_iod     = grid_ssr_iod;
                }
            }

            // Ephemeris IOD
            if (satellite.has_ephemeris_search()) {
                auto const& search = satellite.ephemeris_search();
                row.eph_is_galileo = satellite.id().is_galileo();
                if (search.found) {
                    row.has_eph_iod = true;
                    row.eph_iod     = search.selected.iod;
                    row.eph_iode    = search.selected.iode;
                    row.eph_iodc    = search.selected.iodc;
                    row.eph_iod_nav = search.selected.iod_nav;
                }
                for (size_t i = 0; i < search.valid.size(); i++) {
                    if (i > 0) row.eph_valid_iods += '/';
                    row.eph_valid_iods += std::to_string(search.valid[i].iod);
                }

                // the ephemeris that is closest to the IOD of the SSR orbit correction, the
                // selected one if there is a match
                if (satellite.has_orbit_correction() && !search.valid.empty()) {
                    auto wanted = satellite.orbit_correction().iod;
                    auto best   = search.found ? search.selected : search.valid.front();
                    if (!search.found) {
                        for (auto const& candidate : search.valid) {
                            if (std::abs(iod_offset(satellite.id(), wanted, candidate.iod)) <
                                std::abs(iod_offset(satellite.id(), wanted, best.iod))) {
                                best = candidate;
                            }
                        }
                    }
                    row.has_eph_candidate = true;
                    row.eph_candidate_iod = best.iod;
                    row.eph_iod_offset    = iod_offset(satellite.id(), wanted, best.iod);
                }
            }

            // Geometry + evaluated clock (available if update() completed successfully)
            if (satellite.has_orbit_correction() && satellite.has_clock_correction() &&
                satellite.current_state().true_range > 0.0) {
                auto const& state    = satellite.current_state();
                row.true_range       = state.true_range;
                row.eph_range        = state.eph_range;
                row.orbit            = state.true_range - state.eph_range;
                row.orbit_x          = state.true_position.x - state.eph_position.x;
                row.orbit_y          = state.true_position.y - state.eph_position.y;
                row.orbit_z          = state.true_position.z - state.eph_position.z;
                row.elevation_deg    = state.true_elevation * constant::RAD2DEG;
                row.azimuth_deg      = state.true_azimuth * constant::RAD2DEG;
                row.nadir_deg        = state.true_nadir * constant::RAD2DEG;
                row.eph_clock        = constant::SPEED_OF_LIGHT * -state.eph_clock_bias;
                row.eph_week         = state.eph_week;
                row.eph_toe          = state.eph_toe;
                row.clock_correction = state.clock_correction;
                row.has_geometry     = true;

                if (state.shapiro.valid) {
                    row.shapiro     = state.shapiro.correction;
                    row.has_shapiro = true;
                }
                if (state.earth_solid_tides.valid) {
                    row.solid_tides     = state.earth_solid_tides.displacement;
                    row.solid_tides_x   = state.earth_solid_tides.displacement_vector.x;
                    row.solid_tides_y   = state.earth_solid_tides.displacement_vector.y;
                    row.solid_tides_z   = state.earth_solid_tides.displacement_vector.z;
                    row.has_solid_tides = true;
                }
            }

            // Discard reason
            if (!satellite.enabled() && satellite.disable_reason()) {
                row.discard_reason = satellite.disable_reason();
                row.discard_detail = satellite.disable_detail();
            }

            it->second.write(mGenerationTime, row);
        }
    }

    if (mGenerateDiag && mDiagTropoAll) {
        for (auto const& satellite : mSatellites) {
            write_tropospheric_diag(satellite);
        }
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
        mPhaseDiscontinuity.erase(id);
        mLossCount[id] += 1;
    }

    for (auto& id : active_signals) {
        if (mLockTime.find(id) == mLockTime.end()) {
            VERBOSEF("lli +%s", id.to_string().c_str());
            mLockTime[id] = mGenerationTime;
        }
    }

    if (!active_signals.empty()) mStarted = true;

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

    if (mPhaseAlignment) {
        auto shift_cycles = observation.signal_id().phase_alignment_shift();
        auto shift_meters = shift_cycles * observation.wave_length();
        auto shift_ms     = shift_meters * meter_to_cms;
        delta_phase_range_ms += shift_ms;
    }

    VERBOSEF("%-15s code:  %+.14f (%+.14f)", observation.signal_id().name(), code_range_ms,
             delta_code_range_ms);
    VERBOSEF("%-15s phase: %+.14f (%+.14f)", observation.signal_id().name(), phase_range_ms,
             delta_phase_range_ms);

    rtcm::Signal signal{};
    signal.id                      = observation.signal_id();
    signal.satellite               = satellite.id();
    signal.fine_pseudo_range       = delta_code_range_ms;
    signal.fine_phase_range        = delta_phase_range_ms;
    signal.carrier_to_noise_ratio  = observation.carrier_to_noise_ratio();
    signal.lock_time               = observation.lock_time().seconds;
    signal.require_phase_alignment = mPhaseAlignment;

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
    rtcm.id         = satellite.id();
    rtcm.integer_ms = rtd.integer_ms;
    if (satellite.has_frequency_channel()) rtcm.frequency_channel = satellite.frequency_channel();
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

// Point corrections (not interpolation-residual standard deviations as documented for
// DF218-222) are encoded in `ocs`/`Ics`, following observed real-world network practice rather
// than the strict RTCM 10403.4 field definitions, because Tokoro has no model of
// interpolation-uncertainty growth with distance/height. `ods`/`ohs`/`Ids` are left at zero to
// avoid fabricating a gradient model that does not exist.
void ReferenceStation::build_rtcm_residual(Satellite const& satellite, Float3 const& vrs_llh,
                                           rtcm::Residuals& residuals) NOEXCEPT {
    FUNCTION_SCOPEF("%s", satellite.id().name());

    auto const& correction_data = *mGenerator.mCorrectionData;

    TroposphericCorrection tropo{};
    auto                   has_tropo = correction_data.tropospheric(satellite.id(), vrs_llh, tropo);

    IonosphericCorrection iono{};
    auto                  has_iono = correction_data.ionospheric(satellite.id(), vrs_llh, iono);

    if (!has_tropo && !has_iono) return;

    double frequency = 0.0;
    switch (satellite.id().gnss()) {
    case SatelliteId::Gnss::GPS: frequency = SignalId::GPS_L1_CA.frequency(); break;
    case SatelliteId::Gnss::GLONASS:
        if (!satellite.has_frequency_channel()) return;
        frequency = SignalId::GLONASS_G1_CA.frequency(satellite.frequency_channel());
        break;
    case SatelliteId::Gnss::GALILEO: frequency = SignalId::GALILEO_E1.frequency(); break;
    case SatelliteId::Gnss::BEIDOU: frequency = SignalId::BEIDOU_B1_I.frequency(); break;
    default: return;
    }

    rtcm::Residuals::Satellite rtcm_satellite{};
    rtcm_satellite.id = satellite.id();

    // DF218 (ocs) and DF221 (Ics) are encoded as unsigned fields with ranges 0-0.127m and
    // 0-0.511m respectively; clamp since the point corrections used here can be negative or
    // exceed those ranges (unlike the documented non-negative standard-deviation semantics).
    auto oc = has_tropo ? (tropo.dry + tropo.wet) : 0.0;
    auto lc =
        has_iono ?
            (40.3e10 * (iono.polynomial_residual + iono.grid_residual) / (frequency * frequency)) :
            0.0;
    rtcm_satellite.s_oc = std::min(std::max(oc, 0.0), 0.1275);
    rtcm_satellite.s_od = 0.0;
    rtcm_satellite.s_oh = 0.0;
    rtcm_satellite.s_lc = std::min(std::max(lc, 0.0), 0.5115);
    rtcm_satellite.s_ld = 0.0;

    residuals.satellites.push_back(rtcm_satellite);
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

    rtcm::CommonObservationInfo common{};
    common.reference_station_id = mRtcmReferenceStationId;
    common.clock_steering       = 1;

    rtcm::Observations msm_gps{};
    rtcm::Observations msm_glo{};
    rtcm::Observations msm_gal{};
    rtcm::Observations msm_bds{};
    rtcm::Observations msm_qzs{};

    msm_gps.time = mGenerationTime;
    msm_glo.time = mGenerationTime;
    msm_gal.time = mGenerationTime;
    msm_bds.time = mGenerationTime;
    msm_qzs.time = mGenerationTime;

#ifdef INCLUDE_FORMAT_RINEX
    if (mGenerateRinex) {
        if (mLastRinexEpoch < mGenerationTime) {
            mRinexBuilder.set_antenna_position(mRtcmGroundPosition);
            mRinexBuilder.set_gps_support(mGenerateGps);
            mRinexBuilder.set_glo_support(mGenerateGlo);
            mRinexBuilder.set_gal_support(mGenerateGal);
            mRinexBuilder.set_bds_support(mGenerateBds);
            mRinexBuilder.set_qzs_support(mGenerateQzs);
            for (auto& satellite : mSatellites) {
                if (!satellite.enabled() || !satellite.has_frequency_channel()) continue;
                auto slot = satellite.id().prn();
                if (!slot.valid) continue;
                mRinexBuilder.set_glo_frequency_channel(slot.value, satellite.frequency_channel());
            }

            std::vector<SatelliteId> rinex_satellites;
            for (auto& satellite : mSatellites) {
                if (!satellite.enabled()) continue;
                if (!mGenerateGps && satellite.id().gnss() == SatelliteId::Gnss::GPS) continue;
                if (!mGenerateGlo && satellite.id().gnss() == SatelliteId::Gnss::GLONASS) continue;
                if (!mGenerateGal && satellite.id().gnss() == SatelliteId::Gnss::GALILEO) continue;
                if (!mGenerateBds && satellite.id().gnss() == SatelliteId::Gnss::BEIDOU) continue;
                if (!mGenerateQzs && satellite.id().gnss() == SatelliteId::Gnss::QZSS) continue;
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
                if (!mGenerateQzs && satellite.id().gnss() == SatelliteId::Gnss::QZSS) continue;
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
#endif

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

    if (mGenerateQzs) {
        for (auto& satellite : mSatellites) {
            if (!satellite.enabled()) continue;
            if (satellite.id().gnss() != SatelliteId::Gnss::QZSS) continue;
            build_rtcm_satellite(satellite, msm_qzs);
        }
    }

    if (mGenerateResiduals) {
        auto vrs_llh = ecef_to_llh(mGroundPosition, ellipsoid::gWgs84);

        rtcm::Residuals gps_residuals{};
        rtcm::Residuals gal_residuals{};
        rtcm::Residuals bds_residuals{};
        rtcm::Residuals glo_residuals{};
        glo_residuals.time                 = mGenerationTime;
        glo_residuals.reference_station_id = mRtcmReferenceStationId;
        glo_residuals.n_refs               = 1;
        gps_residuals.time                 = mGenerationTime;
        gps_residuals.reference_station_id = mRtcmReferenceStationId;
        gps_residuals.n_refs               = 1;
        gal_residuals.time                 = mGenerationTime;
        gal_residuals.reference_station_id = mRtcmReferenceStationId;
        gal_residuals.n_refs               = 1;
        bds_residuals.time                 = mGenerationTime;
        bds_residuals.reference_station_id = mRtcmReferenceStationId;
        bds_residuals.n_refs               = 1;

        for (auto& satellite : mSatellites) {
            if (!satellite.enabled()) continue;
            switch (satellite.id().gnss()) {
            case SatelliteId::Gnss::GPS:
                if (mGenerateGps) build_rtcm_residual(satellite, vrs_llh, gps_residuals);
                break;
            case SatelliteId::Gnss::GLONASS:
                if (mGenerateGlo) build_rtcm_residual(satellite, vrs_llh, glo_residuals);
                break;
            case SatelliteId::Gnss::GALILEO:
                if (mGenerateGal) build_rtcm_residual(satellite, vrs_llh, gal_residuals);
                break;
            case SatelliteId::Gnss::BEIDOU:
                if (mGenerateBds) build_rtcm_residual(satellite, vrs_llh, bds_residuals);
                break;
            default: break;
            }
        }

        if (!gps_residuals.satellites.empty()) {
            messages.push_back(rtcm::generate_1030(gps_residuals));
        }
        if (!glo_residuals.satellites.empty()) {
            messages.push_back(rtcm::generate_1031(glo_residuals));
        }
        if (!bds_residuals.satellites.empty()) {
            messages.push_back(rtcm::generate_1303(bds_residuals));
        }
        if (!gal_residuals.satellites.empty()) {
            messages.push_back(rtcm::generate_1304(gal_residuals));
        }
    }

    DEBUGF("GPS: satellites=%2zu, signals=%2zu", msm_gps.satellites.size(), msm_gps.signals.size());
    DEBUGF("GLO: satellites=%2zu, signals=%2zu", msm_glo.satellites.size(), msm_glo.signals.size());
    DEBUGF("GAL: satellites=%2zu, signals=%2zu", msm_gal.satellites.size(), msm_gal.signals.size());
    DEBUGF("BDS: satellites=%2zu, signals=%2zu", msm_bds.satellites.size(), msm_bds.signals.size());
    DEBUGF("QZS: satellites=%2zu, signals=%2zu", msm_qzs.satellites.size(), msm_qzs.signals.size());
    auto will_generate_gps =
        mGenerateGps && msm_gps.satellites.size() > 0 && msm_gps.signals.size() > 0;
    auto will_generate_glo =
        mGenerateGlo && msm_glo.satellites.size() > 0 && msm_glo.signals.size() > 0;
    auto will_generate_gal =
        mGenerateGal && msm_gal.satellites.size() > 0 && msm_gal.signals.size() > 0;
    auto will_generate_bds =
        mGenerateBds && msm_bds.satellites.size() > 0 && msm_bds.signals.size() > 0;
    auto will_generate_qzs =
        mGenerateQzs && msm_qzs.satellites.size() > 0 && msm_qzs.signals.size() > 0;

    // The GNSS indicator bits in 1005/1006 must reflect what is actually being delivered this
    // epoch, not merely what was requested. Advertising a system that has no observations (e.g.
    // GLONASS requested but not yet supported/no satellites available) misleads receivers that
    // expect a corresponding MSM to follow.
    messages.push_back(rtcm::generate_1006(reference_station, will_generate_gps, will_generate_glo,
                                           will_generate_gal));

    rtcm::AntennaDescriptor antenna_descriptor{};
    antenna_descriptor.antenna_descriptor        = mAntennaDescriptor;
    antenna_descriptor.antenna_setup_id          = 0;
    antenna_descriptor.antenna_serial_number     = "";
    antenna_descriptor.receiver_type_descriptor  = mReceiverDescriptor;
    antenna_descriptor.receiver_firmware_version = "";
    antenna_descriptor.receiver_serial_number    = "";
    messages.push_back(rtcm::generate_1033(reference_station, antenna_descriptor));

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

    if (will_generate_glo) {
        // The synthetic observations have no inter-frequency code-phase biases
        rtcm::BiasInformation bias{};
        bias.reference_station_id = mRtcmReferenceStationId;
        bias.indicator            = true;
        bias.mask                 = 0xF;
        bias.l1_ca                = 0.0;
        bias.l1_p                 = 0.0;
        bias.l2_ca                = 0.0;
        bias.l2_p                 = 0.0;
        messages.push_back(rtcm::generate_1230(bias));
    }

    if (will_generate_gps) {
        auto last_msm =
            !will_generate_glo && !will_generate_gal && !will_generate_bds && !will_generate_qzs;
        auto msm_messages =
            rtcm::generate_msm(mRtcmMsmType, last_msm, rtcm::GenericGnssId::GPS, common, msm_gps);
        for (auto& message : msm_messages) {
            messages.push_back(std::move(message));
        }
    }

    if (will_generate_glo) {
        auto last_msm     = !will_generate_gal && !will_generate_bds && !will_generate_qzs;
        auto msm_messages = rtcm::generate_msm(mRtcmMsmType, last_msm, rtcm::GenericGnssId::GLONASS,
                                               common, msm_glo);
        for (auto& message : msm_messages) {
            messages.push_back(std::move(message));
        }
    }

    if (will_generate_gal) {
        auto last_msm     = !will_generate_bds && !will_generate_qzs;
        auto msm_messages = rtcm::generate_msm(mRtcmMsmType, last_msm, rtcm::GenericGnssId::GALILEO,
                                               common, msm_gal);
        for (auto& message : msm_messages) {
            messages.push_back(std::move(message));
        }
    }

    if (will_generate_bds) {
        auto msm_messages = rtcm::generate_msm(mRtcmMsmType, !will_generate_qzs,
                                               rtcm::GenericGnssId::BEIDOU, common, msm_bds);
        for (auto& message : msm_messages) {
            messages.push_back(std::move(message));
        }
    }

    if (will_generate_qzs) {
        auto msm_messages =
            rtcm::generate_msm(mRtcmMsmType, true, rtcm::GenericGnssId::QZSS, common, msm_qzs);
        for (auto& message : msm_messages) {
            messages.push_back(std::move(message));
        }
    }

    return messages;
}

//
//
//

Generator::Generator() NOEXCEPT {
    FUNCTION_SCOPE();
    mIodConsistencyCheck                         = false;
    mSsrGenerationConsistencyCheck               = false;
    mUseReceptionTimeForOrbitAndClockCorrections = false;
    mUseOrbitCorrectionInIteration               = false;
    mIgnoreBitmask                               = false;
}

Generator::~Generator() = default;

bool Generator::set_orbit_frame(char const* frame) NOEXCEPT {
    if (!frame) {
        mOrbitFrames.reset();
        return true;
    }
    auto frames = OrbitFrames::create(frame);
    if (!frames) return false;
    INFOF("broadcast orbits transformed to %s", frames->target_name());
    mOrbitFrames = std::move(frames);
    return true;
}

bool Generator::set_orbit_source_frame(SatelliteId::Gnss gnss, char const* frame) NOEXCEPT {
    if (!mOrbitFrames) {
        ERRORF("the orbit source frame requires an orbit frame");
        return false;
    }
    return mOrbitFrames->set_source(gnss, frame);
}

void Generator::set_fake_correction_point_set(uint16_t set_id, double reference_point_latitude,
                                              double reference_point_longitude,
                                              long   number_of_steps_latitude,
                                              long   number_of_steps_longitude,
                                              double step_of_latitude,
                                              double step_of_longitude) NOEXCEPT {
    CorrectionPointSet cps{};
    cps.set_id                    = set_id;
    cps.reference_point_latitude  = reference_point_latitude;
    cps.reference_point_longitude = reference_point_longitude;
    cps.number_of_steps_latitude  = number_of_steps_latitude;
    cps.number_of_steps_longitude = number_of_steps_longitude;
    cps.step_of_latitude          = step_of_latitude;
    cps.step_of_longitude         = step_of_longitude;
    cps.grid_point_count    = (number_of_steps_longitude + 1) * (number_of_steps_latitude + 1);
    cps.bitmask             = 0xFFFFFFFFFFFFFFFF;
    mFakeCorrectionPointSet = std::unique_ptr<CorrectionPointSet>(new CorrectionPointSet(cps));
}

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

    if (!mCorrectionPointSet && mFakeCorrectionPointSet) {
        WARNF("no correction point set found, using fake correction point set (id=%u)",
              mFakeCorrectionPointSet->set_id);
        mCorrectionPointSet =
            std::unique_ptr<CorrectionPointSet>(new CorrectionPointSet(*mFakeCorrectionPointSet));
    }

    if (!mCorrectionPointSet) {
        WARNF("no correction point set found");
        return false;
    }

    if (!mCorrectionData) {
        mCorrectionData = std::unique_ptr<CorrectionData>(new CorrectionData());
    }
    mCorrectionData->correction_point_set = mCorrectionPointSet.get();
    mCorrectionData->max_orbit_age        = mMaxOrbitAge;
    mCorrectionData->max_clock_age        = mMaxClockAge;
    mCorrectionData->max_bias_age         = mMaxBiasAge;
    mCorrectionData->max_iono_age         = mMaxIonoAge;
    mCorrectionData->max_tropo_age        = mMaxTropoAge;
    mCorrectionData->iono_polynomial_only = mIonoPolynomialOnly;
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
            decode::reference_point_latitude_r16(array.referencePointLatitude_r16);
        auto reference_point_longitude =
            decode::reference_point_longitude_r16(array.referencePointLongitude_r16);
        auto number_of_steps_latitude =
            decode::number_of_steps_latitude_r16(array.numberOfStepsLatitude_r16);
        auto number_of_steps_longitude =
            decode::number_of_steps_longitude_r16(array.numberOfStepsLongitude_r16);
        auto step_of_latitude  = decode::step_of_latitude_r16(array.stepOfLatitude_r16);
        auto step_of_longitude = decode::step_of_longitude_r16(array.stepOfLongitude_r16);

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
            gnss_id != GNSS_ID__gnss_id_bds && gnss_id != GNSS_ID__gnss_id_glonass &&
            gnss_id != GNSS_ID__gnss_id_qzss) {
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
                               ephemeris::Ephemeris& eph, EphemerisSearch* search) const NOEXCEPT {
    FUNCTION_SCOPE();
    if (sv_id.gnss() == SatelliteId::Gnss::GPS) {
        auto it = mGpsEphemeris.find(sv_id);
        if (it == mGpsEphemeris.end()) {
            mMissingEphemeris.push_back({sv_id, iod});
            return false;
        }
        auto& list = it->second;
        if (search) search->total = list.size();

        auto gps_time = ts::Gps(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                     ephemeris.is_valid(gps_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(gps_time)) continue;
            if (search) {
                search->valid.push_back({ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc, 0,
                                         ephemeris.toe, ephemeris.week_number});
            }
            // TODO(ewasjon): [low-priority] Due to a issue for one datafeed the lpp_iod for GPS
            // uses IODE instead of IODC. As IODE by definition is the 8 lower bits of IODC let's
            // just compare them. This could cause problem with ephemeris >6 hours but the time
            // validity check should block that
            auto eph_iode    = ephemeris.lpp_iod & 0xFF;
            auto wanted_iode = iod & 0xFF;
            if (eph_iode != wanted_iode && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            if (search) {
                search->found    = true;
                search->selected = search->valid.back();
            }
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        mMissingEphemeris.push_back({sv_id, iod});
        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::GLONASS) {
        auto it = mGloEphemeris.find(sv_id);
        if (it == mGloEphemeris.end()) {
            mMissingEphemeris.push_back({sv_id, iod});
            return false;
        }
        auto& list = it->second;
        if (search) search->total = list.size();

        auto glo_time = ts::Glo(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            auto tb = ephemeris.reference_time.time_of_day().full_seconds();
            VERBOSEF("  %8.0f | %4u |%s%s", tb, ephemeris.lpp_iod,
                     ephemeris.is_valid(glo_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(glo_time)) continue;
            if (search) search->valid.push_back({ephemeris.lpp_iod, 0, 0, 0, tb, 0});
            // the SSR iod of GLONASS is tb (7 bits)
            if ((ephemeris.lpp_iod & 0x7F) != (iod & 0x7F) && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            if (search) {
                search->found    = true;
                search->selected = search->valid.back();
            }
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        mMissingEphemeris.push_back({sv_id, iod});
        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::GALILEO) {
        auto it = mGalEphemeris.find(sv_id);
        if (it == mGalEphemeris.end()) {
            mMissingEphemeris.push_back({sv_id, iod});
            return false;
        }
        auto& list = it->second;
        if (search) search->total = list.size();

        auto gal_time = ts::Gst(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iod_nav,
                     ephemeris.is_valid(gal_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(gal_time)) continue;
            if (search) {
                search->valid.push_back({ephemeris.lpp_iod, 0, 0, ephemeris.iod_nav, ephemeris.toe,
                                         ephemeris.week_number});
            }
            if (ephemeris.lpp_iod != iod && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            if (search) {
                search->found    = true;
                search->selected = search->valid.back();
            }
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        mMissingEphemeris.push_back({sv_id, iod});
        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::BEIDOU) {
        auto it = mBdsEphemeris.find(sv_id);
        if (it == mBdsEphemeris.end()) {
            mMissingEphemeris.push_back({sv_id, iod});
            return false;
        }
        auto& list = it->second;
        if (search) search->total = list.size();

        auto bds_time = ts::Bdt(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                     ephemeris.is_valid(bds_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(bds_time)) continue;
            if (search) {
                search->valid.push_back({ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc, 0,
                                         ephemeris.toe, ephemeris.week_number});
            }
            if (ephemeris.lpp_iod != iod && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            if (search) {
                search->found    = true;
                search->selected = search->valid.back();
            }
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        mMissingEphemeris.push_back({sv_id, iod});
        return false;
    } else if (sv_id.gnss() == SatelliteId::Gnss::QZSS) {
        auto it = mQzsEphemeris.find(sv_id);
        if (it == mQzsEphemeris.end()) {
            mMissingEphemeris.push_back({sv_id, iod});
            return false;
        }
        auto& list = it->second;
        if (search) search->total = list.size();

        auto qzs_time = ts::Gps(time);
        VERBOSEF("searching: %s %u", sv_id.name(), iod);
        for (auto& ephemeris : list) {
            VERBOSEF("  %4u %8.0f %8.0f | %4u %4u %4u |%s%s", ephemeris.week_number, ephemeris.toe,
                     ephemeris.toc, ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc,
                     ephemeris.is_valid(qzs_time) ? " [time]" : "",
                     ephemeris.lpp_iod == iod ? " [iod]" : "");
            if (!ephemeris.is_valid(qzs_time)) continue;
            if (search) {
                search->valid.push_back({ephemeris.lpp_iod, ephemeris.iode, ephemeris.iodc, 0,
                                         ephemeris.toe, ephemeris.week_number});
            }
            auto eph_iode    = ephemeris.lpp_iod & 0xFF;
            auto wanted_iode = iod & 0xFF;
            if (eph_iode != wanted_iode && mIodConsistencyCheck) continue;
            eph = ephemeris::Ephemeris(ephemeris);
            if (search) {
                search->found    = true;
                search->selected = search->valid.back();
            }
            VERBOSEF("found: %s %u", sv_id.name(), eph.iod());
            return true;
        }

        mMissingEphemeris.push_back({sv_id, iod});
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
    if (list.size() >= mEphemerisMaxCache) {
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
    if (list.size() >= mEphemerisMaxCache) {
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
    if (list.size() >= mEphemerisMaxCache) {
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

void Generator::process_ephemeris(ephemeris::QzsEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_qzs_prn(ephemeris.prn);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: QZS %d", ephemeris.prn);
        return;
    }

    auto& list = mQzsEphemeris[satellite_id];

    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    if (list.size() >= mEphemerisMaxCache) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::QzsEphemeris const& a, ephemeris::QzsEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

void Generator::process_ephemeris(ephemeris::GloEphemeris const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();
    auto satellite_id = SatelliteId::from_glo_slot(ephemeris.slot_number);
    if (!satellite_id.is_valid()) {
        VERBOSEF("invalid satellite id: GLO %d", ephemeris.slot_number);
        return;
    }

    auto& list = mGloEphemeris[satellite_id];

    for (auto& eph : list) {
        if (eph.match(ephemeris)) {
            VERBOSEF("duplicate ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
            return;
        }
    }

    if (list.size() >= mEphemerisMaxCache) {
        WARNF("removing oldest ephemeris: %s (size=%zu)", satellite_id.name(), list.size());
        list.erase(list.begin());
    }

    list.push_back(ephemeris);
    std::sort(list.begin(), list.end(),
              [](ephemeris::GloEphemeris const& a, ephemeris::GloEphemeris const& b) {
                  return a.compare(b);
              });

    DEBUGF("ephemeris: %s (iod=%u)", satellite_id.name(), ephemeris.lpp_iod);
}

bool Generator::get_grid_position(int east, int north, double* lat, double* lon) const NOEXCEPT {
    if (!mCorrectionPointSet) return false;
    ASSERT(lat, "lat is null");
    ASSERT(lon, "lon is null");
    *lat = mCorrectionPointSet->reference_point_latitude -
           north * mCorrectionPointSet->step_of_latitude;
    *lon = mCorrectionPointSet->reference_point_longitude +
           east * mCorrectionPointSet->step_of_longitude;
    return true;
}

bool Generator::get_grid_cell_center_position(int east, int north, double* lat,
                                              double* lon) const NOEXCEPT {
    if (!get_grid_position(east, north, lat, lon)) return false;
    ASSERT(mCorrectionPointSet, "correction point set not initialized");
    *lat -= mCorrectionPointSet->step_of_latitude * 0.5;
    *lon += mCorrectionPointSet->step_of_longitude * 0.5;
    return true;
}

#ifdef ENABLE_TOKORO_SNAPSHOT
SnapshotInput ReferenceStation::snapshot(ts::Tai const& time) const NOEXCEPT {
    SnapshotInput input = mGenerator.snapshot();

    input.time                                      = time;
    input.position                                  = mGroundPosition;
    input.config.itrf_position                      = mGroundPosition;
    input.config.rtcm_position                      = mRtcmGroundPosition;
    input.config.gps                                = mGenerateGps;
    input.config.glo                                = mGenerateGlo;
    input.config.gal                                = mGenerateGal;
    input.config.bds                                = mGenerateBds;
    input.config.qzs                                = mGenerateQzs;
    input.config.shapiro_correction                 = mShapiroCorrection;
    input.config.earth_solid_tides_correction       = mEarthSolidTidesCorrection;
    input.config.phase_windup_correction            = mPhaseWindupCorrection;
    input.config.antenna_phase_variation_correction = mAntennaPhaseVariation;
    input.config.tropospheric_height_correction     = mTropoHeightCorrection;
    input.config.elevation_mask                     = mElevationMask;
    input.config.phase_range_rate                   = mPhaseRangeRate;
    input.config.negative_phase_windup              = mNegativePhaseWindup;
    input.config.negative_correction_sign           = mNegativeCorrectionSign;
    input.config.require_code_bias                  = mRequireCodeBias;
    input.config.require_phase_bias                 = mRequirePhaseBias;
    input.config.require_tropo                      = mRequireTropo;
    input.config.require_iono                       = mRequireIono;
    input.config.use_tropospheric_model             = mUseTroposphericModel;
    input.config.use_ionospheric_height_correction  = mUseIonosphericHeightCorrection;

    return input;
}

SnapshotInput Generator::snapshot() const NOEXCEPT {
    SnapshotInput input;

    for (auto const& pair : mGpsEphemeris) {
        for (auto const& eph : pair.second) {
            SnapshotEphemeris snapshot_eph;
            msgpack::Packer   packer(snapshot_eph.data);
            ephemeris::Ephemeris(eph).msgpack_pack(packer);
            input.ephemeris.push_back(snapshot_eph);
        }
    }
    for (auto const& pair : mGalEphemeris) {
        for (auto const& eph : pair.second) {
            SnapshotEphemeris snapshot_eph;
            msgpack::Packer   packer(snapshot_eph.data);
            ephemeris::Ephemeris(eph).msgpack_pack(packer);
            input.ephemeris.push_back(snapshot_eph);
        }
    }
    for (auto const& pair : mBdsEphemeris) {
        for (auto const& eph : pair.second) {
            SnapshotEphemeris snapshot_eph;
            msgpack::Packer   packer(snapshot_eph.data);
            ephemeris::Ephemeris(eph).msgpack_pack(packer);
            input.ephemeris.push_back(snapshot_eph);
        }
    }
    for (auto const& pair : mQzsEphemeris) {
        for (auto const& eph : pair.second) {
            SnapshotEphemeris snapshot_eph;
            msgpack::Packer   packer(snapshot_eph.data);
            ephemeris::Ephemeris(eph).msgpack_pack(packer);
            input.ephemeris.push_back(snapshot_eph);
        }
    }
    for (auto const& pair : mGloEphemeris) {
        for (auto const& eph : pair.second) {
            SnapshotEphemeris snapshot_eph;
            msgpack::Packer   packer(snapshot_eph.data);
            ephemeris::Ephemeris(eph).msgpack_pack(packer);
            input.ephemeris.push_back(snapshot_eph);
        }
    }

    if (mCorrectionData) {
        mCorrectionData->snapshot(input.orbit_corrections, input.clock_corrections,
                                  input.code_biases, input.phase_biases,
                                  input.ionospheric_polynomials, input.grid_data);
    }

    if (mCorrectionPointSet) {
        input.correction_point_set.set_id           = mCorrectionPointSet->set_id;
        input.correction_point_set.grid_point_count = mCorrectionPointSet->grid_point_count;
        input.correction_point_set.reference_point_latitude =
            mCorrectionPointSet->reference_point_latitude;
        input.correction_point_set.reference_point_longitude =
            mCorrectionPointSet->reference_point_longitude;
        input.correction_point_set.step_of_latitude  = mCorrectionPointSet->step_of_latitude;
        input.correction_point_set.step_of_longitude = mCorrectionPointSet->step_of_longitude;
        input.correction_point_set.number_of_steps_latitude =
            mCorrectionPointSet->number_of_steps_latitude;
        input.correction_point_set.number_of_steps_longitude =
            mCorrectionPointSet->number_of_steps_longitude;
        input.correction_point_set.bitmask = mCorrectionPointSet->bitmask;
    }

    return input;
}

void Generator::load_snapshot(SnapshotInput const& input) NOEXCEPT {
    FUNCTION_SCOPE();
    TRACEF("clear ephemeris");
    mGpsEphemeris.clear();
    mGalEphemeris.clear();
    mBdsEphemeris.clear();
    mQzsEphemeris.clear();
    mGloEphemeris.clear();

    TRACEF("load ephemeris");
    for (auto const& snapshot_eph : input.ephemeris) {
        msgpack::Unpacker    unpacker(snapshot_eph.data.data(), snapshot_eph.data.size());
        ephemeris::Ephemeris eph;
        if (!eph.msgpack_unpack(unpacker)) {
            WARNF("failed to unpack ephemeris");
            continue;
        }

        if (eph.mType == ephemeris::Ephemeris::Type::GPS) {
            auto id = SatelliteId::from_gps_prn(eph.gps_ephemeris.prn);
            DEBUGF("loaded ephemeris: %s", id.name());
            mGpsEphemeris[id].push_back(eph.gps_ephemeris);
        } else if (eph.mType == ephemeris::Ephemeris::Type::GAL) {
            auto id = SatelliteId::from_gal_prn(eph.gal_ephemeris.prn);
            DEBUGF("loaded ephemeris: %s", id.name());
            mGalEphemeris[id].push_back(eph.gal_ephemeris);
        } else if (eph.mType == ephemeris::Ephemeris::Type::BDS) {
            auto id = SatelliteId::from_bds_prn(eph.bds_ephemeris.prn);
            DEBUGF("loaded ephemeris: %s", id.name());
            mBdsEphemeris[id].push_back(eph.bds_ephemeris);
        } else if (eph.mType == ephemeris::Ephemeris::Type::QZS) {
            auto id = SatelliteId::from_qzs_prn(eph.qzs_ephemeris.prn);
            DEBUGF("loaded ephemeris: %s", id.name());
            mQzsEphemeris[id].push_back(eph.qzs_ephemeris);
        } else if (eph.mType == ephemeris::Ephemeris::Type::GLO) {
            auto id = SatelliteId::from_glo_slot(eph.glo_ephemeris.slot_number);
            DEBUGF("loaded ephemeris: %s", id.name());
            mGloEphemeris[id].push_back(eph.glo_ephemeris);
        } else {
            WARNF("unknown ephemeris type: %d", eph.mType);
        }
    }

    TRACEF("clear correction data");
    mCorrectionData.reset();
    mCorrectionData = std::make_unique<CorrectionData>();

    TRACEF("load correction point set");
    if (input.correction_point_set.grid_point_count > 0) {
        mCorrectionPointSet                   = std::make_unique<CorrectionPointSet>();
        mCorrectionPointSet->set_id           = input.correction_point_set.set_id;
        mCorrectionPointSet->grid_point_count = input.correction_point_set.grid_point_count;
        mCorrectionPointSet->reference_point_latitude =
            input.correction_point_set.reference_point_latitude;
        mCorrectionPointSet->reference_point_longitude =
            input.correction_point_set.reference_point_longitude;
        mCorrectionPointSet->step_of_latitude  = input.correction_point_set.step_of_latitude;
        mCorrectionPointSet->step_of_longitude = input.correction_point_set.step_of_longitude;
        mCorrectionPointSet->number_of_steps_latitude =
            input.correction_point_set.number_of_steps_latitude;
        mCorrectionPointSet->number_of_steps_longitude =
            input.correction_point_set.number_of_steps_longitude;
        mCorrectionPointSet->bitmask          = input.correction_point_set.bitmask;
        mCorrectionData->correction_point_set = mCorrectionPointSet.get();
        mCorrectionData->max_orbit_age        = mMaxOrbitAge;
        mCorrectionData->max_clock_age        = mMaxClockAge;
        mCorrectionData->max_bias_age         = mMaxBiasAge;
        mCorrectionData->max_iono_age         = mMaxIonoAge;
        mCorrectionData->max_tropo_age        = mMaxTropoAge;
        mCorrectionData->iono_polynomial_only = mIonoPolynomialOnly;
        mCorrectionData->iono_polynomial_only = mIonoPolynomialOnly;
    }

    TRACEF("load correction data");
    mCorrectionData->load_snapshot(input.orbit_corrections, input.clock_corrections,
                                   input.code_biases, input.phase_biases,
                                   input.ionospheric_polynomials, input.grid_data);
}
#endif

}  // namespace tokoro
}  // namespace generator
