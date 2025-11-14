#include "correction.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

#ifdef DATA_TRACING
#include <datatrace/datatrace.hpp>
#endif

LOGLET_MODULE2(idokeido, clock);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, clock)

namespace idokeido {

Scalar ClockCorrection::evaluate(ts::Tai const& time) const NOEXCEPT {
    FUNCTION_SCOPE();

    VERBOSEF("t:   %s", ts::Utc{time}.rtklib_time_string().c_str());
    VERBOSEF("t0:  %s", ts::Utc{reference_time}.rtklib_time_string().c_str());

    auto t_k = ts::Gps{time}.difference(ts::Gps{reference_time}).full_seconds();
    VERBOSEF("t_k: %+.14f", t_k);

    VERBOSEF("c:      %+24.14f, %+24.14f, %+24.14f", c0, c1, c2);

    auto r0 = c0;
    auto r1 = c1 * t_k;
    auto r2 = c2 * t_k * t_k;
    VERBOSEF("parts:  %+24.14f, %+24.14f, %+24.14f", r0, r1, r2);

    auto result = r0 + r1 + r2;
    VERBOSEF("result: %+24.14f", result);
    return result;
}

}  // namespace idokeido
