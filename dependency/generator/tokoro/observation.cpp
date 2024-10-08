#include "observation.hpp"
#include "data.hpp"
#include "helper.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

namespace generator {
namespace tokoro {

Observation::Observation(Satellite const& satellite, SignalId signal_id, Float3 location) NOEXCEPT
    : mSvId(satellite.id()),
      mSignalId(signal_id),
      mTrueRange(satellite.true_range()),
      mEphRange(satellite.eph_range()),
      mCodeBiasValid(false),
      mPhaseBiasValid(false),
      mTropoValid(false),
      mIonoValid(false) {
    // TODO(ewasjon): For GLONASS, the frequency depends on the channel number
    mFrequency  = signal_id.frequency();
    mWavelength = constant::SPEED_OF_LIGHT / mFrequency;

    mEphOrbitError = mEphRange - mTrueRange;

    // NOTE(ewasjon): The clock bias should be added to the satellite time to "correct" it, however,
    // we want to uncorrect the satellite time, so we subtract it.
    mEphClockBias = -satellite.eph_clock_bias();

    mClockCorrection      = satellite.clock_correction();
    mClockCorrectionValid = true;

    mEmissionTime  = satellite.emission_time();
    mReceptionTime = satellite.reception_time();
    mWgsPosition   = ecef_to_wgs84(location);
    mElevation     = satellite.elevation();

    auto mapping     = hydrostatic_mapping_function(mReceptionTime, mWgsPosition, mElevation);
    mTropoDryMapping = mapping.hydrostatic;
    mTropoWetMapping = mapping.wet;
}

void Observation::compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->phase_bias.find(mSignalId);
    if (it == signals->phase_bias.end()) return;

    mPhaseBias      = it->second.bias;
    mPhaseBiasValid = true;
}

void Observation::compute_code_bias(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    auto signals = correction_data.signal_corrections(mSvId);
    if (!signals) return;

    auto it = signals->code_bias.find(mSignalId);
    if (it == signals->code_bias.end()) return;

    mCodeBias      = it->second.bias;
    mCodeBiasValid = true;
}

void Observation::compute_tropospheric(EcefPosition          location,
                                       CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mSvId.name());

    VERBOSEF("position: (%f, %f, %f)", location.x, location.y, location.z);

    TroposphericCorrection correction{};
    if (!correction_data.tropospheric(mSvId, location, correction)) {
        VERBOSEF("tropospheric correction not found");
        return;
    }

    if (mTropoValid) {
        VERBOSEF("tropospheric correction already computed");
        return;
    }

    mTropoDry   = correction.dry;
    mTropoWet   = correction.wet;
    mTropoValid = true;
}

void Observation::compute_ionospheric(EcefPosition          location,
                                      CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());

    IonosphericCorrection correction{};
    if (!correction_data.ionospheric(mSvId, location, correction)) {
        VERBOSEF("ionospheric correction not found");
        return;
    }

    mIonoGridResidual = correction.grid_residual;
    mIonoPolyResidual = correction.polynomial_residual;
    mIonoValid        = true;
}

double Observation::pseudorange() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    VERBOSEF("true_range: %+24.10f", mTrueRange);

    auto clock_bias = constant::SPEED_OF_LIGHT * mEphClockBias;
    VERBOSEF("clock_bias: %+24.10f (%gs)", clock_bias, mEphClockBias);

    auto clock = 0.0;
    if (mClockCorrectionValid) {
        clock = mClockCorrection;
        VERBOSEF("clock:      %+24.10f (%gs)", clock, mClockCorrection / constant::SPEED_OF_LIGHT);
    } else {
        VERBOSEF("clock:      ---");
    }

    auto code_bias = 0.0;
    if (mCodeBiasValid) {
        code_bias = mCodeBias;
        VERBOSEF("code_bias:  %+24.10f (%gm)", code_bias, mCodeBias);
    } else {
        VERBOSEF("code_bias:  ---");
    }

    auto stec_grid = 0.0;
    auto stec_poly = 0.0;
    if (mIonoValid) {
        stec_grid = 40.3e10 * mIonoGridResidual / (mFrequency * mFrequency);
        stec_poly = 40.3e10 * mIonoPolyResidual / (mFrequency * mFrequency);
        VERBOSEF("stec_grid:  %+24.10f (%g TECU, %g kHz)", stec_grid, mIonoGridResidual, mFrequency);
        VERBOSEF("stec_poly:  %+24.10f (%g TECU, %g kHz)", stec_poly, mIonoPolyResidual, mFrequency);
    } else {
        VERBOSEF("stec_grid:  ---");
        VERBOSEF("stec_poly:  ---");
    }

    auto tropo_wet = 0.0;
    auto tropo_dry = 0.0;
    if (mTropoValid) {
        tropo_dry = mTropoDry * mTropoDryMapping;
        tropo_wet = mTropoWet * mTropoWetMapping;
        VERBOSEF("tropo_dry:  %+24.10f (%gm x %g)", tropo_dry, mTropoDry, mTropoDryMapping);
        VERBOSEF("tropo_wet:  %+24.10f (%gm x %g)", tropo_wet, mTropoWet, mTropoWetMapping);
    } else {
        VERBOSEF("tropo_dry:  ---");
        VERBOSEF("tropo_wet:  ---");
    }

    auto pseudo_correction = clock + code_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;
    auto result            = mTrueRange + clock_bias + pseudo_correction;
    DEBUGF("%s(%s): code  %+24.10f (%g)", mSvId.name(), mSignalId.name(), result, pseudo_correction);
    return result;
}

double Observation::carrier_cycle() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mSvId.name(), mSignalId.name());
    VERBOSEF("true_range: %+24.10f", mTrueRange);

    auto clock_bias = constant::SPEED_OF_LIGHT * mEphClockBias;
    VERBOSEF("clock_bias: %+24.10f (%gs)", clock_bias, mEphClockBias);

    auto clock = 0.0;
    if (mClockCorrectionValid) {
        clock = mClockCorrection;
        VERBOSEF("clock:      %+24.10f (%gs)", clock, mClockCorrection / constant::SPEED_OF_LIGHT);
    } else {
        VERBOSEF("clock:      ---");
    }

    auto phase_bias = 0.0;
    if (mPhaseBiasValid) {
        phase_bias = mPhaseBias;
        VERBOSEF("phase_bias: %+24.10f (%gm)", phase_bias, mPhaseBias);
    } else {
        VERBOSEF("phase_bias: ---");
    }

    auto stec_grid = 0.0;
    auto stec_poly = 0.0;
    if (mIonoValid) {
        stec_grid = -40.3e10 * mIonoGridResidual / (mFrequency * mFrequency);
        stec_poly = -40.3e10 * mIonoPolyResidual / (mFrequency * mFrequency);
        VERBOSEF("stec_grid:  %+24.10f (%g TECU, %g kHz)", stec_grid, mIonoGridResidual, mFrequency);
        VERBOSEF("stec_poly:  %+24.10f (%g TECU, %g kHz)", stec_poly, mIonoPolyResidual, mFrequency);
    } else {
        VERBOSEF("stec_grid:  ---");
        VERBOSEF("stec_poly:  ---");
    }

    auto tropo_wet = 0.0;
    auto tropo_dry = 0.0;
    if (mTropoValid) {
        tropo_dry = mTropoDry * mTropoDryMapping;
        tropo_wet = mTropoWet * mTropoWetMapping;
        VERBOSEF("tropo_dry:  %+24.10f (%gm x %g)", tropo_dry, mTropoDry, mTropoDryMapping);
        VERBOSEF("tropo_wet:  %+24.10f (%gm x %g)", tropo_wet, mTropoWet, mTropoWetMapping);
    } else {
        VERBOSEF("tropo_dry:  ---");
        VERBOSEF("tropo_wet:  ---");
    }

    auto carrier_correction = clock + phase_bias + stec_grid + stec_poly + tropo_dry + tropo_wet;
    auto result             = mTrueRange + clock_bias + carrier_correction;
    DEBUGF("%s(%s): phase %+24.10f (%g)", mSvId.name(), mSignalId.name(), result, carrier_correction);
    return result;
}

}  // namespace tokoro
}  // namespace generator
