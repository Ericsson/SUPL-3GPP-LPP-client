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

struct SatelliteLocation {
    ts::Tai reception_time;
    ts::Tai emission_time;

    /// Ephemeris Parameters
    Float3   eph_position;
    Float3   eph_velocity;
    double   eph_clock_bias;
    double   eph_range;
    Float3   eph_line_of_sight;
    uint16_t eph_iode;

    /// True Parameters
    Float3 true_position;
    Float3 true_velocity;
    double true_range;
    Float3 true_line_of_sight;
    double true_azimuth;
    double true_elevation;
};

class Generator;
struct Satellite {
public:
    EXPLICIT Satellite(SatelliteId id, Float3 ground_position, Generator const& generator) NOEXCEPT;

    void update(ts::Tai const& generation_time) NOEXCEPT;

    NODISCARD const SatelliteId& id() const NOEXCEPT { return mId; }

    NODISCARD SatelliteLocation const& current_location() const NOEXCEPT {
        return mCurrentLocation;
    }
    NODISCARD SatelliteLocation const& next_location() const NOEXCEPT { return mNextLocation; }

    NODISCARD double elevation() const NOEXCEPT { return mCurrentLocation.true_elevation; }

    NODISCARD double pseudorange() const NOEXCEPT;
    NODISCARD double clock_correction() const NOEXCEPT;

    NODISCARD bool enabled() const NOEXCEPT { return mEnabled; }

    void disable() NOEXCEPT { mEnabled = false; }

    NODISCARD double average_code_range() const NOEXCEPT;
    NODISCARD double average_phase_range_rate() const NOEXCEPT;

    void                            reset_observations() NOEXCEPT { mObservations.clear(); }
    std::vector<Observation> const& observations() const NOEXCEPT { return mObservations; }

    Observation& initialize_observation(SignalId signal_id) NOEXCEPT {
        mObservations.emplace_back(*this, signal_id, mGroundPosition);
        return mObservations.back();
    }

protected:
    NODISCARD static bool compute_true_position(SatelliteId id, Float3 ground_position,
                                                ts::Tai const&              reception_time,
                                                ephemeris::Ephemeris const& eph,
                                                OrbitCorrection const&      orbit_correction,
                                                SatelliteLocation&          location) NOEXCEPT;
    NODISCARD static bool compute_azimuth_and_elevation(SatelliteId id, Float3 ground_position,
                                                        SatelliteLocation& location) NOEXCEPT;

    NODISCARD bool find_orbit_correction(CorrectionData const& correction_data) NOEXCEPT;
    NODISCARD bool find_clock_correction(CorrectionData const& correction_data) NOEXCEPT;

private:
    SatelliteId mId;
    Float3      mGroundPosition;
    bool        mEnabled;

    ts::Tai mLastGenerationTime;

    SatelliteLocation mCurrentLocation;
    SatelliteLocation mNextLocation;

    OrbitCorrection mOrbitCorrection;
    ClockCorrection mClockCorrection;

    std::vector<Observation> mObservations;

    Generator const& mGenerator;
};
;

}  // namespace tokoro
}  // namespace generator
