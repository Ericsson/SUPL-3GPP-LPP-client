#include "eph.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, sat);
LOGLET_MODULE3(idokeido, sat, pos);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(idokeido, sat, pos)

namespace idokeido {

void Satellite::compute_position_and_velocity(EphemerisEngine const& ephemeris) NOEXCEPT {
    FUNCTION_SCOPE();

    // Compute satellite transmit time based on pseudorange. This is an estimation, as the pseudo
    // range contains errors. Assuming the error is <100m, this would give a transmit time error of
    // ~350ns. A satellite is moving roughly at 5km/s, so the time error would give us 350ns*5km =
    // 0.00175m = 0.175cm position difference, which is very small and can be neglected.
    auto t = receive_time;
    DEBUGF("t_0: %s", t.rtklib_time_string().c_str());

    t.add_seconds(-pseudo_range / constant::c);
    DEBUGF("t_p: %s (%.4fus, %.4fkm)", t.rtklib_time_string().c_str(),
           1000000.0 * (pseudo_range / constant::c), pseudo_range / 1000.0);

    // Estimate satellite clock bias
    clock_bias = ephemeris.clock_bias(id, t);
    t.add_seconds(-clock_bias);
    DEBUGF("t_c: %s (%.4fus, %.4fkm)", t.rtklib_time_string().c_str(), 1000000.0 * clock_bias,
           clock_bias * constant::c);

    // Compute satellite position and velocity
    auto result             = ephemeris.evaluate(id, t);
    position                = result.position;
    velocity                = result.velocity;
    clock_bias              = result.clock;
    relativistic_correction = result.relativistic_correction;
    group_delay             = result.group_delay;
}

}  // namespace idokeido
