#pragma once
#include "constant.hpp"
#include "data.hpp"
#include "observation.hpp"
#include "sv_id.hpp"

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <maths/float3.hpp>
#include <time/gps.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

class Generator;
struct Satellite {
public:
    EXPLICIT Satellite(SatelliteId id, Float3 ground_position, Generator const& generator) NOEXCEPT;

    void update(ts::Tai const& generation_time) NOEXCEPT;

    NODISCARD bool compute_true_position(ephemeris::Ephemeris const& eph) NOEXCEPT;
    NODISCARD bool compute_azimuth_and_elevation() NOEXCEPT;

    NODISCARD bool find_orbit_correction(CorrectionData const& correction_data) NOEXCEPT;
    NODISCARD bool find_clock_correction(CorrectionData const& correction_data) NOEXCEPT;

    NODISCARD const SatelliteId& id() const NOEXCEPT { return mId; }

    NODISCARD ts::Tai reception_time() const NOEXCEPT { return mReceptionTime; }
    NODISCARD ts::Tai emission_time() const NOEXCEPT { return mEmissionTime; }

    NODISCARD double true_range() const NOEXCEPT { return mTrueRange; }
    NODISCARD double pseudorange() const NOEXCEPT;
    NODISCARD double elevation() const NOEXCEPT { return mTrueElevation; }
    NODISCARD Float3 apc() const NOEXCEPT { return mTruePosition; }
    NODISCARD double eph_range() const NOEXCEPT { return mEphRange; }
    NODISCARD double eph_clock_bias() const NOEXCEPT { return mEphClockBias; }
    NODISCARD Float3 line_of_sight() const NOEXCEPT { return mTrueLineOfSight; }
    NODISCARD double clock_correction() const NOEXCEPT;

    NODISCARD bool enabled() const NOEXCEPT { return mEnabled; }

    void disable() NOEXCEPT { mEnabled = false; }

    NODISCARD double average_code_range() const NOEXCEPT;

    void                            reset_observations() NOEXCEPT { mObservations.clear(); }
    std::vector<Observation> const& observations() const NOEXCEPT { return mObservations; }

    Observation& initialize_observation(SignalId signal_id) NOEXCEPT {
        mObservations.emplace_back(*this, signal_id, mGroundPosition);
        return mObservations.back();
    }

private:
    SatelliteId mId;
    Float3      mGroundPosition;
    bool        mEnabled;

    ts::Tai mLastGenerationTime;
    ts::Tai mReceptionTime;
    ts::Tai mEmissionTime;

    /// Ephemeris Parameters
    Float3 mEphPosition;
    Float3 mEphVelocity;
    double mEphClockBias;
    double mEphRange;
    Float3 mEphLineOfSight;

    /// True Parameters
    Float3 mTruePosition;
    Float3 mTrueVelocity;
    double mTrueRange;
    Float3 mTrueLineOfSight;
    double mTrueAzimuth;
    double mTrueElevation;

    OrbitCorrection mOrbitCorrection;
    ClockCorrection mClockCorrection;

    std::vector<Observation> mObservations;

    Generator const& mGenerator;
};
;

}  // namespace tokoro
}  // namespace generator
