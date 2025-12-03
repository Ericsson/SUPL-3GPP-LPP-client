#pragma once
#include "../frame/etrf.hpp"
#include "../frame/sweref99.hpp"
#include "../frames.hpp"
#include "../transform.hpp"

namespace coordinates {

template <>
struct Transform<ETRF2014, Sweref99> {
    static double apply_epoch(double, double) { return 1999.5; }
    static State  apply(State const& state, double target_epoch);
};

template <>
struct Transform<Sweref99, ETRF2014> {
    static double apply_epoch(double, double target_epoch) { return target_epoch; }
    static State  apply(State const& state, double target_epoch);
};

}  // namespace coordinates
