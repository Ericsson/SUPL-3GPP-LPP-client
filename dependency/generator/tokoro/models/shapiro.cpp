#include "shapiro.hpp"
#include "models/helper.hpp"
#include "satellite.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, shapiro);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, shapiro)

namespace generator {
namespace tokoro {

/**
 * @brief Computes the Shapiro delay for a given satellite and ground position.
 *
 * This function calculates the relativistic path range effect, also known as the Shapiro signal
 * propagation delay, which is the time delay experienced by a signal as it travels through the
 * curved spacetime caused by the Earth's gravitational field.
 *
 * @param satellite The satellite's state, including its true position and range.
 * @param ground_position The ground position in a 3D coordinate system.
 * @return Shapiro The computed Shapiro delay and a boolean indicating success.
 *
 * The calculation is based on the formula provided by the ESA's Navipedia:
 * https://gssc.esa.int/navipedia/index.php/Relativistic_Path_Range_Effect
 */
Shapiro model_shapiro(SatelliteState const& satellite, Float3 ground_position) {
    VSCOPE_FUNCTION();

    auto r_sat = geocentric_distance(satellite.true_position);
    auto r_rcv = geocentric_distance(ground_position);
    auto r     = satellite.true_range;

    auto shapiro = (2 * constant::WGS84_EARTH_GRAVITATIONAL_CONSTANT /
                    (constant::SPEED_OF_LIGHT * constant::SPEED_OF_LIGHT)) *
                   log((r_sat + r_rcv + r) / (r_sat + r_rcv - r));
    return {shapiro, true};
}

}  // namespace tokoro
}  // namespace generator
