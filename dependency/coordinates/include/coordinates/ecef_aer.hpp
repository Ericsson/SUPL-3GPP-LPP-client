#pragma once
#include <type_traits>
#include "aer.hpp"
#include "ecef.hpp"
#include "ellipsoid.hpp"
#include "frame.hpp"
#include "llh.hpp"

namespace coordinates {

Vector3d ecef_to_aer_impl(Vector3d const& ecef, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid);
Vector3d aer_to_ecef_impl(Vector3d const& aer, Vector3d const& origin_llh,
                          Ellipsoid const& ellipsoid);

template <typename Frame>
Aer ecef_to_aer(Ecef<Frame> const& ecef, Llh<Frame> const& origin_llh) {
    static_assert(!std::is_same_v<Frame, NullReferenceFrame>,
                  "ecef_to_aer requires a reference frame with ellipsoid data");
    return Aer{ecef_to_aer_impl(ecef.value, origin_llh.value, FrameTrait<Frame>::ellipsoid)};
}

template <typename Frame>
Ecef<Frame> aer_to_ecef(Aer const& aer, Llh<Frame> const& origin_llh) {
    static_assert(!std::is_same_v<Frame, NullReferenceFrame>,
                  "aer_to_ecef requires a reference frame with ellipsoid data");
    return Ecef<Frame>{aer_to_ecef_impl(aer.value, origin_llh.value, FrameTrait<Frame>::ellipsoid)};
}

}  // namespace coordinates
