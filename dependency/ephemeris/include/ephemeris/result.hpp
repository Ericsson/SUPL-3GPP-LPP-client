#pragma once
#include <core/core.hpp>

#include <maths/float3.hpp>

namespace ephemeris {

struct EphemerisResult {
    Float3 position;
    Float3 velocity;
    double clock;
};

}  // namespace ephemeris
