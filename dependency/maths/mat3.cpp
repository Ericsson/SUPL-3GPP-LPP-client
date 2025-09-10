#include "mat3.hpp"

#include <cmath>

Mat3 Mat3::identity() NOEXCEPT {
    return {
        1, 0, 0, 0, 1, 0, 0, 0, 1,
    };
}

Mat3 Mat3::rotate_x(double angle) NOEXCEPT {
    auto c = std::cos(angle);
    auto s = std::sin(angle);
    return {
        1, 0, 0, 0, c, -s, 0, s, c,
    };
}

Mat3 Mat3::rotate_y(double angle) NOEXCEPT {
    auto c = std::cos(angle);
    auto s = std::sin(angle);
    return {
        c, 0, s, 0, 1, 0, -s, 0, c,
    };
}

Mat3 Mat3::rotate_z(double angle) NOEXCEPT {
    auto c = std::cos(angle);
    auto s = std::sin(angle);
    return {
        c, -s, 0, s, c, 0, 0, 0, 1,
    };
}

#if 0
Mat3 operator*(Mat3 a, Mat3 b) NOEXCEPT {
    return {
        a.m[0] * b.m[0] + a.m[1] * b.m[3] + a.m[2] * b.m[6],
        a.m[0] * b.m[1] + a.m[1] * b.m[4] + a.m[2] * b.m[7],
        a.m[0] * b.m[2] + a.m[1] * b.m[5] + a.m[2] * b.m[8],

        a.m[3] * b.m[0] + a.m[4] * b.m[3] + a.m[5] * b.m[6],
        a.m[3] * b.m[1] + a.m[4] * b.m[4] + a.m[5] * b.m[7],
        a.m[3] * b.m[2] + a.m[4] * b.m[5] + a.m[5] * b.m[8],

        a.m[6] * b.m[0] + a.m[7] * b.m[3] + a.m[8] * b.m[6],
        a.m[6] * b.m[1] + a.m[7] * b.m[4] + a.m[8] * b.m[7],
        a.m[6] * b.m[2] + a.m[7] * b.m[5] + a.m[8] * b.m[8],
    };
}
#endif
#if 0
Mat3 operator*(Mat3 a, Mat3 b) NOEXCEPT {
    // Row-major order
    return {
        a.m[0] * b.m[0] + a.m[1] * b.m[3] + a.m[2] * b.m[6],
        a.m[0] * b.m[1] + a.m[1] * b.m[4] + a.m[2] * b.m[7],
        a.m[0] * b.m[2] + a.m[1] * b.m[5] + a.m[2] * b.m[8],

        a.m[3] * b.m[0] + a.m[4] * b.m[3] + a.m[5] * b.m[6],
        a.m[3] * b.m[1] + a.m[4] * b.m[4] + a.m[5] * b.m[7],
        a.m[3] * b.m[2] + a.m[4] * b.m[5] + a.m[5] * b.m[8],

        a.m[6] * b.m[0] + a.m[7] * b.m[3] + a.m[8] * b.m[6],
        a.m[6] * b.m[1] + a.m[7] * b.m[4] + a.m[8] * b.m[7],
        a.m[6] * b.m[2] + a.m[7] * b.m[5] + a.m[8] * b.m[8],
    };
}
#else
Mat3 operator*(Mat3 a, Mat3 b) NOEXCEPT {
    // Column-major order
    return {
        a.m[0] * b.m[0] + a.m[3] * b.m[1] + a.m[6] * b.m[2],
        a.m[1] * b.m[0] + a.m[4] * b.m[1] + a.m[7] * b.m[2],
        a.m[2] * b.m[0] + a.m[5] * b.m[1] + a.m[8] * b.m[2],

        a.m[0] * b.m[3] + a.m[3] * b.m[4] + a.m[6] * b.m[5],
        a.m[1] * b.m[3] + a.m[4] * b.m[4] + a.m[7] * b.m[5],
        a.m[2] * b.m[3] + a.m[5] * b.m[4] + a.m[8] * b.m[5],

        a.m[0] * b.m[6] + a.m[3] * b.m[7] + a.m[6] * b.m[8],
        a.m[1] * b.m[6] + a.m[4] * b.m[7] + a.m[7] * b.m[8],
        a.m[2] * b.m[6] + a.m[5] * b.m[7] + a.m[8] * b.m[8],
    };
}
#endif
#if 0
Float3 operator*(Mat3 a, Float3 b) NOEXCEPT {
    return {
        a.m[0] * b.x + a.m[1] * b.y + a.m[2] * b.z,
        a.m[3] * b.x + a.m[4] * b.y + a.m[5] * b.z,
        a.m[6] * b.x + a.m[7] * b.y + a.m[8] * b.z,
    };
}
#else
Float3 operator*(Mat3 a, Float3 b) NOEXCEPT {
    return {
        a.m[0] * b.x + a.m[3] * b.y + a.m[6] * b.z,
        a.m[1] * b.x + a.m[4] * b.y + a.m[7] * b.z,
        a.m[2] * b.x + a.m[5] * b.y + a.m[8] * b.z,
    };
}
#endif
