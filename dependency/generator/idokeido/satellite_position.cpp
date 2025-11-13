#include "correction.hpp"
#include "eph.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE3(idokeido, sat, pos);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(idokeido, sat, pos)

namespace idokeido {

bool satellite_position(SatelliteId id, ts::Tai receive_time, Scalar pseudo_range,
                        EphemerisEngine const& ephemeris, RelativisticModel relativistic_model,
                        SatellitePosition& result) NOEXCEPT {
    FUNCTION_SCOPEF("%s", id.name());

    // Compute satellite transmit time based on pseudorange. This is an estimation, as the pseudo
    // range contains errors. Assuming the error is <100m, this would give a transmit time error of
    // ~350ns. A satellite is moving roughly at 5km/s, so the time error would give us 350ns*5km =
    // 0.00175m = 0.175cm position difference, which is very small and can be neglected.
    auto t = receive_time;
    VERBOSEF("t_0: %s", t.rtklib_time_string().c_str());

    t.add_seconds(-pseudo_range / constant::c);
    VERBOSEF("t_p: %s (%.4fms, %.4fkm)", t.rtklib_time_string().c_str(),
             1000.0 * (pseudo_range / constant::c), pseudo_range / 1000.0);

    // Estimate satellite clock bias
    Scalar clock_bias = 0.0;
    if (!ephemeris.clock_bias(id, t, clock_bias)) {
        return false;
    }
    t.add_seconds(-clock_bias);
    VERBOSEF("t_c: %s (%.4fus, %.4fkm)", t.rtklib_time_string().c_str(), 1000000.0 * clock_bias,
             clock_bias * constant::c / 1000.0);

    // Compute satellite position and velocity
    EphemerisEngine::Satellite eph{};
    if (!ephemeris.evaluate(id, t, relativistic_model, eph)) {
        return false;
    }

    result.transmit_time  = t;
    result.receive_time   = receive_time;
    result.id             = id;
    result.eph_position   = eph.position;
    result.eph_velocity   = eph.velocity;
    result.eph_clock_bias = eph.clock;
    result.group_delay    = eph.group_delay;

    return true;
}

bool satellite_position(SatelliteId id, ts::Tai receive_time, Scalar pseudo_range,
                        ephemeris::Ephemeris const& ephemeris, RelativisticModel relativistic_model,
                        OrbitCorrection const* orbit_correction,
                        SatellitePosition&     result) NOEXCEPT {
    FUNCTION_SCOPEF("%s", id.name());

    // Compute satellite transmit time based on pseudorange. This is an estimation, as the pseudo
    // range contains errors. Assuming the error is <100m, this would give a transmit time error of
    // ~350ns. A satellite is moving roughly at 5km/s, so the time error would give us 350ns*5km =
    // 0.00175m = 0.175cm position difference, which is very small and can be neglected.
    auto t = receive_time;
    VERBOSEF("t_0: %s", t.rtklib_time_string().c_str());

    t.add_seconds(-pseudo_range / constant::c);
    VERBOSEF("t_p: %s (%.4fms, %.4fkm)", t.rtklib_time_string().c_str(),
             1000.0 * (pseudo_range / constant::c), pseudo_range / 1000.0);

    // Estimate satellite clock bias
    auto clock_bias = ephemeris.clock_bias(t);
    t.add_seconds(-clock_bias);
    VERBOSEF("t_c: %s (%.4fus, %.4fkm)", t.rtklib_time_string().c_str(), 1000000.0 * clock_bias,
             clock_bias * constant::c / 1000.0);

    // Compute satellite position and velocity
    auto eph = ephemeris.compute(t);

    auto rc = 0.0;
    switch (relativistic_model) {
    case RelativisticModel::None: rc = 0.0; break;
    case RelativisticModel::Broadcast: rc = eph.relativistic_correction_brdc; break;
    case RelativisticModel::Dotrv: rc = eph.relativistic_correction_dotrv; break;
    }

    result.id              = id;
    result.transmit_time   = t;
    result.receive_time    = receive_time;
    result.eph_position    = {eph.position.x, eph.position.y, eph.position.z};
    result.eph_velocity    = {eph.velocity.x, eph.velocity.y, eph.velocity.z};
    result.eph_clock_bias  = eph.clock + rc;
    result.group_delay     = 0.0;
    result.true_position   = {eph.position.x, eph.position.y, eph.position.z};
    result.true_velocity   = {eph.position.x, eph.position.y, eph.position.z};
    result.true_clock_bias = eph.clock + rc;

    if (orbit_correction) {
        result.true_position =
            orbit_correction->evaluate(t, result.eph_position, result.eph_velocity);
    }

    return true;
}

}  // namespace idokeido
