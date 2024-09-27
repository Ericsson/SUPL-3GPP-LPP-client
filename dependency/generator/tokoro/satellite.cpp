#include "satellite.hpp"
#include "data.hpp"
#include "ecef.hpp"
#include "helper.hpp"
#include "wgs84.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

#define ORBIT_CORRECTED_IN_ITERATION 0

namespace generator {
namespace tokoro {

Satellite::Satellite(SatelliteId id, ephemeris::Ephemeris ephemeris, ts::Tai reception_time,
                     Float3 vrs_location) NOEXCEPT : mId{id},
                                                     mEph{ephemeris},
                                                     mReceptionTime{reception_time},
                                                     mReceptionLocation{vrs_location} {}

bool Satellite::compute_true_position() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    // initial guess is that emission = reception
    auto t_r = mReceptionTime;
    auto t_e = t_r;

    // because the t_e can never equal t_r, the initial guess can be shifted by a small amount
    // (e.g. -0.08 seconds) to help find the correct emission time faster (this is was RTKLIB/CLAS
    // uses)
    t_e = t_e + ts::Timestamp{-0.08};

    for (auto i = 0; i < 10; i++) {
        VERBOSEF("iteration %i: %+f us", i, t_e.difference(t_r).full_seconds() * 1000000.0);

        // ephemeral position at t_e
        auto result = mEph.compute(t_e);
        VERBOSEF("    x=%f, y=%f, z=%f", result.position.x, result.position.y, result.position.z);
        VERBOSEF("    dx=%f, dy=%f, dz=%f", result.velocity.x, result.velocity.y,
                 result.velocity.z);
        VERBOSEF("    clock_bias=%f", result.clock);

#if ORBIT_CORRECTED_IN_ITERATION
        // correct the satellite position
        Float3 satellite_position{};
        if (!mOrbitCorrection.correction(ts::Tai{t_e}, result.position, result.velocity,
                                         satellite_position)) {
            WARNF("failed to correct satellite position");
        }
#else
        auto satellite_position = result.position;
#endif

        // compute the pseudo-range (this is not the true range, as it contains the satellite orbit
        // error and the satellite clock error)
        auto pseudorange = geometric_distance(satellite_position, mReceptionLocation);
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

    auto t_e2          = t_e + ts::Timestamp{0.1};
    auto final_result  = mEph.compute(t_e);
    auto final_result2 = mEph.compute(t_e2);

    auto delta_position = final_result2.position - final_result.position;
    VERBOSEF("delta_position: (%f, %f, %f)", delta_position.x, delta_position.y, delta_position.z);
    auto velocity = delta_position / 0.1;
    VERBOSEF("velocity: (%f, %f, %f)", velocity.x, velocity.y, velocity.z);

    mEphPosition  = final_result.position;
    mEphVelocity  = final_result.velocity;
    mEphClockBias = final_result.clock;
    mEphRange     = geometric_distance(mEphPosition, mReceptionLocation, &mEphLineOfSight);
    if (!mEphLineOfSight.normalize()) {
        WARNF("line of sight (eph) could not be normalized");
    }

    mTruePosition = mEphPosition;
    // TODO(ewasjon): Does the orbit velocity differ between the uncorrected and corrected
    // positions? I think so, because the orbit correction can have time-dependent terms.
    mTrueVelocity = mEphVelocity;
    if (!mOrbitCorrection.correction(t_e, mEphPosition, mEphVelocity, mTruePosition)) {
        WARNF("failed to correct satellite position");
    }

    mTrueRange = geometric_distance(mTruePosition, mReceptionLocation, &mTrueLineOfSight);
    if (!mTrueLineOfSight.normalize()) {
        WARNF("line of sight (true) could not be normalized");
    }

    auto true_travel_time = mTrueRange / constant::SPEED_OF_LIGHT;
    mEmissionTime         = t_r + ts::Timestamp{-true_travel_time};
    auto calculated_time  = mEmissionTime + ts::Timestamp{true_travel_time};

    VERBOSEF("eph parameters:");
    VERBOSEF("    position: (%.14f, %.14f, %.14f)", mEphPosition.x, mEphPosition.y, mEphPosition.z);
    VERBOSEF("    velocity: (%f, %f, %f)", mEphVelocity.x, mEphVelocity.y, mEphVelocity.z);
    VERBOSEF("    clock bias: %+.14f", mEphClockBias);
    VERBOSEF("    range: %.14f", mEphRange);
    VERBOSEF("    line of sight: (%f, %f, %f)", mEphLineOfSight.x, mEphLineOfSight.y,
             mEphLineOfSight.z);

    VERBOSEF("true parameters:");
    VERBOSEF("    emission time:   %s", mEmissionTime.rtklib_time_string().c_str());
    VERBOSEF("    reception time:  %s", mReceptionTime.rtklib_time_string().c_str());
    VERBOSEF("    calculated time: %s", calculated_time.rtklib_time_string().c_str());
    VERBOSEF("    position: (%.14f, %.14f, %.14f)", mTruePosition.x, mTruePosition.y,
             mTruePosition.z);
    VERBOSEF("    velocity: (%f, %f, %f)", mTrueVelocity.x, mTrueVelocity.y, mTrueVelocity.z);
    VERBOSEF("    range: %.14f", mTrueRange);
    VERBOSEF("    line of sight: (%f, %f, %f)", mTrueLineOfSight.x, mTrueLineOfSight.y,
             mTrueLineOfSight.z);
    return true;
}

bool Satellite::compute_azimuth_and_elevation() NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    mTrueAzimuth   = 0.0;
    mTrueElevation = 0.0;

    if (mTrueRange < 1.0) {
        VERBOSEF("satellite is too close");
        return false;
    }

    Wgs84Position wgs84{};
    wgs84 = ecef_to_wgs84(mReceptionLocation);
    VERBOSEF("WGS84: (%f, %f, %f)", wgs84.x, wgs84.y, wgs84.z);

    if (wgs84.z > -constant::RE_WGS84) {
        Float3 enu{};
        if (!ecef_to_enu(wgs84, mTrueLineOfSight, enu)) {
            VERBOSEF("failed to convert ECEF to ENU");
            return false;
        }

        VERBOSEF("ENU: (%f, %f, %f)", enu.x, enu.y, enu.z);

        auto azimuth = 0.0;
        if (enu.length_squared() >= 1e-12) {
            azimuth = std::atan2(enu.x, enu.y);
        }

        if (azimuth < 0) {
            azimuth += 2 * constant::PI;
        }

        auto elevation = std::asin(enu.z);
        mTrueAzimuth   = azimuth;
        mTrueElevation = elevation;
        VERBOSEF("azimuth: %.2f, elevation: %.2f", mTrueAzimuth * constant::RAD2DEG,
                 mTrueElevation * constant::RAD2DEG);
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
        WARNF("satellite missing orbit corrections [sv=%s]", mId.name());
        return false;
    }

    mOrbitCorrection = *correction;
    return true;
}

bool Satellite::find_clock_correction(CorrectionData const& correction_data) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());

    auto correction = correction_data.clock_correction(mId);
    if (!correction) {
        WARNF("satellite missing clock corrections [sv=%s]", mId.name());
        return false;
    }

    mClockCorrection = *correction;
    return true;
}

double Satellite::pseudorange() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());
    auto orbit = mTrueRange - mEphRange;
    VERBOSEF("orbit:      %+24.10f", orbit);
    auto clock_bias = constant::SPEED_OF_LIGHT * -mEphClockBias;
    VERBOSEF("clock_bias: %+24.10f", clock_bias);
    auto clock = clock_correction();
    VERBOSEF("clock:      %+24.10f", clock);

    auto result = mTrueRange + orbit + clock_bias + clock;
    VERBOSEF("==          %+24.10f", result);
    return result;
}

double Satellite::clock_correction() const NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", mId.name());
    auto delta_t_sv = mClockCorrection.correction(mEmissionTime);
    VERBOSEF("delta_t_sv: %f", delta_t_sv);
    return delta_t_sv;
}

}  // namespace tokoro
}  // namespace generator
