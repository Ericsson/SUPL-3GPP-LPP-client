#pragma once
#include <core/core.hpp>
#include <maths/float3.hpp>

#include <math.h>

struct Mat3 {
    double m[9];

    static Mat3 identity() NOEXCEPT;
    static Mat3 rotate_x(double angle) NOEXCEPT;
    static Mat3 rotate_y(double angle) NOEXCEPT;
    static Mat3 rotate_z(double angle) NOEXCEPT;
};

NODISCARD Mat3   operator*(Mat3 a, Mat3 b) NOEXCEPT;
NODISCARD Float3 operator*(Mat3 a, Float3 b) NOEXCEPT;
