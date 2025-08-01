#pragma once
#include "idokeido.hpp"

namespace idokeido {

// Construct a local tangental coordinate system from a geodetic position
void enu_basis_from_llh(Vector3 const& llh, Vector3& east, Vector3& north, Vector3& up) NOEXCEPT;

// Construct a local tangental coordinate system from an ECEF position (assumes spherical Earth)
void enu_basis_from_xyz(Vector3 const& location, Vector3& east, Vector3& north,
                        Vector3& up) NOEXCEPT;

// Transform an ECEF vector to an ENU vector at a geodetic position
Vector3 ecef_to_enu_at_llh(Vector3 const& llh, Vector3 const& ecef_vector);

}  // namespace idokeido
