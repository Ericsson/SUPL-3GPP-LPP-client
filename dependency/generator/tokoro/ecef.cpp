#include "ecef.hpp"
#include "constant.hpp"

#include <math.h>

#include <maths/float3.hpp>

namespace generator {
namespace tokoro {
using EcefPosition = Float3;

bool ecef_to_enu(Float3 llh, Float3 ecef_vector, Float3& enu) {
    auto sinp = std::sin(llh.x * constant::DEG2RAD);
    auto cosp = std::cos(llh.x * constant::DEG2RAD);
    auto sinl = std::sin(llh.y * constant::DEG2RAD);
    auto cosl = std::cos(llh.y * constant::DEG2RAD);

    auto x = -sinl * ecef_vector.x + cosl * ecef_vector.y;
    auto y = -sinp * cosl * ecef_vector.x - sinp * sinl * ecef_vector.y + cosp * ecef_vector.z;
    auto z = cosp * cosl * ecef_vector.x + cosp * sinl * ecef_vector.y + sinp * ecef_vector.z;

    enu.x = x;
    enu.y = y;
    enu.z = z;
    return true;
}

}  // namespace tokoro
}  // namespace generator
