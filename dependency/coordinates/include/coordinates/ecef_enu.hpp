#pragma once
#include <type_traits>
#include "ecef.hpp"
#include "ellipsoid.hpp"
#include "enu.hpp"
#include "frame.hpp"
#include "llh.hpp"

namespace coordinates {

Eigen::Matrix3d enu_rotation_matrix(double lat, double lon);

Vector3d ecef_to_enu_impl(Vector3d const& ecef, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid);
Vector3d enu_to_ecef_impl(Vector3d const& enu, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid);

template <typename Frame>
Enu<Frame> ecef_to_enu(Ecef<Frame> const& ecef, Llh<Frame> const& origin_llh) {
    static_assert(!std::is_same_v<Frame, NullReferenceFrame>,
                  "ecef_to_enu requires a reference frame with ellipsoid data");
    return Enu<Frame>{ecef_to_enu_impl(ecef.value, origin_llh.value, FrameTrait<Frame>::ellipsoid)};
}

template <typename Frame>
Ecef<Frame> enu_to_ecef(Enu<Frame> const& enu, Llh<Frame> const& origin_llh) {
    static_assert(!std::is_same_v<Frame, NullReferenceFrame>,
                  "enu_to_ecef requires a reference frame with ellipsoid data");
    return Ecef<Frame>{enu_to_ecef_impl(enu.value, origin_llh.value, FrameTrait<Frame>::ellipsoid)};
}

}  // namespace coordinates
