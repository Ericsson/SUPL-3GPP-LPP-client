#pragma once
#include <cmath>
#include "encoder.hpp"

// #define ROUND(x) (floor((x) + 0.5))

template <typename T>
static inline T rtcm_round(T value) {
    return floor(value + 0.5);
}
