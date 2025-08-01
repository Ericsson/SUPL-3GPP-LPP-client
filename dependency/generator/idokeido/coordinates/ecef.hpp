#pragma once
#include "idokeido.hpp"
#include "reference_ellipsoid.hpp"

namespace idokeido {

Vector3 ecef_to_llh(Vector3 ecef, ReferenceEllipsoid const& ellipsoid);
Vector3 llh_to_ecef(Vector3 llh, ReferenceEllipsoid const& ellipsoid);

}  // namespace idokeido
