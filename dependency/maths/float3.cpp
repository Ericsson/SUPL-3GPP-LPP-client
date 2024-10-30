#include "float3.hpp"

double Float3::length_squared() const NOEXCEPT {
    return x * x + y * y + z * z;
}

double Float3::length() const NOEXCEPT {
    return std::sqrt(length_squared());
}

bool Float3::normalize() NOEXCEPT {
    auto l = length();
    if (l < 1E-12) {
        return false;
    }

    x /= l;
    y /= l;
    z /= l;
    return true;
}

Float3 operator+(Float3 a, Float3 b) NOEXCEPT {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Float3 operator-(Float3 a, Float3 b) NOEXCEPT {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Float3 operator*(Float3 a, double b) NOEXCEPT {
    return {a.x * b, a.y * b, a.z * b};
}

Float3 operator/(Float3 a, double b) NOEXCEPT {
    return {a.x / b, a.y / b, a.z / b};
}

Float3 cross_product(Float3 a, Float3 b) NOEXCEPT {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

double dot_product(Float3 a, Float3 b) NOEXCEPT {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
