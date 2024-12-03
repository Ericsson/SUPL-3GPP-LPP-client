#pragma once
#include <maths/float3.hpp>

#include <math.h>

namespace generator {
namespace tokoro {

// Construct a local tangental coordinate system from a geodetic position
void enu_basis_from_llh(Float3 llh, Float3& east, Float3& north, Float3& up) NOEXCEPT;

// Construct a local tangental coordinate system from an ECEF position (assumes spherical Earth)
void enu_basis_from_xyz(Float3 location, Float3& east, Float3& north, Float3& up) NOEXCEPT;

// Transform an ECEF vector to an ENU vector at a geodetic position
Float3 ecef_to_enu_at_llh(Float3 llh, Float3 ecef_vector);

}  // namespace tokoro
}  // namespace generator
