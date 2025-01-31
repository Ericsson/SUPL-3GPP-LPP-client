#pragma once
#include "constant.hpp"
#include "data.hpp"
#include "models/earth_solid_tides.hpp"
#include "models/phase_windup.hpp"
#include "models/shapiro.hpp"
#include "observation.hpp"
#include "sv_id.hpp"

#include <ephemeris/ephemeris.hpp>
#include <generator/rtcm/satellite_id.hpp>
#include <maths/float3.hpp>
#include <time/gps.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct SatelliteState {
    ts::Tai reception_time;
    ts::Tai emission_time;
    Float3 ground_position;

    /// Ephemeris Parameters
    Float3   eph_position;
    Float3   eph_velocity;
    double   eph_clock_bias;
    double   eph_range;
    double   eph_relativistic_correction;
    Float3   eph_line_of_sight;
    uint16_t eph_iod;

    /// True Parameters
    Float3 true_position;
    Float3 true_velocity;
    double true_range;
    Float3 true_line_of_sight;
    double true_azimuth;
    double true_elevation;
    double true_nadir;

    Float3 orbit_radial_axis;
    Float3 orbit_along_axis;
    Float3 orbit_cross_axis;
    double orbit_delta_t;

    PhaseWindup     phase_windup;
    Shapiro         shapiro;
    EarthSolidTides earth_solid_tides;
};

class Generator;

struct Satellite {
public:
    EXPLICIT Satellite(SatelliteId id, Float3 ground_position, Generator const& generator) NOEXCEPT;

    void update(ts::Tai const& generation_time) NOEXCEPT;

    NODISCARD const SatelliteId& id() const NOEXCEPT { return mId; }

    NODISCARD SatelliteState const& current_state() const NOEXCEPT { return mCurrentState; }
    NODISCARD SatelliteState const& next_state() const NOEXCEPT { return mNextState; }

    NODISCARD double elevation() const NOEXCEPT { return mCurrentState.true_elevation; }

    NODISCARD double pseudorange() const NOEXCEPT;
    NODISCARD double clock_correction() const NOEXCEPT;

    NODISCARD bool enabled() const NOEXCEPT { return mEnabled; }

    void disable() NOEXCEPT { mEnabled = false; }

    NODISCARD double average_code_range() const NOEXCEPT;
    NODISCARD double average_phase_range_rate() const NOEXCEPT;

    void                            reset_observations() NOEXCEPT { mObservations.clear(); }
    std::vector<Observation> const& observations() const NOEXCEPT { return mObservations; }

    Observation& initialize_observation(SignalId signal_id) NOEXCEPT {
        mObservations.emplace_back(*this, signal_id, mGroundPositionEcef);
        return mObservations.back();
    }

    void remove_discarded_observations() NOEXCEPT;

    void compute_shapiro() NOEXCEPT;
    void compute_earth_solid_tides() NOEXCEPT;
    void compute_phase_windup() NOEXCEPT;

    void datatrace_report() NOEXCEPT;

protected:
    NODISCARD static bool
    compute_true_position(SatelliteId id, Float3 ground_position, ts::Tai const& reception_time,
                          ephemeris::Ephemeris const& eph, OrbitCorrection const& orbit_correction,
                          SatelliteState& state,
                          bool            use_reception_time_for_orbit_and_clock_corrections,
                          bool            use_orbit_correction_in_iteration) NOEXCEPT;
    NODISCARD static bool compute_azimuth_and_elevation(SatelliteId id, Float3 ground_position,
                                                        SatelliteState& state) NOEXCEPT;

    NODISCARD bool find_orbit_correction(CorrectionData const& correction_data) NOEXCEPT;
    NODISCARD bool find_clock_correction(CorrectionData const& correction_data) NOEXCEPT;

    void compute_shapiro(SatelliteState& state) NOEXCEPT;
    void compute_earth_solid_tides(SatelliteState& state) NOEXCEPT;
    void compute_phase_windup(SatelliteState& state) NOEXCEPT;

private:
    SatelliteId mId;
    Float3      mGroundPositionEcef;
    Float3      mGroundPositionLlh;
    bool        mEnabled;

    ts::Tai mLastGenerationTime;

    SatelliteState mCurrentState;
    SatelliteState mNextState;

    OrbitCorrection mOrbitCorrection;
    ClockCorrection mClockCorrection;

    std::vector<Observation> mObservations;

    Generator const& mGenerator;
};
;

}  // namespace tokoro
}  // namespace generator
