#pragma once
#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <generator/tokoro/ecef.hpp>
#include <generator/tokoro/wgs84.hpp>

#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct CorrectionData;
struct Satellite;

struct Observation {
public:
    EXPLICIT Observation(Satellite const& satellite, SignalId signal_id, Float3 location) NOEXCEPT;

    void compute_phase_bias(CorrectionData const& correction_data) NOEXCEPT;
    void compute_code_bias(CorrectionData const& correction_data) NOEXCEPT;
    void compute_tropospheric(EcefPosition          location,
                              CorrectionData const& correction_data) NOEXCEPT;
    void compute_ionospheric(EcefPosition location, CorrectionData const& correction_data) NOEXCEPT;

    void compute_shapiro() NOEXCEPT;
    void compute_phase_windup() NOEXCEPT;
    void compute_solid_tides() NOEXCEPT;

    NODISCARD double pseudorange() const NOEXCEPT;
    NODISCARD double carrier_cycle() const NOEXCEPT;

    NODISCARD SatelliteId const& sv_id() const NOEXCEPT { return mSvId; }
    NODISCARD SignalId const&    signal_id() const NOEXCEPT { return mSignalId; }

    NODISCARD bool has_phase_bias() const NOEXCEPT { return mPhaseBiasValid; }
    NODISCARD bool has_code_bias() const NOEXCEPT { return mCodeBiasValid; }
    NODISCARD bool has_tropospheric() const NOEXCEPT { return mTropoValid; }
    NODISCARD bool has_ionospheric() const NOEXCEPT { return mIonoValid; }

private:
    SatelliteId   mSvId;
    SignalId      mSignalId;
    ts::Tai       mEmissionTime;
    ts::Tai       mReceptionTime;
    Float3        mGroundPosition;
    Wgs84Position mWgsPosition;
    double        mElevation;

    Float3 mSatelliteApc;

    double mFrequency;
    double mWavelength;

    double mTrueRange;
    double mEphRange;

    double mEphOrbitError;
    double mEphClockBias;

    double mClockCorrection;
    bool   mClockCorrectionValid;

    double mCodeBias;
    bool   mCodeBiasValid;

    double mPhaseBias;
    bool   mPhaseBiasValid;

    double mTropoDry;
    double mTropoWet;
    double mTropoDryMapping;
    double mTropoWetMapping;
    bool   mTropoValid;

    double mIonoGridResidual;
    double mIonoPolyResidual;
    bool   mIonoValid;

    double mShapiro;
    bool   mShapiroValid;
    double mPhaseWindup;
    bool   mPhaseWindupValid;
    double mSolidTides;
    bool   mSolidTidesValid;
};

}  // namespace tokoro
}  // namespace generator
