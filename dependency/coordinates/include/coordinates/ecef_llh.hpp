#pragma once
#include <type_traits>
#include "ecef.hpp"
#include "ellipsoid.hpp"
#include "frame.hpp"
#include "llh.hpp"

namespace coordinates {

Vector3d ecef_to_llh_impl(Vector3d const& ecef, Ellipsoid const& e);
Vector3d llh_to_ecef_impl(Vector3d const& llh, Ellipsoid const& e);

template <typename Frame>
Llh<Frame> ecef_to_llh(Ecef<Frame> const& ecef) {
    static_assert(!std::is_same_v<Frame, NullReferenceFrame>,
                  "ecef_to_llh requires a reference frame with ellipsoid data");
    return Llh<Frame>{ecef_to_llh_impl(ecef.value, FrameTrait<Frame>::ellipsoid)};
}

template <typename Frame>
Ecef<Frame> llh_to_ecef(Llh<Frame> const& llh) {
    static_assert(!std::is_same_v<Frame, NullReferenceFrame>,
                  "llh_to_ecef requires a reference frame with ellipsoid data");
    return Ecef<Frame>{llh_to_ecef_impl(llh.value, FrameTrait<Frame>::ellipsoid)};
}

}  // namespace coordinates
