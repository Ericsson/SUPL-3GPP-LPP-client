#pragma once
#include <maths/float3.hpp>

#include <math.h>

namespace generator {
namespace tokoro {

Float3 ecef_to_wgs84(Float3 ecef) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator
