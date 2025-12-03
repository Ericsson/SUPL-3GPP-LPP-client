#pragma once
#include <type_traits>
#include "ecef.hpp"
#include "ellipsoid.hpp"
#include "frame.hpp"
#include "llh.hpp"
#include "ned.hpp"

namespace coordinates {

Vector3d ecef_to_ned_impl(Vector3d const& ecef, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid);
Vector3d ned_to_ecef_impl(Vector3d const& ned, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid);

template <typename Frame>
Ned<Frame> ecef_to_ned(Ecef<Frame> const& ecef, Llh<Frame> const& origin_llh) {
    static_assert(!std::is_same<Frame, NullReferenceFrame>::value,
                  "ecef_to_ned requires a reference frame with ellipsoid data");
    return Ned<Frame>{ecef_to_ned_impl(ecef.value, origin_llh.value, FrameTrait<Frame>::ellipsoid)};
}

template <typename Frame>
Ecef<Frame> ned_to_ecef(Ned<Frame> const& ned, Llh<Frame> const& origin_llh) {
    static_assert(!std::is_same<Frame, NullReferenceFrame>::value,
                  "ned_to_ecef requires a reference frame with ellipsoid data");
    return Ecef<Frame>{ned_to_ecef_impl(ned.value, origin_llh.value, FrameTrait<Frame>::ellipsoid)};
}

}  // namespace coordinates
