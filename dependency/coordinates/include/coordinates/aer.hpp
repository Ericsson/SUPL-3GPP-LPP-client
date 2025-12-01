#pragma once
#include <cmath>
#include "ecef.hpp"

namespace coordinates {

struct Aer {
    Vector3d value;

    double azimuth() const { return value.x(); }
    double elevation() const { return value.y(); }
    double range() const { return value.z(); }

    double& azimuth() { return value.x(); }
    double& elevation() { return value.y(); }
    double& range() { return value.z(); }

    double azimuth_deg() const { return value.x() * 180.0 / M_PI; }
    double elevation_deg() const { return value.y() * 180.0 / M_PI; }

    static Aer from_degrees(double az_deg, double el_deg, double range) {
        return Aer{Vector3d(az_deg * M_PI / 180.0, el_deg * M_PI / 180.0, range)};
    }
};

}  // namespace coordinates
