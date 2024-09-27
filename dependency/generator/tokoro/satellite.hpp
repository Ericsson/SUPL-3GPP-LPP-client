#pragma once
#include "constant.hpp"
#include "data.hpp"
#include "sv_id.hpp"

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <maths/float3.hpp>
#include <time/gps.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct Satellite {
public:
    EXPLICIT Satellite(SatelliteId id, ephemeris::Ephemeris ephemeris, ts::Tai reception_time,
                       Float3 vrs_location) NOEXCEPT;

    NODISCARD bool compute_true_position() NOEXCEPT;
    NODISCARD bool compute_azimuth_and_elevation() NOEXCEPT;

    NODISCARD bool find_orbit_correction(CorrectionData const& correction_data) NOEXCEPT;
    NODISCARD bool find_clock_correction(CorrectionData const& correction_data) NOEXCEPT;

    NODISCARD const SatelliteId& id() const NOEXCEPT { return mId; }

    NODISCARD ts::Tai reception_time() const NOEXCEPT { return mReceptionTime; }
    NODISCARD ts::Tai emission_time() const NOEXCEPT { return mEmissionTime; }

    NODISCARD double true_range() const NOEXCEPT { return mTrueRange; }
    NODISCARD double pseudorange() const NOEXCEPT;
    NODISCARD double elevation() const NOEXCEPT { return mTrueElevation; }
    NODISCARD double eph_range() const NOEXCEPT { return mEphRange; }
    NODISCARD double eph_clock_bias() const NOEXCEPT { return mEphClockBias; }
    NODISCARD double clock_correction() const NOEXCEPT;

private:
    SatelliteId          mId;
    ephemeris::Ephemeris mEph;

    ts::Tai mReceptionTime;
    ts::Tai mEmissionTime;
    Float3  mReceptionLocation;

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
};
;

}  // namespace tokoro
}  // namespace generator
