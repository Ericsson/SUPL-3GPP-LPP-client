#pragma once
#include <core/core.hpp>

#include <math.h>

struct Float3 {
    double x;
    double y;
    double z;

    NODISCARD double length_squared() const NOEXCEPT;
    NODISCARD double length() const NOEXCEPT;
    NODISCARD bool normalize() NOEXCEPT;
};

NODISCARD Float3 operator+(Float3 a, Float3 b) NOEXCEPT;
NODISCARD Float3 operator-(Float3 a, Float3 b) NOEXCEPT;
NODISCARD Float3 operator*(Float3 a, double b) NOEXCEPT;
NODISCARD Float3 operator/(Float3 a, double b) NOEXCEPT;
NODISCARD Float3 cross_product(Float3 a, Float3 b) NOEXCEPT;

