#pragma once
#include <core/core.hpp>
#include <msgpack/msgpack.hpp>

struct Float3 {
    double x;
    double y;
    double z;

    NODISCARD double length_squared() const NOEXCEPT;
    NODISCARD double length() const NOEXCEPT;
    NODISCARD bool   normalize() NOEXCEPT;

    MSGPACK_DEFINE(x, y, z)
};

NODISCARD Float3 operator+(Float3 a, Float3 b) NOEXCEPT;
NODISCARD Float3 operator-(Float3 a, Float3 b) NOEXCEPT;
NODISCARD Float3 operator*(Float3 a, double b) NOEXCEPT;
NODISCARD Float3 operator*(double a, Float3 b) NOEXCEPT;
NODISCARD Float3 operator/(Float3 a, double b) NOEXCEPT;
NODISCARD Float3 cross_product(Float3 a, Float3 b) NOEXCEPT;
NODISCARD double dot_product(Float3 a, Float3 b) NOEXCEPT;

NODISCARD inline Float3 operator-(Float3 a) NOEXCEPT {
    return Float3{-a.x, -a.y, -a.z};
}
