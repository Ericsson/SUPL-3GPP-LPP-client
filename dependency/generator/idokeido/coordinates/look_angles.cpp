#include "look_angles.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, look_angles);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, look_angles)

namespace idokeido {

bool compute_look_angles(Vector3 const& ground_ecef, Vector3 const& enu,
                         Vector3 const& satellite_ecef, LookAngles& look_angles) NOEXCEPT {
    FUNCTION_SCOPE();
    VERBOSEF("ground_ecef: %+.4f, %+.4f, %+.4f", ground_ecef.x(), ground_ecef.y(), ground_ecef.z());
    VERBOSEF("satellite_ecef: %+.4f, %+.4f, %+.4f", satellite_ecef.x(), satellite_ecef.y(),
             satellite_ecef.z());
    VERBOSEF("enu:         %+.4f, %+.4f, %+.4f", enu.x(), enu.y(), enu.z());

    auto azimuth = 0.0;
    if (enu.squaredNorm() >= 1e-12) {
        azimuth = std::atan2(enu.x(), enu.y());
    }

    if (azimuth < 0) {
        azimuth += 2 * constant::pi;
    }

    auto elevation = std::asin(enu.z());

    Vector3 satellite_to_earth    = -satellite_ecef;
    Vector3 satellite_to_receiver = ground_ecef - satellite_ecef;
    satellite_to_earth.normalize();
    satellite_to_receiver.normalize();

    auto nadir = std::acos(satellite_to_earth.dot(satellite_to_receiver));

    look_angles.azimuth   = azimuth;
    look_angles.elevation = elevation;
    look_angles.nadir     = nadir;

    VERBOSEF("azimuth=%+.4f, elevation=%+.4f, nadir=%+.4f", look_angles.azimuth * constant::r2d,
             look_angles.elevation * constant::r2d, look_angles.nadir * constant::r2d);
    return true;
}

}  // namespace idokeido
