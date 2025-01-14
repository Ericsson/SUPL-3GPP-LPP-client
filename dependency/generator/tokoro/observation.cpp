#include "observation.hpp"
#include "coordinate.hpp"
#include "coordinates/eci.hpp"
#include "coordinates/enu.hpp"
#include "data.hpp"
#include "models/astronomical_arguments.hpp"
#include "models/geoid.hpp"
#include "models/helper.hpp"
#include "models/mops.hpp"
#include "models/nutation.hpp"
#include "models/phase_windup.hpp"
#include "models/shapiro.hpp"
#include "models/sun_moon.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>
#include <maths/mat3.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#define LOGLET_CURRENT_MODULE "tokoro"

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

    mCarrierToNoiseRatio = 47.0;   // TODO: How do we choose this value?
    mLockTime            = 525.0;  // TODO: How do we determine this value?
    mNegativePhaseWindup = false;

    mClockCorrection       = Correction{satellite.clock_correction(), true};
    mCodeBias              = Correction{0.0, false};
    mPhaseBias             = Correction{0.0, false};
    mTropospheric          = TroposphericDelay{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, false};
    mIonospheric           = IonosphericDelay{0.0, 0.0, false};
    mAntennaPhaseVariation = Correction{0.0, false};

    mGroundPosition = location;
    mGroundLlh      = ecef_to_llh(location, ellipsoid::WGS84);

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
}

void Observation::compute_antenna_phase_variation() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    mAntennaPhaseVariation.valid = false;
}

void Observation::compute_ranges() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    auto time_delta =
        (mNext->reception_time.timestamp() - mCurrent->reception_time.timestamp()).full_seconds();

    // Discard this observation if it is missing important corrections, however, we still want to
    // compute the ranges to report via DATA TRACING.
    if (!has_phase_bias()) {
        WARNF("discarded: %s %s - no phase bias", mSvId.name(), mSignalId.name());
        discard();
    }
    if (!has_code_bias()) {
        WARNF("discarded: %s %s - no code bias", mSvId.name(), mSignalId.name());
        discard();
    }
    if (!has_tropospheric()) {
        WARNF("discarded: %s %s - no tropospheric correction", mSvId.name(), mSignalId.name());
        discard();
    }
    if (!has_ionospheric()) {
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

#ifdef DATA_TRACING
    datatrace::Observation dt_obs{};
    dt_obs.frequency                   = mFrequency * 1.0e3;
    dt_obs.geo_range                   = true_range0;
    dt_obs.eph_range                   = mCurrent->eph_range;
    dt_obs.eph_relativistic_correction = mCurrent->eph_relativistic_correction;
    dt_obs.orbit                       = true_range0 - mCurrent->eph_range;
    dt_obs.sat_clock                   = clock_bias0;
    dt_obs.phase_lock_time             = mLockTime;
    dt_obs.carrier_to_noise_ratio      = mCarrierToNoiseRatio;

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

    auto stec_grid = 0.0;
    auto stec_poly = 0.0;
    if (mIonospheric.valid) {
        stec_grid = 40.3e10 * mIonospheric.grid_residual / (mFrequency * mFrequency);
        stec_poly = 40.3e10 * mIonospheric.poly_residual / (mFrequency * mFrequency);
        VERBOSEF("stec_grid:    %+24.10f (%gTECU,%gkHz)", stec_grid, mIonospheric.grid_residual,
                 mFrequency);
        VERBOSEF("stec_poly:    %+24.10f (%gTECU,%gkHz)", stec_poly, mIonospheric.poly_residual,
                 mFrequency);
#ifdef DATA_TRACING
        dt_obs.stec_grid = stec_grid;
        dt_obs.stec_poly = stec_poly;
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

    auto tropo_wet = 0.0;
    auto tropo_dry = 0.0;
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
        if(mNegativePhaseWindup) {
            negative_factor = -1.0;
        }

        phase_windup0 *= negative_factor;
        phase_windup1 *= negative_factor;

        VERBOSEF("phase_windup: %+24.10f (%gc x %gm,%gm/s)", phase_windup0,
                 mCurrent->phase_windup.correction_velocity, mWavelength,
                 (phase_windup1 - phase_windup0) / time_delta);
#ifdef DATA_TRACING
        dt_obs.phase_windup          = negative_factor * mCurrent->phase_windup.correction_sun * mWavelength;
        dt_obs.phase_windup_velocity = negative_factor * mCurrent->phase_windup.correction_velocity * mWavelength;
        dt_obs.phase_windup_angle    = negative_factor * mCurrent->phase_windup.correction_angle * mWavelength;
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
        auto code_correction0 = clock0 + code_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;
        auto code_correction1 = clock1 + code_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;

        auto phase_correction0 =
            clock0 + phase_bias - stec_grid - stec_poly + tropo_dry + tropo_wet;
        auto phase_correction1 =
            clock1 + phase_bias - stec_grid - stec_poly + tropo_dry + tropo_wet;

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
#endif

#if 0
    printf("TRACK-OBS,%s,%s,%g,%u,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,%.14f,TOTAL,%.14f,%.14f,"
           "TOTAL,%.14f,%."
           "14f,%.14f,%.14f",
           mSvId.name(), mSignalId.name(), mFrequency / 1.0e6, mCurrent->eph_iod,
           mCurrent->true_range, clock_bias, clock, code_bias, phase_bias, stec_grid, stec_poly,
           tropo_dry, tropo_wet, shapiro, solid_tides, phase_windup, antenna_phase_variation);
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
