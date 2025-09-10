#include "earth_solid_tides.hpp"
#include "constant.hpp"
#include "coordinates/enu.hpp"
#include "models/sun_moon.hpp"
#include "satellite.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, est);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, est)

namespace generator {
namespace tokoro {

static bool compute_solid_tide_pole(ts::Tai const& time, Float3 const& up, Float3 const& body,
                                    double g_constant, Float3& pole) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    VERBOSEF("up:     (%+.4f, %+.4f, %+.4f)", up.x, up.y, up.z);
    VERBOSEF("body:   (%+.4f, %+.4f, %+.4f)", body.x, body.y, body.z);

    auto body_distance = body.length();
    auto body_unit     = body;
    if (!body_unit.normalize()) {
        VERBOSEF("failed to ...");
        return false;
    }

    VERBOSEF("body_unit:     (%+.4f, %+.4f, %+.4f)", body_unit.x, body_unit.y, body_unit.z);
    VERBOSEF("body_distance: %+.4f", body_distance);

    auto gm  = g_constant / constant::GME;
    auto re4 = constant::RE_WGS84 * constant::RE_WGS84 * constant::RE_WGS84 * constant::RE_WGS84;
    auto br3 = body_distance * body_distance * body_distance;
    auto k2  = gm * re4 / br3;
    VERBOSEF("k2: %+.14f", k2);

    auto h2 = 0.6078;
    auto l2 = 0.0847;
    VERBOSEF("h2: %+.14f", h2);
    VERBOSEF("l2: %+.14f", l2);

    auto a  = dot_product(body_unit, up);
    auto dp = k2 * 3.0 * l2 * a;
    auto du = k2 * (h2 * (1.5 * a * a - 0.5) - 3.0 * l2 * a * a);
    VERBOSEF("dp: %+.14f", dp);
    VERBOSEF("du: %+.14f", du);

    pole.x = dp * body_unit.x + du * up.x;
    pole.y = dp * body_unit.y + du * up.y;
    pole.z = dp * body_unit.z + du * up.z;

    VERBOSEF("pole: (%+.14f, %+.14f, %+.14f)", pole.x, pole.y, pole.z);
    return true;
}

EarthSolidTides model_earth_solid_tides(ts::Tai const& time, SatelliteState const& satellite,
                                        Float3 ground_position_ecef, Float3 ground_position_llh) {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    Float3 east{};
    Float3 north{};
    Float3 up{};
    enu_basis_from_xyz(ground_position_ecef, east, north, up);

    auto sm = sun_and_moon_position_ecef(time);

    Float3 sun_pole{};
    compute_solid_tide_pole(time, up, sm.sun, constant::SUN_GRAVITATIONAL_CONSTANT, sun_pole);

    Float3 moon_pole{};
    compute_solid_tide_pole(time, up, sm.moon, constant::MOON_GRAVITATIONAL_CONSTANT, moon_pole);

    auto sin2l    = std::sin(2.0 * ground_position_llh.x);
    auto delta_up = -0.012 * sin2l * std::sin(sm.gmst + ground_position_llh.y);
    VERBOSEF("ground: %+.4f, %+.4f, %+.4f", ground_position_llh.x, ground_position_llh.y,
             ground_position_llh.z);
    VERBOSEF("sin2l: %+.14f", sin2l);
    VERBOSEF("delta_up: %+.14f", delta_up);

    EarthSolidTides result{};
    result.displacement_vector = sun_pole + moon_pole + delta_up * up;
    result.displacement = -dot_product(satellite.true_line_of_sight, result.displacement_vector);
    result.valid        = true;
    return result;
}

}  // namespace tokoro
}  // namespace generator
