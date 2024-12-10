#pragma once
#include "constant.hpp"

#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

inline double geometric_distance(Float3 a, Float3 b, Float3* line_of_sight = nullptr) {
    auto delta    = a - b;
    auto distance = delta.length();

    // correct for rotation ECEF
    auto dot_omega_e = 7.2921151467e-5;
    auto correction  = dot_omega_e * (a.x * b.y - a.y * b.x) / constant::SPEED_OF_LIGHT;

    if (line_of_sight) {
        *line_of_sight = delta / distance;
    }

    return distance + correction;
}

inline double geocentric_distance(Float3 a) {
    auto delta    = a;
    auto distance = delta.length();
    return distance;
}

struct RangeTimeDivision {
    int32_t integer_ms;
    double  rough_range;
    double  used_range;
    double  unused_range;
};

inline RangeTimeDivision range_time_division(double range) {
#define RTCM_CALC_P2_N(N) (static_cast<double>((static_cast<uint64_t>(1) << (N))))
#define RTCM_CALC_N2_N(N) (1.0 / RTCM_CALC_P2_N(N))
#define RTCM_N2_10 RTCM_CALC_N2_N(10)
    auto time_range           = range * 1.0e3 / constant::SPEED_OF_LIGHT;
    auto integer_ms           = static_cast<int32_t>(time_range);
    auto converted_integer_ms = static_cast<double>(integer_ms);
    time_range -= converted_integer_ms;

    auto rough_range           = static_cast<long>(floor((time_range / RTCM_N2_10) + 0.5));
    if(rough_range >= 1024) {
        // if the range is close to the next integer millisecond (79.9999), then there are cases
        // where integer ms is 79 and rough range is 1.0, which is not valid. In this case, we
        // should adjust the integer ms and rough range.
        integer_ms += 1;
        converted_integer_ms += 1.0;
        time_range -= 1.0;
        rough_range = static_cast<long>(floor((time_range / RTCM_N2_10) + 0.5));
    }

    auto converted_rough_range = static_cast<double>(rough_range) * RTCM_N2_10;
    time_range -= converted_rough_range;

    return {
        integer_ms,
        converted_rough_range,
        converted_integer_ms + converted_rough_range,
        time_range,
    };
}

struct HydrostaticAndWetMapping {
    double hydrostatic;
    double wet;
};

HydrostaticAndWetMapping hydrostatic_mapping_function(ts::Tai time, Float3 position,
                                                      double elevation);

}  // namespace tokoro
}  // namespace generator
