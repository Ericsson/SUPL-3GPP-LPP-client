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
#include "models/troposphere_models.hpp"
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

    mFrequency  = satellite.has_frequency_channel() ?
                      signal_id.frequency(satellite.frequency_channel()) :
                      signal_id.frequency();
    mWavelength = constant::SPEED_OF_LIGHT / mFrequency / 1000.0;

    mCarrierToNoiseRatio = 0.0;
    mLockTime            = {
        mCurrent->reception_time,
        0.0,
        true,
    };
    mNegativePhaseWindup           = false;
    mNegativeCorrectionSign        = false;
    mSsrGenerationConsistencyCheck = false;
    mOrbitClockSsrIod              = satellite.orbit_correction().ssr_iod;
    mClockSsrIod                   = satellite.clock_correction_data().ssr_iod;

    mRequirePhaseBias               = true;
    mRequireCodeBias                = true;
    mRequireTropospheric            = true;
    mRequireIonospheric             = true;
    mUseTroposphericModel           = false;
    mUseIonosphericHeightCorrection = false;

    mClockCorrection = Correction{satellite.clock_correction(), true};
    mCodeBias        = Correction{0.0, false};
    mPhaseBias       = Correction{0.0, false};
    mTropospheric =
        TroposphericDelay{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, false, false, false, true};
    mIonospheric           = IonosphericDelay{0.0, 0.0, false, 0.0, 0.0, 0.0, false, false};
    mAntennaPhaseVariation = Correction{0.0, false};

    mGroundPosition = location;
    mGroundLlh      = ecef_to_llh(location, ellipsoid::gWgs84);

    auto mapping = hydrostatic_mapping_function(mCurrent->reception_time, mGroundLlh,
                                                mCurrent->true_elevation, true);
    mTropospheric.mapping_hydrostatic = mapping.hydrostatic;
    mTropospheric.mapping_wet         = mapping.wet;
}

void Observation::compute_tropospheric_mapping() NOEXCEPT {
    VSCOPE_FUNCTION();

    // the geoid height is only needed by the zenith delay models
    TroposphereSite site{mCurrent->reception_time, mGroundLlh, 0.0};

    MappingValues values{};
    if (!mapping_values(mMappingFunction, mTroposphereData, site, mCurrent->true_elevation,
                        mTropoHydrostaticDelta, values)) {
        WARNF("failed to compute the %s mapping function (missing or out of range data)",
              mapping_function_name(mMappingFunction));
        mTropospheric.mapping_valid = false;
        return;
    }

    mTropospheric.mapping_hydrostatic = values.hydrostatic;
    mTropospheric.mapping_wet         = values.wet;
    mTropospheric.mapping_valid       = true;
}

void Observation::compute_tropospheric_model() NOEXCEPT {
    VSCOPE_FUNCTION();

    mTropospheric.valid_model = false;
    if (!mUseTroposphericModel) return;

    auto geoid_height = Geoid::height(mGroundLlh.x, mGroundLlh.y, Geoid::Model::EMBEDDED);

    TroposphereSite site{mCurrent->reception_time, mGroundLlh, geoid_height};
    ZenithDelays    delays{};
    if (!zenith_delays(mTroposphereModel, mTroposphereData, site, delays)) {
        WARNF("failed to compute the %s tropospheric model (missing or out of range data)",
              troposphere_model_name(mTroposphereModel));
        return;
    }

    mTropospheric.model_hydrostatic = delays.hydrostatic;
    mTropospheric.model_wet         = delays.wet;
    mTropospheric.valid_model       = true;
}

void Observation::compute_tropospheric_height() NOEXCEPT {
    VSCOPE_FUNCTION();

    auto geoid_height = Geoid::height(mGroundLlh.x, mGroundLlh.y, Geoid::Model::EMBEDDED);

    TroposphereSite site{mCurrent->reception_time, mGroundLlh, geoid_height};
    double          hydrostatic = 0.0;
    double          wet         = 0.0;
    if (height_scaling(site, hydrostatic, wet)) {
        mTropospheric.height_mapping_hydrostatic = hydrostatic;
        mTropospheric.height_mapping_wet         = wet;
        mTropospheric.valid_height_mapping       = true;
    } else {
        WARNF("failed to compute tropospheric height correction");
        mTropospheric.valid_height_mapping = false;
    }
}

void Observation::update_lock_time(LockTime const& lock_time) NOEXCEPT {
    mLockTime = lock_time;
}

void Observation::compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    DataStatus status;
    auto       bias = correction_data.phase_bias(mSvId, mSignalId, &status);
    if (!bias) {
        mPhaseBiasDetail = status.text();
        return;
    }

    auto it               = bias;
    mPhaseBiasSsrIod      = it->ssr_iod;
    mPhaseBiasSsrIodValid = true;
    if (mSsrGenerationConsistencyCheck && it->ssr_iod != mOrbitClockSsrIod) {
        VERBOSEF("phase bias belongs to a different SSR generation than the orbit/clock "
                 "correction [sv=%s,signal=%s,orbit_iod=%u,bias_iod=%u]",
                 mSvId.name(), mSignalId.name(), mOrbitClockSsrIod, it->ssr_iod);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "ssr_iod_mismatch(bias=%u,orbit=%u)",
                 static_cast<unsigned>(it->ssr_iod), static_cast<unsigned>(mOrbitClockSsrIod));
        mPhaseBiasDetail = buffer;
        return;
    }
    mPhaseBias                = Correction{it->bias, true};
    mPhaseBiasEpoch           = it->epoch_time;
    mPhaseDiscontinuity       = it->discontinuity;
    mHasPhaseIntegerIndicator = it->integer_indicator_valid;
    mPhaseIntegerIndicator    = it->integer_indicator;
}

void Observation::compute_code_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    DataStatus status;
    auto       it = correction_data.code_bias(mSvId, mSignalId, &status);
    if (!it) {
        mCodeBiasDetail = status.text();
        return;
    }
    mCodeBiasSsrIod      = it->ssr_iod;
    mCodeBiasSsrIodValid = true;
    if (mSsrGenerationConsistencyCheck && it->ssr_iod != mOrbitClockSsrIod) {
        VERBOSEF("code bias belongs to a different SSR generation than the orbit/clock "
                 "correction [sv=%s,signal=%s,orbit_iod=%u,bias_iod=%u]",
                 mSvId.name(), mSignalId.name(), mOrbitClockSsrIod, it->ssr_iod);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "ssr_iod_mismatch(bias=%u,orbit=%u)",
                 static_cast<unsigned>(it->ssr_iod), static_cast<unsigned>(mOrbitClockSsrIod));
        mCodeBiasDetail = buffer;
        return;
    }
    mCodeBias      = Correction{it->bias, true};
    mCodeBiasEpoch = it->epoch_time;
}

void Observation::compute_tropospheric(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mSvId.name());

    TroposphericCorrection correction{};
    DataStatus             status;
    if (!correction_data.tropospheric(mSvId, mGroundLlh, correction, &status)) {
        VERBOSEF("tropospheric correction not found");
        mTropoDetail = status.text();
        return;
    }

    if (mTropospheric.valid) {
        VERBOSEF("tropospheric correction already computed");
        return;
    }

    mTropospheric.hydrostatic = correction.dry;
    mTropospheric.wet         = correction.wet;
    mTropospheric.valid       = true;

    mGradient.ssr_valid     = correction.has_gradient;
    mGradient.ssr_dry_north = correction.dry_north;
    mGradient.ssr_dry_east  = correction.dry_east;
    mGradient.ssr_wet_north = correction.wet_north;
    mGradient.ssr_wet_east  = correction.wet_east;
}

void Observation::compute_tropospheric_gradient() NOEXCEPT {
    VSCOPE_FUNCTION();

    mGradient.valid   = false;
    mGradient.north   = 0.0;
    mGradient.east    = 0.0;
    mGradient.mapping = 0.0;
    mGradient.delay   = 0.0;

    double north = 0.0;
    double east  = 0.0;
    switch (mGradientSource) {
    case GradientSource::None: return;
    case GradientSource::SsrGrid: {
        if (!mTropospheric.valid || !mGradient.ssr_valid) return;

        // the zenith delays are scaled to the height of the station, so is their slope
        auto hydrostatic_scale = 1.0;
        auto wet_scale         = 1.0;
        if (mTropospheric.valid_height_mapping) {
            hydrostatic_scale = mTropospheric.height_mapping_hydrostatic;
            wet_scale         = mTropospheric.height_mapping_wet;
        }

        TroposphericCorrection slope{};
        slope.has_gradient = true;
        slope.dry_north    = mGradient.ssr_dry_north;
        slope.dry_east     = mGradient.ssr_dry_east;
        slope.wet_north    = mGradient.ssr_wet_north;
        slope.wet_east     = mGradient.ssr_wet_east;
        if (!ssr_grid_gradient(slope, hydrostatic_scale, wet_scale, north, east)) return;
        break;
    }
    case GradientSource::Constant:
        if (!mGradientConstantSet) return;
        north = mGradientConstantNorth;
        east  = mGradientConstantEast;
        break;
    }

    mGradient.north   = north;
    mGradient.east    = east;
    mGradient.mapping = gradient_mapping_value(mGradientMapping, mCurrent->true_elevation,
                                               mTropospheric.mapping_hydrostatic);
    mGradient.delay   = gradient_delay(mGradient.mapping, mCurrent->true_azimuth, north, east);
    mGradient.valid   = true;
    VERBOSEF("gradient: G_N %+.6f G_E %+.6f m_g %+.6f az %.2f -> %+.6f m", north, east,
             mGradient.mapping, mCurrent->true_azimuth * constant::RAD2DEG, mGradient.delay);
}

static double compute_vtec_mapping(double altitude, double elevation) {
    auto r          = ellipsoid::gWgs84.semi_major_axis + altitude;
    auto h          = 506.7e3;
    auto alpha      = 0.9782;
    auto sin        = std::sin(alpha * (constant::PI / 2.0 - elevation));
    auto height     = r / (r + h);
    auto cos_zenith = std::sqrt(1 - height * height * sin * sin);
    auto mapping    = 1.0 / cos_zenith;
    return mapping;
}

void Observation::compute_ionospheric(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    IonosphericCorrection correction{};
    DataStatus            status;
    if (!correction_data.ionospheric(mSvId, mGroundLlh, correction, &status)) {
        VERBOSEF("ionospheric correction not found");
        mIonoDetail = status.text();
        return;
    }

    mIonospheric.grid_residual = correction.grid_residual;
    mIonospheric.poly_residual = correction.polynomial_residual;
    mIonospheric.grid_valid    = correction.grid_valid;
    mIonospheric.valid         = true;

    mIonospheric.quality       = correction.quality;
    mIonospheric.quality_valid = correction.quality_valid;
    mIonospheric.vtec_mapping  = compute_vtec_mapping(0, mCurrent->true_elevation);

    auto ellipsoidal_height = mGroundLlh.z;
    auto vtec_mf_0          = compute_vtec_mapping(0, mCurrent->true_elevation);
    auto vtec_mf_alt        = compute_vtec_mapping(ellipsoidal_height, mCurrent->true_elevation);
    mIonospheric.height_correction = vtec_mf_0 / vtec_mf_alt;
}

#ifdef INCLUDE_FORMAT_ANTEX
void Observation::compute_antenna_phase_variation(format::antex::Antex const& antex) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    mAntennaPhaseVariation.valid = false;
    mAntennaPcoRange             = 0.0;  // PCO intentionally not applied (PCV only)

    // Phase-variation pattern (nadir/azimuth dependent), phase only.
    format::antex::PhaseVariation pv{};
    if (!antex.phase_variation(mSvId, mSignalId, mCurrent->reception_time, mCurrent->true_azimuth,
                               mCurrent->true_nadir, pv)) {
        VERBOSEF("antenna phase variation not found");
        return;
    }

    mAntennaPhaseVariation.correction = pv.value;
    mAntennaPhaseVariation.valid      = true;
}
#endif

void Observation::compute_ranges() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    auto time_delta =
        (mNext->reception_time.timestamp() - mCurrent->reception_time.timestamp()).full_seconds();

    // Discard this observation if it is missing important corrections, however, we still want to
    // compute the ranges to report via DATA TRACING.
    std::string discard_reason;
    std::string discard_detail;
    auto        add_reason = [&](char const* reason, std::string const& detail) {
        if (!discard_reason.empty()) discard_reason += '+';
        discard_reason += reason;
        if (!discard_detail.empty()) discard_detail += ';';
        discard_detail += reason;
        discard_detail += ':';
        discard_detail += detail.empty() ? "unknown" : detail;
    };
    if (mRequireCodeBias && !has_code_bias()) {
        WARNF("discarded: %s %s - no code bias", mSvId.name(), mSignalId.name());
        add_reason("no_code_bias", mCodeBiasDetail);
        discard();
    }
    if (mRequirePhaseBias && !has_phase_bias()) {
        WARNF("discarded: %s %s - no phase bias", mSvId.name(), mSignalId.name());
        add_reason("no_phase_bias", mPhaseBiasDetail);
        discard();
    }

    if (mRequireTropospheric && !has_tropospheric()) {
        WARNF("discarded: %s %s - no tropospheric correction", mSvId.name(), mSignalId.name());
        add_reason("no_tropospheric", mTropoDetail);
        discard();
    }
    if (!mTropospheric.mapping_valid &&
        (mTropospheric.valid || (mUseTroposphericModel && mTropospheric.valid_model))) {
        WARNF("discarded: %s %s - no tropospheric mapping function", mSvId.name(),
              mSignalId.name());
        add_reason("no_tropospheric_mapping",
                   std::string("mapping=") + mapping_function_name(mMappingFunction));
        discard();
    }
    if (mRequireIonospheric && !has_ionospheric()) {
        WARNF("discarded: %s %s - no ionospheric correction", mSvId.name(), mSignalId.name());
        add_reason("no_ionospheric", mIonoDetail);
        discard();
    }
    mDiscardReason = discard_reason;
    mDiscardDetail = discard_detail;

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
        clock0 = mCurrent->clock_correction;
        clock1 = mNext->clock_correction;
        if (mNegativeCorrectionSign) {
            clock0 = -clock0;
            clock1 = -clock1;
        }
        VERBOSEF("clock:        %+24.10f (%gs,%gm/s)", clock0,
                 mCurrent->clock_correction / constant::SPEED_OF_LIGHT,
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
        if (mNegativeCorrectionSign) code_bias = -code_bias;
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
        if (mNegativeCorrectionSign) phase_bias = -phase_bias;
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

    // The gradient term is only applied together with a tropospheric delay
    auto tropo_gradient = 0.0;
    if (mGradient.valid && mTropospheric.mapping_valid &&
        (mTropospheric.valid || (mUseTroposphericModel && mTropospheric.valid_model))) {
        tropo_gradient = mGradient.delay;
        VERBOSEF("tropo_grad:   %+24.10f", tropo_gradient);
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

    auto phase_windup0         = 0.0;
    auto phase_windup1         = 0.0;
    auto phase_windup_sun      = 0.0;
    auto phase_windup_velocity = 0.0;
    auto phase_windup_yaw      = 0.0;
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

        phase_windup_sun = negative_factor * mCurrent->phase_windup.correction_sun * mWavelength;
        phase_windup_velocity =
            negative_factor * mCurrent->phase_windup.correction_velocity * mWavelength;
        phase_windup_yaw = negative_factor * mCurrent->phase_windup.correction_angle * mWavelength;

        VERBOSEF("phase_windup: %+24.10f (%gc x %gm,%gm/s)", phase_windup0,
                 mCurrent->phase_windup.correction_velocity, mWavelength,
                 (phase_windup1 - phase_windup0) / time_delta);
#ifdef DATA_TRACING
        dt_obs.phase_windup          = phase_windup_sun;
        dt_obs.phase_windup_velocity = phase_windup_velocity;
        dt_obs.phase_windup_angle    = phase_windup_yaw;
#endif
    } else {
        VERBOSEF("phase_windup: ---");
    }

    auto antenna_pco = 0.0;  // geometric PCO offset on LOS: code + phase
    auto antenna_pcv = 0.0;  // phase-variation pattern: phase only
    if (mAntennaPhaseVariation.valid) {
        antenna_pco = mAntennaPcoRange;
        antenna_pcv = mAntennaPhaseVariation.correction;
        VERBOSEF("ant_pco:      %+24.10f (%gm)", antenna_pco, antenna_pco);
        VERBOSEF("ant_pcv:      %+24.10f (%gm)", antenna_pcv, antenna_pcv);
#ifdef DATA_TRACING
        dt_obs.antenna_phase_variation = antenna_pco + antenna_pcv;
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

        auto tropo0 = tropo_dry + tropo_wet + tropo_model_dry + tropo_model_wet + tropo_gradient;
        auto tropo1 = tropo_dry + tropo_wet + tropo_model_dry + tropo_model_wet + tropo_gradient;

        auto code_correction0 = clock0 + code_bias + stec0 + tropo0;
        auto code_correction1 = clock1 + code_bias + stec1 + tropo1;

        auto phase_correction0 = clock0 + phase_bias - stec0 + tropo0;
        auto phase_correction1 = clock1 + phase_bias - stec1 + tropo1;

        auto final_code_correction0 = shapiro0 + earth_solid_tides0 + antenna_pco;
        auto final_code_correction1 = shapiro1 + earth_solid_tides1 + antenna_pco;

        auto final_phase_correction0 =
            shapiro0 + earth_solid_tides0 + phase_windup0 + antenna_pco + antenna_pcv;
        auto final_phase_correction1 =
            shapiro1 + earth_solid_tides1 + phase_windup1 + antenna_pco + antenna_pcv;

        auto code_result0 = true_range0 + clock_bias0 + code_correction0 + final_code_correction0;
        auto code_result1 = true_range1 + clock_bias1 + code_correction1 + final_code_correction1;

        auto ambiguity_range = static_cast<double>(mPhaseAmbiguity) * mWavelength;
        auto phase_result0   = true_range0 + clock_bias0 + phase_correction0 +
                             final_phase_correction0 + ambiguity_range;
        auto phase_result1 = true_range1 + clock_bias1 + phase_correction1 +
                             final_phase_correction1 + ambiguity_range;

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

    // The SSR phase discontinuity indicator is incremented by the provider whenever the phase
    // bias of this signal has a discontinuity, i.e. a cycle slip.
    if (mPhaseBias.valid && mHasPreviousPhaseDiscontinuity) {
        mCycleSlipChecked = true;
        if (mPhaseDiscontinuity != mPreviousPhaseDiscontinuity) {
            mCycleSlip = true;
            WARNF("cycle slip: %s %s phase discontinuity indicator %u -> %u", mSvId.name(),
                  mSignalId.name(), static_cast<unsigned>(mPreviousPhaseDiscontinuity),
                  static_cast<unsigned>(mPhaseDiscontinuity));
        }
    }

    if (mDiagFile) {
        auto    msm_id = mSignalId.as_msm();
        DiagRow row{};
        row.prn    = mSvId.lpp_id().valid ? static_cast<int>(mSvId.lpp_id().value) + 1 : 0;
        row.sig_id = msm_id.valid ? static_cast<int>(msm_id.value) : 0;
        row.has_orbit_ssr_iod      = true;
        row.orbit_ssr_iod          = mOrbitClockSsrIod;
        row.has_clock_ssr_iod      = true;
        row.clock_ssr_iod          = mClockSsrIod;
        row.has_code_bias_ssr_iod  = mCodeBiasSsrIodValid;
        row.code_bias_ssr_iod      = mCodeBiasSsrIod;
        row.has_phase_bias_ssr_iod = mPhaseBiasSsrIodValid;
        row.phase_bias_ssr_iod     = mPhaseBiasSsrIod;
        row.clock                  = clock0;
        row.code_bias              = code_bias;
        row.phase_bias             = phase_bias;
        row.stec_grid              = stec_grid;
        row.stec_poly              = stec_poly;
        row.stec_height_correction = stec_height_correction;
        row.tropo_dry              = tropo_dry + tropo_model_dry;
        row.tropo_wet              = tropo_wet + tropo_model_wet;
        row.tropo_total =
            tropo_dry + tropo_wet + tropo_model_dry + tropo_model_wet + tropo_gradient;
        row.tropo_dry_mapping = mTropospheric.mapping_hydrostatic;
        row.tropo_wet_mapping = mTropospheric.mapping_wet;
        row.tropo_dry_zenith =
            mTropospheric.valid ?
                mTropospheric.hydrostatic :
                (mTropospheric.valid_model ? mTropospheric.model_hydrostatic : 0.0);
        row.tropo_wet_zenith      = mTropospheric.valid ?
                                        mTropospheric.wet :
                                        (mTropospheric.valid_model ? mTropospheric.model_wet : 0.0);
        row.phase_windup          = phase_windup0;
        row.phase_windup_sun      = phase_windup_sun;
        row.phase_windup_velocity = phase_windup_velocity;
        row.phase_windup_yaw      = phase_windup_yaw;
        row.ant_phase             = antenna_pco + antenna_pcv;
        row.cnr                   = mCarrierToNoiseRatio;
        row.lock_time             = mLockTime.seconds;
        row.frequency_mhz         = mFrequency * 1.0e-3;  // kHz → MHz
        if (mCodeBias.valid) row.code_bias_epoch.set(mCodeBiasEpoch, mCurrent->reception_time);
        if (mPhaseBias.valid) row.phase_bias_epoch.set(mPhaseBiasEpoch, mCurrent->reception_time);
        row.has_phase_discontinuity = mPhaseBias.valid;
        row.phase_discontinuity     = mPhaseDiscontinuity;
        // TODO(ewasjon): report the real per satellite/signal integer cycles once integer support
        // is added (row.integer_cycles defaults to 0).
        row.has_phase_integer = mPhaseBias.valid && mHasPhaseIntegerIndicator;
        row.phase_integer     = mPhaseIntegerIndicator;
        row.has_cycle_slip    = mCycleSlipChecked;
        row.cycle_slip        = mCycleSlip;
        row.has_clock         = mClockCorrection.valid;
        row.has_code_bias     = mCodeBias.valid;
        row.has_phase_bias    = mPhaseBias.valid;
        row.has_ionospheric   = mIonospheric.valid;
        row.has_stec_grid     = mIonospheric.valid && mIonospheric.grid_valid;
        row.has_tropospheric =
            mTropospheric.valid || (mUseTroposphericModel && mTropospheric.valid_model);
        row.has_phase_windup = mCurrent->phase_windup.valid;
        row.has_ant_phase    = mAntennaPhaseVariation.valid;
        if (is_valid()) {
            auto stec0 = stec_grid + stec_poly;
            if (mUseIonosphericHeightCorrection) stec0 *= stec_height_correction;
            row.stec_total = stec0;
            auto tropo0    = row.tropo_total;
            row.code_correction =
                clock0 + code_bias + stec0 + tropo0 + shapiro0 + earth_solid_tides0 + antenna_pco;
            row.phase_correction = clock0 + phase_bias - stec0 + tropo0 + shapiro0 +
                                   earth_solid_tides0 + phase_windup0 + antenna_pco + antenna_pcv;
            row.code_range  = mCodeRange;
            row.phase_range = mPhaseRange;
            row.phase_rate  = mPhaseRangeRate;
            if (mDiagDiscardReason) {
                row.valid          = false;
                row.discard_reason = mDiagDiscardReason;
            } else {
                row.valid = true;
            }
        } else {
            row.valid          = false;
            row.discard_reason = mDiagDiscardReason ? mDiagDiscardReason : discard_reason;
        }
        // the detail also has the reasons that the reason column does not show when the signal is
        // masked by the station
        if (mDiagDiscardReason) {
            row.discard_detail = std::string(mDiagDiscardReason) + ":" +
                                 (mDiagDiscardDetail.empty() ? "unknown" : mDiagDiscardDetail);
            if (!discard_detail.empty()) row.discard_detail += ";" + discard_detail;
        } else {
            row.discard_detail = discard_detail;
        }
        mDiagFile->write(mCurrent->reception_time, row);
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
