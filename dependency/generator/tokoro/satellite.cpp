#include "satellite.hpp"
#include "coordinate.hpp"
#include "data.hpp"
#include "generator.hpp"
#include "models/helper.hpp"
#include "coordinates/enu.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

#define ORBIT_CORRECTED_IN_ITERATION 0

namespace generator {
namespace tokoro {

Satellite::Satellite(SatelliteId id, Float3 ground_position, Generator const& generator) NOEXCEPT
    : mId{id},
      mGroundPositionEcef{ground_position},
      mEnabled{false},
      mGenerator{generator} {
    mGroundPositionLlh = ecef_to_llh(mGroundPositionEcef, ellipsoid::WGS84);
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
    if (!mGenerator.find_ephemeris(mId, generation_time, mOrbitCorrection.iode, eph)) {
        WARNF("ephemeris not found [sv=%s,iode=%u]", mId.name(), mOrbitCorrection.iode);
        return;
    }

    auto current_time = generation_time;
    auto next_time    = generation_time + ts::Timestamp{1.0};

    if (!compute_true_position(mId, mGroundPositionEcef, current_time, eph, mOrbitCorrection,
                               mCurrentLocation)) {
        WARNF("failed to compute true position [sv=%s]", mId.name());
        return;
    }

    if (!compute_true_position(mId, mGroundPositionEcef, next_time, eph, mOrbitCorrection,
                               mNextLocation)) {
        WARNF("failed to compute true position [sv=%s]", mId.name());
        return;
    }

    if (!compute_azimuth_and_elevation(mId, mGroundPositionLlh, mCurrentLocation)) {
        WARNF("failed to compute azimuth and elevation [sv=%s]", mId.name());
        return;
    }

    if (!compute_azimuth_and_elevation(mId, mGroundPositionLlh, mNextLocation)) {
        WARNF("failed to compute azimuth and elevation [sv=%s]", mId.name());
        return;
    }

    mEnabled = true;
}

bool Satellite::compute_true_position(SatelliteId id, Float3 ground_position,
                                      ts::Tai const&              reception_time,
                                      ephemeris::Ephemeris const& eph,
                                      OrbitCorrection const&      orbit_correction,
                                      SatelliteLocation&          location) NOEXCEPT {
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

#if ORBIT_CORRECTED_IN_ITERATION
        // correct the satellite position
        Float3 satellite_position{};
        if (!orbit_correction.correction(ts::Tai{t_e}, result.position, result.velocity,
                                         satellite_position)) {
            WARNF("failed to correct satellite position");
        }
#else
        auto satellite_position = result.position;
#endif

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

    location.eph_iode       = eph.iode();
    location.eph_position   = final_result.position;
    location.eph_velocity   = final_result.velocity;
    location.eph_clock_bias = final_result.clock;
    location.eph_range =
        geometric_distance(location.eph_position, ground_position, &location.eph_line_of_sight);
    if (!location.eph_line_of_sight.normalize()) {
        WARNF("line of sight (eph) could not be normalized");
    }

    location.true_position = location.eph_position;
    // TODO(ewasjon): Does the orbit velocity differ between the uncorrected and corrected
    // positions? I think so, because the orbit correction can have time-dependent terms.
    location.true_velocity = location.eph_velocity;
    if (!orbit_correction.correction(t_e, location.eph_position, location.eph_velocity,
                                     location.true_position)) {
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

    auto relative_correction =
        eph.relativistic_correction(location.true_position, location.true_velocity);
    location.eph_clock_bias += relative_correction;

    location.true_range =
        geometric_distance(location.true_position, ground_position, &location.true_line_of_sight);
    if (!location.true_line_of_sight.normalize()) {
        WARNF("line of sight (true) could not be normalized");
    }

    auto true_travel_time   = location.true_range / constant::SPEED_OF_LIGHT;
    location.reception_time = t_r;
    location.emission_time  = t_r + ts::Timestamp{-true_travel_time};
    auto calculated_time    = location.emission_time + ts::Timestamp{true_travel_time};

    VERBOSEF("eph parameters:");
    VERBOSEF("    position: (%.14f, %.14f, %.14f)", location.eph_position.x,
             location.eph_position.y, location.eph_position.z);
    VERBOSEF("    velocity: (%f, %f, %f)", location.eph_velocity.x, location.eph_velocity.y,
             location.eph_velocity.z);
    VERBOSEF("    clock bias: %+.14f", location.eph_clock_bias);
    VERBOSEF("    range: %.14f", location.eph_range);
    VERBOSEF("    line of sight: (%f, %f, %f)", location.eph_line_of_sight.x,
             location.eph_line_of_sight.y, location.eph_line_of_sight.z);

    VERBOSEF("true parameters:");
    VERBOSEF("    t_e:             %s (GPS %.16f)", t_e.rtklib_time_string().c_str(),
             ts::Gps{t_e}.time_of_week().full_seconds());
    VERBOSEF("    emission time:   %s (GPS %.16f)",
             location.emission_time.rtklib_time_string().c_str(),
             ts::Gps{location.emission_time}.time_of_week().full_seconds());
    VERBOSEF("    reception time:  %s (GPS %.16f)",
             location.reception_time.rtklib_time_string().c_str(),
             ts::Gps{location.reception_time}.time_of_week().full_seconds());
    VERBOSEF("    calculated time: %s (GPS %.16f)", calculated_time.rtklib_time_string().c_str(),
             ts::Gps{calculated_time}.time_of_week().full_seconds());
    VERBOSEF("    position: (%.14f, %.14f, %.14f)", location.true_position.x,
             location.true_position.y, location.true_position.z);
    VERBOSEF("    velocity: (%f, %f, %f)", location.true_velocity.x, location.true_velocity.y,
             location.true_velocity.z);
    VERBOSEF("    range: %.14f", location.true_range);
    VERBOSEF("    line of sight: (%f, %f, %f)", location.true_line_of_sight.x,
             location.true_line_of_sight.y, location.true_line_of_sight.z);

    return true;
}

bool Satellite::compute_azimuth_and_elevation(SatelliteId id, Float3 ground_position_llh,
                                              SatelliteLocation& location) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", id.name());

    location.true_azimuth   = 0.0;
    location.true_elevation = 0.0;

    if (location.true_range < 1.0) {
        VERBOSEF("satellite is too close");
        return false;
    }

    if (ground_position_llh.z > -constant::RE_WGS84) {
        auto enu = ecef_to_enu_at_llh(ground_position_llh, location.true_line_of_sight);
        VERBOSEF("ENU: (%f, %f, %f)", enu.x, enu.y, enu.z);

        auto azimuth = 0.0;
        if (enu.length_squared() >= 1e-12) {
            azimuth = std::atan2(enu.x, enu.y);
        }

        if (azimuth < 0) {
            azimuth += 2 * constant::PI;
        }

        auto elevation          = std::asin(enu.z);
        location.true_azimuth   = azimuth;
        location.true_elevation = elevation;
        VERBOSEF("azimuth: %.2f, elevation: %.2f", location.true_azimuth * constant::RAD2DEG,
                 location.true_elevation * constant::RAD2DEG);
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
        DEBUGF("satellite missing orbit corrections [sv=%s]", mId.name());
        return false;
    }

    mOrbitCorrection = *correction;
    return true;
}

bool Satellite::find_clock_correction(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    auto correction = correction_data.clock_correction(mId);
    if (!correction) {
        DEBUGF("satellite missing clock corrections [sv=%s]", mId.name());
        return false;
    }

    mClockCorrection = *correction;
    return true;
}

double Satellite::pseudorange() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());
    auto orbit = mCurrentLocation.true_range - mCurrentLocation.eph_range;
    VERBOSEF("orbit:      %+24.10f", orbit);
    auto clock_bias = constant::SPEED_OF_LIGHT * -mCurrentLocation.eph_clock_bias;
    VERBOSEF("clock_bias: %+24.10f", clock_bias);
    auto clock = clock_correction();
    VERBOSEF("clock:      %+24.10f", clock);

    auto result = mCurrentLocation.true_range + orbit + clock_bias + clock;
    VERBOSEF("==          %+24.10f", result);
    return result;
}

double Satellite::clock_correction() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());
    auto delta_t_sv = mClockCorrection.correction(mCurrentLocation.emission_time);
    VERBOSEF("delta_t_sv: %f", delta_t_sv);
    return delta_t_sv;
}

NODISCARD double Satellite::average_code_range() const NOEXCEPT {
    if (mObservations.size() == 0) return 0.0;
    double sum = 0.0;
    for (auto const& observation : mObservations) {
        sum += observation.code_range();
    }
    return sum / mObservations.size();
}

NODISCARD double Satellite::average_phase_range_rate() const NOEXCEPT {
    if (mObservations.size() == 0) return 0.0;
    double sum = 0.0;
    for (auto const& observation : mObservations) {
        sum += observation.phase_range_rate();
    }
    return sum / mObservations.size();
}

}  // namespace tokoro
}  // namespace generator
