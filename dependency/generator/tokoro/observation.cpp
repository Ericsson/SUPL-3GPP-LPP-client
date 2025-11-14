#include "observation.hpp"
#include "coordinate.hpp"
#include "coordinates/eci.hpp"
#include "coordinates/enu.hpp"
#include "data/correction.hpp"
#include "models/astronomical_arguments.hpp"
#include "models/geoid.hpp"
#include "models/helper.hpp"
#include "models/mops.hpp"
#include "models/nutation.hpp"
#include "models/phase_windup.hpp"
#include "models/shapiro.hpp"
#include "models/sun_moon.hpp"
#include "satellite.hpp"

#include <cmath>
#include <loglet/loglet.hpp>
#include <maths/mat3.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE2(tokoro, obs);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, obs)

namespace generator {
namespace tokoro {

Observation::Observation(Satellite const& satellite, SignalId signal_id, Float3 location) NOEXCEPT
    : mSvId(satellite.id()),
      mSignalId(signal_id),
      mCurrent{&satellite.current_state()},
      mNext{&satellite.next_state()} {
    mIsValid = true;

    // TODO(ewasjon): For GLONASS, the frequency depends on the channel number
    mFrequency  = signal_id.frequency();
    mWavelength = constant::SPEED_OF_LIGHT / mFrequency / 1000.0;

    mCarrierToNoiseRatio = 0.0;
    mLockTime            = {
        mCurrent->reception_time,
        0.0,
        true,
    };
    mNegativePhaseWindup = false;

    mRequirePhaseBias               = true;
    mRequireCodeBias                = true;
    mRequireTropospheric            = true;
    mRequireIonospheric             = true;
    mUseTroposphericModel           = false;
    mUseIonosphericHeightCorrection = false;

    mClockCorrection = Correction{satellite.clock_correction(), true};
    mCodeBias        = Correction{0.0, false};
    mPhaseBias       = Correction{0.0, false};
    mTropospheric = TroposphericDelay{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, false, false};
    mIonospheric  = IonosphericDelay{0.0, 0.0, false, 0.0, 0.0, 0.0, false};
    mAntennaPhaseVariation = Correction{0.0, false};

    mGroundPosition = location;
    mGroundLlh      = ecef_to_llh(location, ellipsoid::gWgs84);

    auto mapping = hydrostatic_mapping_function(mCurrent->reception_time, mGroundLlh,
                                                mCurrent->true_elevation);
    mTropospheric.mapping_hydrostatic = mapping.hydrostatic;
    mTropospheric.mapping_wet         = mapping.wet;
}

void Observation::compute_tropospheric_height() NOEXCEPT {
    VSCOPE_FUNCTION();

    auto ellipsoidal_height = mGroundLlh.z;
    auto geoid_height       = Geoid::height(mGroundLlh.x, mGroundLlh.y, Geoid::Model::EMBEDDED);

    HydrostaticAndWetDelay alt_0{};
    HydrostaticAndWetDelay alt_eh{};
    auto                   mops_0 =
        mops_tropospheric_delay(mCurrent->reception_time, mGroundLlh.x, 0.0, geoid_height, alt_0);
    auto mops_eh = mops_tropospheric_delay(mCurrent->reception_time, mGroundLlh.x,
                                           ellipsoidal_height, geoid_height, alt_eh);
    if (mops_0 && mops_eh) {
        mTropospheric.height_mapping_hydrostatic = alt_eh.hydrostatic / alt_0.hydrostatic;
        mTropospheric.height_mapping_wet         = alt_eh.wet / alt_0.wet;
        mTropospheric.valid_height_mapping       = true;
    } else {
        WARNF("failed to compute tropospheric height correction");
        mTropospheric.valid_height_mapping = false;
    }

    if (mops_eh) {
        mTropospheric.model_hydrostatic = alt_eh.hydrostatic;
        mTropospheric.model_wet         = alt_eh.wet;
        mTropospheric.valid_model       = true;
    } else {
        mTropospheric.valid_model = false;
    }
}

void Observation::update_lock_time(LockTime const& lock_time) NOEXCEPT {
    mLockTime = lock_time;
}

void Observation::compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->phase_bias.find(mSignalId);
    if (it == signals->phase_bias.end()) return;
    mPhaseBias = Correction{it->second.bias, true};
}

void Observation::compute_code_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->code_bias.find(mSignalId);
    if (it == signals->code_bias.end()) return;
    mCodeBias = Correction{it->second.bias, true};
}

void Observation::compute_tropospheric(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mSvId.name());

    TroposphericCorrection correction{};
    if (!correction_data.tropospheric(mSvId, mGroundLlh, correction)) {
        VERBOSEF("tropospheric correction not found");
        return;
    }

    if (mTropospheric.valid) {
        VERBOSEF("tropospheric correction already computed");
        return;
    }

    mTropospheric.hydrostatic = correction.dry;
    mTropospheric.wet         = correction.wet;
    mTropospheric.valid       = true;
}

static double compute_vtec_mapping(double altitude, double elevation) {
    auto r       = ellipsoid::gWgs84.semi_major_axis + altitude;
    auto h       = 506.7e3;
    auto alpha   = 0.9782;
    auto sin     = std::sin(alpha * (constant::PI / 2.0 - elevation));
    auto height  = r / (r + h);
    auto mapping = std::sqrt(1 - height * height * sin * sin);
    return mapping;
}

void Observation::compute_ionospheric(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    IonosphericCorrection correction{};
    if (!correction_data.ionospheric(mSvId, mGroundLlh, correction)) {
        VERBOSEF("ionospheric correction not found");
        return;
    }

    mIonospheric.grid_residual = correction.grid_residual;
    mIonospheric.poly_residual = correction.polynomial_residual;
    mIonospheric.valid         = true;

    mIonospheric.quality       = correction.quality;
    mIonospheric.quality_valid = correction.quality_valid;
    mIonospheric.vtec_mapping  = compute_vtec_mapping(0, mCurrent->true_elevation);

    auto ellipsoidal_height = mGroundLlh.z;
    auto vtec_mf_0          = compute_vtec_mapping(0, mCurrent->true_elevation);
    auto vtec_mf_alt        = compute_vtec_mapping(ellipsoidal_height, mCurrent->true_elevation);
    mIonospheric.height_correction = vtec_mf_0 / vtec_mf_alt;
}

void Observation::compute_antenna_phase_variation(format::antex::Antex const& antex) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    mAntennaPhaseVariation.valid = false;

    format::antex::PhaseVariation phase_variation{};
    if (!antex.phase_variation(mSvId, mSignalId, mCurrent->reception_time, mCurrent->true_azimuth,
                               mCurrent->true_nadir, phase_variation)) {
        VERBOSEF("antenna phase variation not found");
        return;
    }

    mAntennaPhaseVariation.correction = phase_variation.value;
    mAntennaPhaseVariation.valid      = true;
}

void Observation::compute_ranges() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    auto time_delta =
        (mNext->reception_time.timestamp() - mCurrent->reception_time.timestamp()).full_seconds();

    // Discard this observation if it is missing important corrections, however, we still want to
    // compute the ranges to report via DATA TRACING.
    if (mRequireCodeBias && !has_code_bias()) {
        WARNF("discarded: %s %s - no code bias", mSvId.name(), mSignalId.name());
        discard();
    }
    if (mRequirePhaseBias && !has_phase_bias()) {
        WARNF("discarded: %s %s - no phase bias", mSvId.name(), mSignalId.name());
        discard();
    }

    if (mRequireTropospheric && !has_tropospheric()) {
        WARNF("discarded: %s %s - no tropospheric correction", mSvId.name(), mSignalId.name());
        discard();
    }
    if (mRequireIonospheric && !has_ionospheric()) {
        WARNF("discarded: %s %s - no ionospheric correction", mSvId.name(), mSignalId.name());
        discard();
    }

    auto true_range0 = mCurrent->true_range;
    auto true_range1 = mNext->true_range;
    VERBOSEF("true_range:   %+24.10f (%gm,%gm/s)", mCurrent->true_range,
             (true_range1 - true_range0) / time_delta);

    auto clock_bias0 = constant::SPEED_OF_LIGHT * -mCurrent->eph_clock_bias;
    auto clock_bias1 = constant::SPEED_OF_LIGHT * -mNext->eph_clock_bias;
    VERBOSEF("clock_bias:   %+24.10f (%gs,%gm/s)", clock_bias0, -mCurrent->eph_clock_bias,
             (clock_bias1 - clock_bias0) / time_delta);

    // TODO(ewasjon): Think more about how we compute the carrier-to-noise ratio
    auto carrier_to_noise_ratio = 45.0;
    if (mIonospheric.quality_valid) {
        // TODO(ewasjon): What formula should we use to compute the carrier-to-noise ratio?
        auto delta = mIonospheric.quality - 33.6664;
        if (delta > 0) delta = 0;
        auto value = delta * delta * 0.002;
        carrier_to_noise_ratio += value;
        VERBOSEF("cnr:          %+24.10f (%g)", carrier_to_noise_ratio, mIonospheric.quality);
    }

    if (mCurrent->true_elevation < constant::DEG2RAD * 40.0) {
        // TODO(ewasjon): What formula should we use to compute the carrier-to-noise ratio?
        auto delta = 40.0 - constant::RAD2DEG * mCurrent->true_elevation;
        auto x     = delta * 0.05;
        auto value = x * x;
        carrier_to_noise_ratio -= value;
        VERBOSEF("cnr:          %+24.10f (%gdeg)", carrier_to_noise_ratio,
                 constant::RAD2DEG * mCurrent->true_elevation);
    }

    mCarrierToNoiseRatio = carrier_to_noise_ratio;

#ifdef DATA_TRACING
    datatrace::Observation dt_obs{};
    dt_obs.frequency                   = mFrequency * 1.0e3;
    dt_obs.geo_range                   = true_range0;
    dt_obs.eph_range                   = mCurrent->eph_range;
    dt_obs.eph_relativistic_correction = mCurrent->eph_relativistic_correction;
    dt_obs.orbit                       = true_range0 - mCurrent->eph_range;
    dt_obs.sat_clock                   = clock_bias0;
    dt_obs.phase_lock_time             = mLockTime.seconds;
    dt_obs.carrier_to_noise_ratio      = carrier_to_noise_ratio;
    dt_obs.eph_iod                     = mCurrent->eph_iod;

    dt_obs.orbit_radial_axis = mCurrent->orbit_radial_axis;
    dt_obs.orbit_cross_axis  = mCurrent->orbit_cross_axis;
    dt_obs.orbit_along_axis  = mCurrent->orbit_along_axis;
    dt_obs.orbit_delta_t     = mCurrent->orbit_delta_t;
#endif

    auto clock0 = 0.0;
    auto clock1 = 0.0;
    if (mClockCorrection.valid) {
        clock0 = mClockCorrection.correction;
        clock1 = mClockCorrection.correction;  // TODO(ewasjon): This is not correct
        VERBOSEF("clock:        %+24.10f (%gs,%gm/s)", clock0,
                 mClockCorrection.correction / constant::SPEED_OF_LIGHT,
                 (clock1 - clock0) / time_delta);
#ifdef DATA_TRACING
        dt_obs.clock = clock0;
#endif
    } else {
        VERBOSEF("clock:        ---");
    }

    auto code_bias = 0.0;
    if (mCodeBias.valid) {
        code_bias = mCodeBias.correction;
        VERBOSEF("code_bias:    %+24.10f (%gm)", code_bias, mCodeBias.correction);
#ifdef DATA_TRACING
        dt_obs.code_bias = code_bias;
#endif
    } else {
        VERBOSEF("code_bias:    ---");
    }

    auto phase_bias = 0.0;
    if (mPhaseBias.valid) {
        phase_bias = mPhaseBias.correction;
        VERBOSEF("phase_bias:   %+24.10f (%gm)", phase_bias, mPhaseBias.correction);
#ifdef DATA_TRACING
        dt_obs.phase_bias = phase_bias;
#endif
    } else {
        VERBOSEF("phase_bias:   ---");
    }

    auto stec_grid              = 0.0;
    auto stec_poly              = 0.0;
    auto stec_height_correction = 0.0;
#ifdef DATA_TRACING
    auto vtec_mapping = 0.0;
#endif
    if (mIonospheric.valid) {
        stec_grid              = 40.3e10 * mIonospheric.grid_residual / (mFrequency * mFrequency);
        stec_poly              = 40.3e10 * mIonospheric.poly_residual / (mFrequency * mFrequency);
        stec_height_correction = mIonospheric.height_correction;
#ifdef DATA_TRACING
        vtec_mapping = mIonospheric.vtec_mapping;
#endif
        VERBOSEF("stec_grid:    %+24.10f (%gTECU,%gkHz)", stec_grid, mIonospheric.grid_residual,
                 mFrequency);
        VERBOSEF("stec_poly:    %+24.10f (%gTECU,%gkHz)", stec_poly, mIonospheric.poly_residual,
                 mFrequency);
#ifdef DATA_TRACING
        dt_obs.stec_grid              = stec_grid;
        dt_obs.stec_poly              = stec_poly;
        dt_obs.vtec_mapping           = vtec_mapping;
        dt_obs.stec_height_correction = stec_height_correction;
#endif
    } else {
        VERBOSEF("stec_grid:    ---");
        VERBOSEF("stec_poly:    ---");
    }

    auto tropo_wet_height_correction = 1.0;
    auto tropo_dry_height_correction = 1.0;
    if (mTropospheric.valid_height_mapping) {
        tropo_dry_height_correction = mTropospheric.height_mapping_hydrostatic;
        tropo_wet_height_correction = mTropospheric.height_mapping_wet;
#ifdef DATA_TRACING
        dt_obs.tropo_dry_height_correction = tropo_dry_height_correction;
        dt_obs.tropo_wet_height_correction = tropo_wet_height_correction;
#endif
    }

    auto tropo_wet       = 0.0;
    auto tropo_dry       = 0.0;
    auto tropo_model_wet = 0.0;
    auto tropo_model_dry = 0.0;
    if (mTropospheric.valid) {
        tropo_dry = mTropospheric.hydrostatic * mTropospheric.mapping_hydrostatic *
                    tropo_dry_height_correction;
        tropo_wet = mTropospheric.wet * mTropospheric.mapping_wet * tropo_wet_height_correction;
        VERBOSEF("tropo_dry:    %+24.10f (%gm x %g x %g)", tropo_dry, mTropospheric.hydrostatic,
                 mTropospheric.mapping_hydrostatic, tropo_dry_height_correction);
        VERBOSEF("tropo_wet:    %+24.10f (%gm x %g x %g)", tropo_wet, mTropospheric.wet,
                 mTropospheric.mapping_wet, tropo_wet_height_correction);
#ifdef DATA_TRACING
        dt_obs.tropo_dry_mapping = mTropospheric.mapping_hydrostatic;
        dt_obs.tropo_wet_mapping = mTropospheric.mapping_wet;
        dt_obs.tropo_dry         = tropo_dry;
        dt_obs.tropo_wet         = tropo_wet;
#endif
    } else if (mUseTroposphericModel && mTropospheric.valid_model) {
        tropo_model_dry = mTropospheric.model_hydrostatic * mTropospheric.mapping_hydrostatic;
        tropo_model_wet = mTropospheric.model_wet * mTropospheric.mapping_wet;
        VERBOSEF("tropo_dry:    %+24.10f (%gm x %g)", tropo_dry, mTropospheric.model_hydrostatic,
                 mTropospheric.mapping_hydrostatic);
        VERBOSEF("tropo_wet:    %+24.10f (%gm x %g)", tropo_wet, mTropospheric.model_wet,
                 mTropospheric.mapping_wet);
#ifdef DATA_TRACING
        dt_obs.tropo_dry = tropo_model_dry;
        dt_obs.tropo_wet = tropo_model_wet;
#endif
    } else {
        VERBOSEF("tropo_dry:    ---");
        VERBOSEF("tropo_wet:    ---");
    }

    auto shapiro0 = 0.0;
    auto shapiro1 = 0.0;
    if (mCurrent->shapiro.valid) {
        shapiro0 = mCurrent->shapiro.correction;
        shapiro1 = mCurrent->shapiro.correction;
        if (mNext->shapiro.valid) {
            shapiro1 = mNext->shapiro.correction;
        }
        VERBOSEF("shapiro:      %+24.10f (%gm,%gm/s)", shapiro0, mCurrent->shapiro.correction,
                 (shapiro1 - shapiro0) / time_delta);
#ifdef DATA_TRACING
        dt_obs.shapiro = shapiro0;
#endif
    } else {
        VERBOSEF("shapiro:      ---");
    }

    auto earth_solid_tides0 = 0.0;
    auto earth_solid_tides1 = 0.0;
    if (mCurrent->earth_solid_tides.valid) {
        earth_solid_tides0 = mCurrent->earth_solid_tides.displacement;
        earth_solid_tides1 = mCurrent->earth_solid_tides.displacement;
        if (mNext->earth_solid_tides.valid) {
            earth_solid_tides1 = mNext->earth_solid_tides.displacement;
        }
        VERBOSEF("solid_tides:  %+24.10f (%gm,(%g,%g,%g),%gm/s)", earth_solid_tides0,
                 mCurrent->earth_solid_tides.displacement,
                 mCurrent->earth_solid_tides.displacement_vector.x,
                 mCurrent->earth_solid_tides.displacement_vector.y,
                 mCurrent->earth_solid_tides.displacement_vector.z,
                 (earth_solid_tides1 - earth_solid_tides0) / time_delta);
#ifdef DATA_TRACING
        dt_obs.earth_solid_tides = earth_solid_tides0;
#endif
    } else {
        VERBOSEF("solid_tides:  ---");
    }

    auto phase_windup0 = 0.0;
    auto phase_windup1 = 0.0;
    if (mCurrent->phase_windup.valid) {
        phase_windup0 = mCurrent->phase_windup.correction_velocity * mWavelength;
        phase_windup1 = mCurrent->phase_windup.correction_velocity * mWavelength;

        if (mNext->phase_windup.valid) {
            phase_windup1 = mNext->phase_windup.correction_velocity * mWavelength;
        }

        auto negative_factor = 1.0;
        if (mNegativePhaseWindup) {
            negative_factor = -1.0;
        }

        phase_windup0 *= negative_factor;
        phase_windup1 *= negative_factor;

        VERBOSEF("phase_windup: %+24.10f (%gc x %gm,%gm/s)", phase_windup0,
                 mCurrent->phase_windup.correction_velocity, mWavelength,
                 (phase_windup1 - phase_windup0) / time_delta);
#ifdef DATA_TRACING
        dt_obs.phase_windup = negative_factor * mCurrent->phase_windup.correction_sun * mWavelength;
        dt_obs.phase_windup_velocity =
            negative_factor * mCurrent->phase_windup.correction_velocity * mWavelength;
        dt_obs.phase_windup_angle =
            negative_factor * mCurrent->phase_windup.correction_angle * mWavelength;
#endif
    } else {
        VERBOSEF("phase_windup: ---");
    }

    auto antenna_phase_variation = 0.0;
    if (mAntennaPhaseVariation.valid) {
        antenna_phase_variation = mAntennaPhaseVariation.correction;
        VERBOSEF("ant_phase:    %+24.10f (%gm)", antenna_phase_variation,
                 mAntennaPhaseVariation.correction);
#ifdef DATA_TRACING
        dt_obs.antenna_phase_variation = antenna_phase_variation;
#endif
    } else {
        VERBOSEF("ant_phase:    ---");
    }

    if (is_valid()) {
        auto stec0 = stec_grid + stec_poly;
        auto stec1 = stec_grid + stec_poly;

        if (mUseIonosphericHeightCorrection) {
            stec0 *= stec_height_correction;
            stec1 *= stec_height_correction;
        }

        auto tropo0 = tropo_dry + tropo_wet + tropo_model_dry + tropo_model_wet;
        auto tropo1 = tropo_dry + tropo_wet + tropo_model_dry + tropo_model_wet;

        auto code_correction0 = clock0 + code_bias + stec0 + tropo0;
        auto code_correction1 = clock1 + code_bias + stec1 + tropo1;

        auto phase_correction0 = clock0 + phase_bias - stec0 + tropo0;
        auto phase_correction1 = clock1 + phase_bias - stec1 + tropo1;

        auto final_code_correction0 = shapiro0 + earth_solid_tides0;
        auto final_code_correction1 = shapiro1 + earth_solid_tides1;

        auto final_phase_correction0 =
            shapiro0 + earth_solid_tides0 + phase_windup0 + antenna_phase_variation;
        auto final_phase_correction1 =
            shapiro1 + earth_solid_tides1 + phase_windup1 + antenna_phase_variation;

        auto code_result0 = true_range0 + clock_bias0 + code_correction0 + final_code_correction0;
        auto code_result1 = true_range1 + clock_bias1 + code_correction1 + final_code_correction1;

        auto phase_result0 =
            true_range0 + clock_bias0 + phase_correction0 + final_phase_correction0;
        auto phase_result1 =
            true_range1 + clock_bias1 + phase_correction1 + final_phase_correction1;

        auto phase_delta = phase_result1 - phase_result0;
        auto phase_rate  = phase_delta / time_delta;

        DEBUGF("%s(%s): code  %+24.10f (%g,%g,%gm/s)", mSvId.name(), mSignalId.name(), code_result0,
               code_correction0, final_code_correction0,
               (code_result1 - code_result0) / time_delta);
        DEBUGF("%s(%s): phase %+24.10f (%g,%g)", mSvId.name(), mSignalId.name(), phase_result0,
               phase_correction0, final_phase_correction0);
        DEBUGF("%s(%s): phase rate %+19.10f (%g,%g)", mSvId.name(), mSignalId.name(), phase_rate,
               phase_delta, time_delta);

#ifdef DATA_TRACING
        dt_obs.code_range       = code_result0;
        dt_obs.phase_range      = phase_result0;
        dt_obs.phase_range_rate = phase_rate;
        dt_obs.stec             = stec0;
        dt_obs.tropo            = tropo0;
#endif

        mCodeRange      = code_result0;
        mPhaseRange     = phase_result0;
        mPhaseRangeRate = phase_rate;
    }

#ifdef DATA_TRACING
    datatrace::report_observation(mCurrent->reception_time, mSvId.name(), mSignalId.name(), dt_obs);
#endif
}

double Observation::code_range() const NOEXCEPT {
    return mCodeRange;
}

double Observation::phase_range() const NOEXCEPT {
    return mPhaseRange;
}

}  // namespace tokoro
}  // namespace generator
