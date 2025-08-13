#include "correction.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE2(idokeido, orbit);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, orbit)

namespace idokeido {

Vector3 OrbitCorrection::evaluate(ts::Tai const& time, Vector3 const& eph_position,
                                  Vector3 const& eph_velocity) const NOEXCEPT {
    FUNCTION_SCOPE();

    auto e_along = eph_velocity;
    e_along.normalize();
    VERBOSEF("e_along:   %+24.14f, %+24.14f, %+24.14f", e_along.x(), e_along.y(), e_along.z());

    auto e_cross = eph_position.cross(eph_velocity);
    e_cross.normalize();
    VERBOSEF("e_cross:   %+24.14f, %+24.14f, %+24.14f", e_cross.x(), e_cross.y(), e_cross.z());

    auto e_radial = e_along.cross(e_cross);
    VERBOSEF("e_radial:  %+24.14f, %+24.14f, %+24.14f", e_radial.x(), e_radial.y(), e_radial.z());

    VERBOSEF("t:   %s", time.rtklib_time_string().c_str());
    VERBOSEF("t0:  %s", reference_time.rtklib_time_string().c_str());

    auto t_k = time.difference_seconds(reference_time);
    VERBOSEF("t_k: %+.14f", t_k);

    auto delta_at = delta + dot_delta * t_k;
    VERBOSEF("delta:     %+24.14f, %+24.14f, %+24.14f", delta.x(), delta.y(), delta.z());
    VERBOSEF("dot_delta: %+24.14f, %+24.14f, %+24.14f", dot_delta.x(), dot_delta.y(),
             dot_delta.z());
    VERBOSEF("delta_at:  %+24.14f, %+24.14f, %+24.14f", delta_at.x(), delta_at.y(), delta_at.z());

    auto x = e_radial.x() * delta_at.x() + e_along.x() * delta_at.y() + e_cross.x() * delta_at.z();
    auto y = e_radial.y() * delta_at.x() + e_along.y() * delta_at.y() + e_cross.y() * delta_at.z();
    auto z = e_radial.z() * delta_at.x() + e_along.z() * delta_at.y() + e_cross.z() * delta_at.z();

    VERBOSEF("result:    %+24.14f, %+24.14f, %+24.14f", x, y, z);
    auto result = Vector3{x, y, z};
    return eph_position - result;
}

}  // namespace idokeido
