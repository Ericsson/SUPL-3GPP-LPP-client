#pragma once
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct CorrectionData;
struct Satellite;
struct SatelliteLocation;

struct TroposphericDelay {
    double hydrostatic;
    double wet;
    double mapping_hydrostatic;
    double mapping_wet;
    double height_mapping_hydrostatic;
    double height_mapping_wet;
    bool   valid;
    bool   valid_height_mapping;
};

struct IonosphericDelay {
    double grid_residual;
    double poly_residual;
    bool   valid;
};

struct Correction {
    double correction;
    bool   valid;
};

struct SolidEarthTides {
    double displacement;
    Float3 displacement_vector;
    bool   valid;
};

struct Observation {
public:
    EXPLICIT Observation(Satellite const& satellite, SignalId signal_id, Float3 location) NOEXCEPT;

    void compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT;
    void compute_code_bias(CorrectionData const& correction_data) NOEXCEPT;
    void compute_tropospheric(CorrectionData const& correction_data) NOEXCEPT;
    void compute_ionospheric(CorrectionData const& correction_data) NOEXCEPT;

    void compute_tropospheric_height() NOEXCEPT;

    void compute_shapiro() NOEXCEPT;
    void compute_phase_windup() NOEXCEPT;
    void compute_earth_solid_tides() NOEXCEPT;
    void compute_antenna_phase_variation() NOEXCEPT;

    void compute_ranges() NOEXCEPT;

    NODISCARD bool is_valid() const NOEXCEPT { return mIsValid; }
    void           discard() NOEXCEPT { mIsValid = false; }

    NODISCARD double code_range() const NOEXCEPT;
    NODISCARD double phase_range() const NOEXCEPT;
    NODISCARD double phase_range_rate() const NOEXCEPT { return mPhaseRangeRate; }

    NODISCARD bool has_phase_bias() const NOEXCEPT { return mPhaseBias.valid; }
    NODISCARD bool has_code_bias() const NOEXCEPT { return mCodeBias.valid; }
    NODISCARD bool has_tropospheric() const NOEXCEPT { return mTropospheric.valid; }
    NODISCARD bool has_ionospheric() const NOEXCEPT { return mIonospheric.valid; }

    NODISCARD SatelliteId const& sv_id() const NOEXCEPT { return mSvId; }
    NODISCARD SignalId const&    signal_id() const NOEXCEPT { return mSignalId; }

private:
    SatelliteId mSvId;
    SignalId    mSignalId;
    bool        mIsValid;
    uint16_t    mIode;

    Float3 mGroundPosition;
    Float3 mGroundLlh;

    SatelliteLocation const& mCurrent;
    SatelliteLocation const& mNext;

    double mFrequency;
    double mWavelength;

    Correction        mClockCorrection;
    Correction        mCodeBias;
    Correction        mPhaseBias;
    TroposphericDelay mTropospheric;
    IonosphericDelay  mIonospheric;
    Correction        mShapiro;
    Correction        mPhaseWindup;
    SolidEarthTides   mEarthSolidTides;
    Correction        mAntennaPhaseVariation;

    double mPhaseRange;
    double mPhaseRangeRate;
    double mCodeRange;
};

}  // namespace tokoro
}  // namespace generator
