#include "look_angles.hpp"
#include "reference_ellipsoid.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, look_angles);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, look_angles)

namespace idokeido {

bool compute_look_angles(Vector3 const& ground_ecef, Vector3 const& enu,
                         Vector3 const& satellite_ecef, LookAngles& look_angles) NOEXCEPT {
    FUNCTION_SCOPE();
    VERBOSEF("ground_ecef:    %+.4f %+.4f %+.4f", ground_ecef.x(), ground_ecef.y(),
             ground_ecef.z());
    VERBOSEF("satellite_ecef: %+.4f %+.4f %+.4f", satellite_ecef.x(), satellite_ecef.y(),
             satellite_ecef.z());
    VERBOSEF("enu:            %+.4f %+.4f %+.4f", enu.x(), enu.y(), enu.z());

    if (ground_ecef.norm() >= ellipsoid::WGS84.semi_minor_axis) {
        look_angles.azimuth = 0.0;
        if (enu.squaredNorm() >= 1e-12) {
            look_angles.azimuth = std::atan2(enu.x(), enu.y());
        }

        if (look_angles.azimuth < 0) {
            look_angles.azimuth += 2 * constant::K_PI;
        }

        look_angles.elevation = std::asin(enu.z());

        Vector3 satellite_to_earth    = -satellite_ecef;
        Vector3 satellite_to_receiver = ground_ecef - satellite_ecef;
        satellite_to_earth.normalize();
        satellite_to_receiver.normalize();

        look_angles.nadir = std::acos(satellite_to_earth.dot(satellite_to_receiver));
    } else {
        look_angles.azimuth   = 0.0;
        look_angles.elevation = constant::K_PI / 2.0;
        look_angles.nadir     = 0.0;
    }

    VERBOSEF("azimuth=%+.4f, elevation=%+.4f, nadir=%+.4f", look_angles.azimuth * constant::K_R2D,
             look_angles.elevation * constant::K_R2D, look_angles.nadir * constant::K_R2D);
    return true;
}

}  // namespace idokeido
