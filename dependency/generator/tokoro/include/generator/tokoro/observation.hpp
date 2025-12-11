#pragma once
#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>
#include "models/earth_solid_tides.hpp"
#include "models/phase_windup.hpp"
#include "models/shapiro.hpp"

#ifdef INCLUDE_FORMAT_ANTEX
#include <format/antex/antex.hpp>
#endif
#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct CorrectionData;
struct Satellite;
struct SatelliteState;

struct TroposphericDelay {
    double hydrostatic;
    double wet;
    double mapping_hydrostatic;
    double mapping_wet;
    double height_mapping_hydrostatic;
    double height_mapping_wet;
    double model_hydrostatic;
    double model_wet;
    bool   valid;
    bool   valid_height_mapping;
    bool   valid_model;
};

struct IonosphericDelay {
    double grid_residual;
    double poly_residual;
    bool   valid;
    double vtec_mapping;
    double height_correction;
    double quality;
    bool   quality_valid;
};

struct Correction {
    double correction;
    bool   valid;
};

struct LockTime {
    ts::Tai time;
    double  seconds;
    bool    lost;
};

struct Observation {
public:
    EXPLICIT Observation(Satellite const& satellite, SignalId signal_id, Float3 location) NOEXCEPT;

    void update_lock_time(LockTime const& lock_time) NOEXCEPT;

    void compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT;
    void compute_code_bias(CorrectionData const& correction_data) NOEXCEPT;
    void compute_tropospheric(CorrectionData const& correction_data) NOEXCEPT;
    void compute_ionospheric(CorrectionData const& correction_data) NOEXCEPT;

    void compute_tropospheric_height() NOEXCEPT;

#ifdef INCLUDE_FORMAT_ANTEX
    void compute_antenna_phase_variation(format::antex::Antex const& antex) NOEXCEPT;
#endif

    void compute_ranges() NOEXCEPT;

    NODISCARD bool is_valid() const NOEXCEPT { return mIsValid; }
    void           discard() NOEXCEPT { mIsValid = false; }

    void set_negative_phase_windup(bool enabled) NOEXCEPT { mNegativePhaseWindup = enabled; }
    void set_require_code_bias(bool enabled) NOEXCEPT { mRequireCodeBias = enabled; }
    void set_require_phase_bias(bool enabled) NOEXCEPT { mRequirePhaseBias = enabled; }
    void set_require_tropo(bool enabled) NOEXCEPT { mRequireTropospheric = enabled; }
    void set_require_iono(bool enabled) NOEXCEPT { mRequireIonospheric = enabled; }
    void set_use_tropospheric_model(bool enabled) NOEXCEPT { mUseTroposphericModel = enabled; }
    void set_use_ionospheric_height_correction(bool enabled) NOEXCEPT {
        mUseIonosphericHeightCorrection = enabled;
    }

    NODISCARD double code_range() const NOEXCEPT;
    NODISCARD double phase_range() const NOEXCEPT;
    NODISCARD double phase_range_rate() const NOEXCEPT { return mPhaseRangeRate; }
    NODISCARD double wave_length() const NOEXCEPT { return mWavelength; }

    NODISCARD double carrier_to_noise_ratio() const NOEXCEPT { return mCarrierToNoiseRatio; }
    NODISCARD LockTime const& lock_time() const NOEXCEPT { return mLockTime; }

    NODISCARD bool has_phase_bias() const NOEXCEPT { return mPhaseBias.valid; }
    NODISCARD bool has_code_bias() const NOEXCEPT { return mCodeBias.valid; }
    NODISCARD bool has_tropospheric() const NOEXCEPT { return mTropospheric.valid; }
    NODISCARD bool has_ionospheric() const NOEXCEPT { return mIonospheric.valid; }

    NODISCARD SatelliteId const& sv_id() const NOEXCEPT { return mSvId; }
    NODISCARD SignalId const&    signal_id() const NOEXCEPT { return mSignalId; }
    NODISCARD SatelliteSignalId  ss_id() const NOEXCEPT { return {mSvId, mSignalId}; }

private:
    SatelliteId mSvId;
    SignalId    mSignalId;
    bool        mIsValid;

    Float3 mGroundPosition;
    Float3 mGroundLlh;

    SatelliteState const* mCurrent;
    SatelliteState const* mNext;

    double   mFrequency;
    double   mWavelength;
    double   mCarrierToNoiseRatio;
    LockTime mLockTime;
    bool     mNegativePhaseWindup;

    bool mRequireCodeBias;
    bool mRequirePhaseBias;
    bool mRequireTropospheric;
    bool mRequireIonospheric;
    bool mUseTroposphericModel;
    bool mUseIonosphericHeightCorrection;

    Correction        mClockCorrection;
    Correction        mCodeBias;
    Correction        mPhaseBias;
    TroposphericDelay mTropospheric;
    IonosphericDelay  mIonospheric;
    Correction        mAntennaPhaseVariation;

    double mPhaseRange;
    double mPhaseRangeRate;
    double mCodeRange;
};

}  // namespace tokoro
}  // namespace generator
