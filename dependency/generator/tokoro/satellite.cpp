#include "satellite.hpp"
#include "coordinate.hpp"
#include "coordinates/enu.hpp"
#include "data/correction.hpp"
#include "generator.hpp"
#include "models/helper.hpp"

#include <cmath>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

#include <algorithm>

LOGLET_MODULE2(tokoro, sat);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, sat)

namespace generator {
namespace tokoro {

Satellite::Satellite(SatelliteId id, Float3 ground_position, Generator const& generator) NOEXCEPT
    : mId(id),
      mGroundPositionEcef(ground_position),
      mEnabled(false),
      mGenerator(generator) {
    mGroundPositionLlh = ecef_to_llh(mGroundPositionEcef, ellipsoid::WGS84);
    mCurrentState      = {};
    mNextState         = {};
}

void Satellite::update(ts::Tai const& generation_time) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    mEnabled            = false;
    mLastGenerationTime = generation_time;
    if (!mGenerator.mCorrectionData) {
        WARNF("no correction data available [sv=%s]", mId.name());
        return;
    }

    // Find orbit and clock corrections
    if (!find_orbit_correction(*mGenerator.mCorrectionData)) return;
    if (!find_clock_correction(*mGenerator.mCorrectionData)) return;

    // Find broadcast ephemeris
    ephemeris::Ephemeris eph{};
    if (!mGenerator.find_ephemeris(mId, generation_time, mOrbitCorrection.iod, eph)) {
        DEBUGF("ephemeris not found [sv=%s,iod=%u]", mId.name(), mOrbitCorrection.iod);
        return;
    }

    auto current_time = generation_time;
    auto next_time    = generation_time + ts::Timestamp{0.1};

    if (!compute_true_position(mId, mGroundPositionEcef, current_time, eph, mOrbitCorrection,
                               mCurrentState,
                               mGenerator.mUseReceptionTimeForOrbitAndClockCorrections,
                               mGenerator.mUseOrbitCorrectionInIteration)) {
        WARNF("failed to compute true position [sv=%s]", mId.name());
        return;
    }

    if (!compute_true_position(mId, mGroundPositionEcef, next_time, eph, mOrbitCorrection,
                               mNextState, mGenerator.mUseReceptionTimeForOrbitAndClockCorrections,
                               mGenerator.mUseOrbitCorrectionInIteration)) {
        WARNF("failed to compute true position [sv=%s]", mId.name());
        return;
    }

    if (!compute_azimuth_and_elevation(mId, mGroundPositionLlh, mCurrentState)) {
        WARNF("failed to compute azimuth and elevation [sv=%s]", mId.name());
        return;
    }

    if (!compute_azimuth_and_elevation(mId, mGroundPositionLlh, mNextState)) {
        WARNF("failed to compute azimuth and elevation [sv=%s]", mId.name());
        return;
    }

    mEnabled = true;
}

void Satellite::datatrace_report() NOEXCEPT {
    if (!mEnabled) return;
    VSCOPE_FUNCTION();
#ifdef DATA_TRACING
    datatrace::Satellite dt_sat{};
    dt_sat.position             = mCurrentState.true_position;
    dt_sat.velocity             = mCurrentState.true_velocity;
    dt_sat.eph_position         = mCurrentState.eph_position;
    dt_sat.elevation            = mCurrentState.true_elevation * constant::RAD2DEG;
    dt_sat.azimuth              = mCurrentState.true_azimuth * constant::RAD2DEG;
    dt_sat.nadir                = mCurrentState.true_nadir * constant::RAD2DEG;
    dt_sat.iod                  = mCurrentState.eph_iod;
    dt_sat.ground_position_ecef = mGroundPositionEcef;
    dt_sat.ground_position_llh  = mGroundPositionLlh;
    dt_sat.reception_time       = mCurrentState.reception_time.timestamp().full_seconds();
    dt_sat.emission_time        = mCurrentState.emission_time.timestamp().full_seconds();
    dt_sat.sun_position         = mCurrentState.sun_moon_position.sun;
    dt_sat.moon_position        = mCurrentState.sun_moon_position.moon;
    dt_sat.gmst                 = mCurrentState.sun_moon_position.gmst;
    datatrace::report_satellite(mCurrentState.reception_time, mId.name(), dt_sat);
#endif
}

bool Satellite::compute_true_position(SatelliteId id, Float3 ground_position,
                                      ts::Tai const&              reception_time,
                                      ephemeris::Ephemeris const& eph,
                                      OrbitCorrection const&      orbit_correction,
                                      SatelliteState&             state,
                                      bool use_reception_time_for_orbit_and_clock_corrections,
                                      bool use_orbit_correction_in_iteration) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", id.name());

    VERBOSEF("iode: %u", eph.iode());
    VERBOSEF("iodc: %u", eph.iodc());

    // initial guess is that emission = reception
    auto t_r = reception_time;
    auto t_e = t_r;

    // because the t_e can never equal t_r, the initial guess can be shifted by a small amount
    // (e.g. -0.08 seconds) to help find the correct emission time faster (this is was RTKLIB/CLAS
    // uses)
    t_e = t_e + ts::Timestamp{-0.08};

    for (auto i = 0; i < 10; i++) {
        VERBOSEF("iteration %i: %+f us  %s (GPS %.16f)", i,
                 t_e.difference(t_r).full_seconds() * 1000000.0, t_e.rtklib_time_string().c_str(),
                 ts::Gps{t_e}.time_of_week().full_seconds());

        // ephemeral position at t_e
        auto result = eph.compute(t_e);
        VERBOSEF("    x=%f, y=%f, z=%f", result.position.x, result.position.y, result.position.z);
        VERBOSEF("    dx=%f, dy=%f, dz=%f", result.velocity.x, result.velocity.y,
                 result.velocity.z);
        VERBOSEF("    clock_bias=%f", result.clock);

        Float3 satellite_position{};
        if (use_orbit_correction_in_iteration) {
            // correct the satellite position
            if (!orbit_correction.correction(ts::Tai{t_e}, result.position, result.velocity,
                                             satellite_position, nullptr, nullptr, nullptr,
                                             nullptr)) {
                WARNF("failed to correct satellite position");
            }
        } else {
            satellite_position = result.position;
        }

        // compute the pseudo-range (this is not the true range, as it contains the satellite orbit
        // error and the satellite clock error)
        auto pseudorange = geometric_distance(satellite_position, ground_position);
        auto travel_time = pseudorange / constant::SPEED_OF_LIGHT;
        VERBOSEF("    range=%f, time=%f", i, pseudorange, travel_time);

        // the new emission time is the reception time minus the travel time
        auto new_t_e   = t_r + ts::Timestamp{-travel_time};
        auto delta_t_e = new_t_e.difference(t_e);

        t_e = new_t_e;

        // check if the delta has converged
        if (std::abs(delta_t_e.full_seconds()) < 1.0e-12) {
            break;
        }
    }

    auto final_result = eph.compute(t_e);

#if 0
    auto t_e2         = t_e + ts::Timestamp{0.1};
    auto final_result2 = mEph.compute(t_e2);
    auto delta_position = final_result2.position - final_result.position;
    VERBOSEF("delta_position: (%f, %f, %f)", delta_position.x, delta_position.y, delta_position.z);
    auto velocity = delta_position / 0.1;
    VERBOSEF("velocity by ts: (%f, %f, %f)", velocity.x, velocity.y, velocity.z);
#endif

    state.eph_iod        = eph.iod();
    state.eph_position   = final_result.position;
    state.eph_velocity   = final_result.velocity;
    state.eph_clock_bias = final_result.clock;
    state.eph_range =
        geometric_distance(state.eph_position, ground_position, &state.eph_line_of_sight);
    if (!state.eph_line_of_sight.normalize()) {
        WARNF("line of sight (eph) could not be normalized");
    }

    state.true_position = state.eph_position;
    // TODO(ewasjon): Does the orbit velocity differ between the uncorrected and corrected
    // positions? I think so, because the orbit correction can have time-dependent terms.
    state.true_velocity = state.eph_velocity;

    // TODO(ewasjon): Which "time" should be used for the orbit correction?
    auto reference_time = t_e;
    if (use_reception_time_for_orbit_and_clock_corrections) {
        reference_time = t_r;
    }

    if (!orbit_correction.correction(reference_time, state.eph_position, state.eph_velocity,
                                     state.true_position, &state.orbit_radial_axis,
                                     &state.orbit_along_axis, &state.orbit_cross_axis,
                                     &state.orbit_delta_t)) {
        WARNF("failed to correct satellite position");
    }

#if 0
    for (int orbit_r = 0; orbit_r <= 1; orbit_r++) {
        for (int orbit_a = 0; orbit_a <= 1; orbit_a++) {
            for (int orbit_c = 0; orbit_c <= 1; orbit_c++) {
                auto new_orbit_correction = mOrbitCorrection;
                if (orbit_r == 1) {
                    new_orbit_correction.delta.x     = -new_orbit_correction.delta.x;
                    new_orbit_correction.dot_delta.x = -new_orbit_correction.dot_delta.x;
                }
                if (orbit_a == 1) {
                    new_orbit_correction.delta.y     = -new_orbit_correction.delta.y;
                    new_orbit_correction.dot_delta.y = -new_orbit_correction.dot_delta.y;
                }
                if (orbit_c == 1) {
                    new_orbit_correction.delta.z     = -new_orbit_correction.delta.z;
                    new_orbit_correction.dot_delta.z = -new_orbit_correction.dot_delta.z;
                }
                Float3 orbit_corrected_position{};
                new_orbit_correction.correction(t_e, mEphPosition, mEphVelocity,
                                                                    orbit_corrected_position);

                Float3 line_of_sight{};
                auto   orbit_corrected_range = geometric_distance(orbit_corrected_position,
                                                                  mReceptionLocation, &line_of_sight);
                VERBOSEF("[%s,%s,%s]: %.14f, %.14f, %.14f, %.14f", orbit_r == 0 ? "+R" : "-R",
                         orbit_a == 0 ? "+A" : "-A", orbit_c == 0 ? "+C" : "-C",
                         orbit_corrected_position.x, orbit_corrected_position.y,
                         orbit_corrected_position.z, orbit_corrected_range);
            }
        }
    }

#endif

    auto relativistic_correction =
        eph.relativistic_correction(state.true_position, state.true_velocity);
    state.eph_clock_bias += relativistic_correction;
    state.eph_relativistic_correction = relativistic_correction;

    state.ground_position = ground_position;
    state.true_range =
        geometric_distance(state.true_position, ground_position, &state.true_line_of_sight);
    if (!state.true_line_of_sight.normalize()) {
        WARNF("line of sight (true) could not be normalized");
    }

    auto true_travel_time = state.true_range / constant::SPEED_OF_LIGHT;
    state.reception_time  = t_r;
    state.emission_time   = t_r + ts::Timestamp{-true_travel_time};
    auto calculated_time  = state.emission_time + ts::Timestamp{true_travel_time};

    VERBOSEF("eph parameters:");
    VERBOSEF("    position: (%.14f, %.14f, %.14f)", state.eph_position.x, state.eph_position.y,
             state.eph_position.z);
    VERBOSEF("    velocity: (%f, %f, %f)", state.eph_velocity.x, state.eph_velocity.y,
             state.eph_velocity.z);
    VERBOSEF("    clock bias: %+.14f", state.eph_clock_bias);
    VERBOSEF("    range: %.14f", state.eph_range);
    VERBOSEF("    line of sight: (%f, %f, %f)", state.eph_line_of_sight.x,
             state.eph_line_of_sight.y, state.eph_line_of_sight.z);

    VERBOSEF("true parameters:");
    VERBOSEF("    t_e:             %s (GPS %.16f)", t_e.rtklib_time_string().c_str(),
             ts::Gps{t_e}.time_of_week().full_seconds());
    VERBOSEF("    emission time:   %s (GPS %.16f)",
             state.emission_time.rtklib_time_string().c_str(),
             ts::Gps{state.emission_time}.time_of_week().full_seconds());
    VERBOSEF("    reception time:  %s (GPS %.16f)",
             state.reception_time.rtklib_time_string().c_str(),
             ts::Gps{state.reception_time}.time_of_week().full_seconds());
    VERBOSEF("    calculated time: %s (GPS %.16f)", calculated_time.rtklib_time_string().c_str(),
             ts::Gps{calculated_time}.time_of_week().full_seconds());
    VERBOSEF("    position: (%.14f, %.14f, %.14f)", state.true_position.x, state.true_position.y,
             state.true_position.z);
    VERBOSEF("    velocity: (%f, %f, %f)", state.true_velocity.x, state.true_velocity.y,
             state.true_velocity.z);
    VERBOSEF("    range: %.14f", state.true_range);
    VERBOSEF("    line of sight: (%f, %f, %f)", state.true_line_of_sight.x,
             state.true_line_of_sight.y, state.true_line_of_sight.z);

    return true;
}

bool Satellite::compute_azimuth_and_elevation(SatelliteId id, Float3 ground_position_llh,
                                              SatelliteState& state) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", id.name());

    state.true_azimuth   = 0.0;
    state.true_elevation = 0.0;
    state.true_nadir     = 0.0;

    if (state.true_range < 1.0) {
        VERBOSEF("satellite is too close");
        return false;
    }

    if (ground_position_llh.z > -constant::RE_WGS84) {
        auto enu = ecef_to_enu_at_llh(ground_position_llh, state.true_line_of_sight);
        VERBOSEF("ENU: (%f, %f, %f)", enu.x, enu.y, enu.z);

        auto azimuth = 0.0;
        if (enu.length_squared() >= 1e-12) {
            azimuth = std::atan2(enu.x, enu.y);
        }

        if (azimuth < 0) {
            azimuth += 2 * constant::PI;
        }

        auto elevation = std::asin(enu.z);

        auto satellite_to_earth    = -state.true_position;
        auto satellite_to_receiver = state.ground_position - state.true_position;
        if (!satellite_to_earth.normalize()) {
            WARNF("failed to normalize satellite to earth vector");
            return false;
        } else if (!satellite_to_receiver.normalize()) {
            WARNF("failed to normalize satellite to receiver vector");
            return false;
        }

        auto nadir           = std::acos(dot_product(satellite_to_earth, satellite_to_receiver));
        state.true_azimuth   = azimuth;
        state.true_elevation = elevation;
        state.true_nadir     = nadir;
        VERBOSEF("azimuth: %.2f, elevation: %.2f, nadir: %.2f",
                 state.true_azimuth * constant::RAD2DEG, state.true_elevation * constant::RAD2DEG,
                 state.true_nadir * constant::RAD2DEG);
        return true;
    } else {
        VERBOSEF("altitude is below sea level");
        return false;
    }
}

bool Satellite::find_orbit_correction(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    auto correction = correction_data.orbit_correction(mId);
    if (!correction) {
        VERBOSEF("satellite missing orbit corrections [sv=%s]", mId.name());
        return false;
    }

    mOrbitCorrection = *correction;
    return true;
}

bool Satellite::find_clock_correction(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    auto correction = correction_data.clock_correction(mId);
    if (!correction) {
        VERBOSEF("satellite missing clock corrections [sv=%s]", mId.name());
        return false;
    }

    mClockCorrection = *correction;
    return true;
}

void Satellite::compute_sun_position() NOEXCEPT {
    compute_sun_position(mCurrentState);
    compute_sun_position(mNextState);
}

void Satellite::compute_sun_position(SatelliteState& state) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mId.name(), state.reception_time.rtklib_time_string().c_str());

    state.sun_moon_position = sun_and_moon_position_ecef(state.reception_time);
}

void Satellite::compute_shapiro() NOEXCEPT {
    compute_shapiro(mCurrentState);
    compute_shapiro(mNextState);
}

void Satellite::compute_earth_solid_tides() NOEXCEPT {
    compute_earth_solid_tides(mCurrentState);
    compute_earth_solid_tides(mNextState);
}

void Satellite::compute_phase_windup() NOEXCEPT {
    compute_phase_windup(mCurrentState);

    // The phase windup is dependent on the previous state, instead of using the previous next
    // state, use the new current state
    mNextState.phase_windup = mCurrentState.phase_windup;
    compute_phase_windup(mNextState);
}

void Satellite::compute_shapiro(SatelliteState& state) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mId.name(), state.reception_time.rtklib_time_string().c_str());

    state.shapiro = model_shapiro(state, mGroundPositionEcef);
}

void Satellite::compute_earth_solid_tides(SatelliteState& state) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mId.name(), state.reception_time.rtklib_time_string().c_str());

    // TODO(ewasjon): Use sun_moon_position in model_earth_solid_tides instead of re-computing it
    state.earth_solid_tides = model_earth_solid_tides(state.reception_time, state,
                                                      mGroundPositionEcef, mGroundPositionLlh);

    VERBOSEF("disp x: %+.14f * %+.14f = %+.14f", state.earth_solid_tides.displacement_vector.x,
             state.true_line_of_sight.x,
             state.earth_solid_tides.displacement_vector.x * state.true_line_of_sight.x);
    VERBOSEF("disp y: %+.14f * %+.14f = %+.14f", state.earth_solid_tides.displacement_vector.y,
             state.true_line_of_sight.y,
             state.earth_solid_tides.displacement_vector.y * state.true_line_of_sight.y);
    VERBOSEF("disp z: %+.14f * %+.14f = %+.14f", state.earth_solid_tides.displacement_vector.z,
             state.true_line_of_sight.z,
             state.earth_solid_tides.displacement_vector.z * state.true_line_of_sight.z);
    VERBOSEF("solid tides: %+.14f", state.earth_solid_tides.displacement);
}

void Satellite::compute_phase_windup(SatelliteState& state) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s, %s", mId.name(), state.reception_time.rtklib_time_string().c_str());

    // TODO(ewasjon): Use sun_moon_position in model_phase_windup instead of re-computing it
    state.phase_windup = model_phase_windup(state.reception_time, state, mGroundPositionEcef,
                                            mGroundPositionLlh, state.phase_windup);
    VERBOSEF("phase_windup: %+.14f", state.phase_windup.correction_sun);
}

double Satellite::pseudorange() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());
    auto orbit = mCurrentState.true_range - mCurrentState.eph_range;
    VERBOSEF("orbit:      %+24.10f", orbit);
    auto clock_bias = constant::SPEED_OF_LIGHT * -mCurrentState.eph_clock_bias;
    VERBOSEF("clock_bias: %+24.10f", clock_bias);
    auto clock = clock_correction();
    VERBOSEF("clock:      %+24.10f", clock);

    auto result = mCurrentState.true_range + orbit + clock_bias + clock;
    VERBOSEF("==          %+24.10f", result);
    return result;
}

double Satellite::clock_correction() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    auto reference_time = mCurrentState.emission_time;
    if (mGenerator.mUseReceptionTimeForOrbitAndClockCorrections) {
        reference_time = mCurrentState.reception_time;
    }

    auto delta_t_sv = mClockCorrection.correction(reference_time);
    VERBOSEF("delta_t_sv: %f", delta_t_sv);
    return delta_t_sv;
}

double Satellite::average_code_range() const NOEXCEPT {
    if (mObservations.size() == 0) return 0.0;
    double sum = 0.0;
    for (auto const& observation : mObservations) {
        sum += observation.code_range();
    }
    return sum / static_cast<double>(mObservations.size());
}

double Satellite::average_phase_range_rate() const NOEXCEPT {
    if (mObservations.size() == 0) return 0.0;
    double sum = 0.0;
    for (auto const& observation : mObservations) {
        sum += observation.phase_range_rate();
    }
    return sum / static_cast<double>(mObservations.size());
}

void Satellite::remove_discarded_observations() NOEXCEPT {
    mObservations.erase(std::remove_if(mObservations.begin(), mObservations.end(),
                                       [](Observation const& observation) {
                                           return !observation.is_valid();
                                       }),
                        mObservations.end());
}

}  // namespace tokoro
}  // namespace generator
