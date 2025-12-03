#pragma once
#include <cmath>
#include "ecef.hpp"

namespace coordinates {

template <typename Frame = NullReferenceFrame>
struct Llh {
    Vector3d value;

    double latitude() const { return value.x(); }
    double longitude() const { return value.y(); }
    double height() const { return value.z(); }

    double& latitude() { return value.x(); }
    double& longitude() { return value.y(); }
    double& height() { return value.z(); }

    double latitude_deg() const { return value.x() * 180.0 / M_PI; }
    double longitude_deg() const { return value.y() * 180.0 / M_PI; }

    static Llh from_degrees(double lat_deg, double lon_deg, double height) {
        return Llh{Vector3d(lat_deg * M_PI / 180.0, lon_deg * M_PI / 180.0, height)};
    }

    Llh<NullReferenceFrame> to_any() const { return Llh<NullReferenceFrame>{value}; }

    static Llh from_any(Llh<NullReferenceFrame> const& llh) { return Llh{llh.value}; }
};

}  // namespace coordinates
