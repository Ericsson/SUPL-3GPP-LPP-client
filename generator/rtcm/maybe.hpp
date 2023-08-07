#pragma once
#include "types.hpp"

template <typename T>
struct Maybe {
    T    value;
    bool valid;

    Maybe() : valid(false) {}
    Maybe(const T& value) : value(value), valid(true) {}
};
