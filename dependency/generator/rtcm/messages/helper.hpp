#pragma once
#include <cmath>
#include "encoder.hpp"

// #define ROUND(x) (floor((x) + 0.5))

template <typename T>
static inline T rtcm_round(T value) {
    return floor(value + 0.5);
}

// Quantizes `value` to an unsigned field of `bits` bits with resolution `scale`. Rounds to the
// nearest step (truncation would turn exact multiples such as 29*0.5 into 28 through binary
// rounding error) and saturates instead of wrapping on out-of-range or non-finite input.
static inline uint32_t rtcm_quantize_unsigned(double value, double scale, unsigned bits) {
    auto max_value = static_cast<double>((uint64_t{1} << bits) - 1);
    if (!std::isfinite(value)) return 0;
    auto steps = rtcm_round(value / scale);
    if (steps <= 0.0) return 0;
    if (steps >= max_value) return static_cast<uint32_t>(max_value);
    return static_cast<uint32_t>(steps);
}

// Signed counterpart; the most negative value of the field is reserved and never produced.
static inline int32_t rtcm_quantize_signed(double value, double scale, unsigned bits) {
    auto max_value = static_cast<double>((int64_t{1} << (bits - 1)) - 1);
    if (!std::isfinite(value)) return 0;
    auto steps = rtcm_round(value / scale);
    if (steps >= max_value) return static_cast<int32_t>(max_value);
    if (steps <= -max_value) return static_cast<int32_t>(-max_value);
    return static_cast<int32_t>(steps);
}

// The residual messages (MT1030/1031/1303/1304) carry the satellite count in a 5-bit field.
static constexpr int RTCM_RESIDUAL_MAX_SATELLITES = 31;
