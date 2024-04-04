#pragma once
#include "types.hpp"

template <typename T>
struct Maybe {
    T    value;
    bool valid;

    Maybe() : valid(false) {}
    Maybe(T const& new_value) : value(new_value), valid(true) {}
};
