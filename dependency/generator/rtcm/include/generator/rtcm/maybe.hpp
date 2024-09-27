#pragma once
#include <core/core.hpp>

template <typename T>
struct Maybe {
    T    value;
    bool valid;

    Maybe() : valid(false) {}
    Maybe(T const& new_value) : value(new_value), valid(true) {}
};
