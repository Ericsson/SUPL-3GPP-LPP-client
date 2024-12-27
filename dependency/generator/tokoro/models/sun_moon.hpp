#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct SunMoonPosition {
    Float3 sun;
    Float3 moon;
    double gmst;
};

SunMoonPosition sun_and_moon_position_ecef(ts::Tai const& time) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator
