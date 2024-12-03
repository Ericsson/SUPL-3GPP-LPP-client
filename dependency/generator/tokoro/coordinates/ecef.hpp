#pragma once
#include <maths/float3.hpp>

#include <math.h>

namespace generator {
namespace tokoro {

// Transform ecef vector to local tangental coordinate
bool ecef_to_enu(Float3 llh, Float3 ecef_vector, Float3& enu);

}  // namespace tokoro
}  // namespace generator
