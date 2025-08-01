#include "satellite.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE3(idokeido, sat, pos);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(idokeido, sat, pos)

namespace idokeido {

static void satellite_position() {}

void Satellite::compute_position_and_velocity() NOEXCEPT {
    FUNCTION_SCOPE();

    auto t = receive_time;
    t -= ts::Timestamp{pseudo_range

}

}  // namespace idokeido
