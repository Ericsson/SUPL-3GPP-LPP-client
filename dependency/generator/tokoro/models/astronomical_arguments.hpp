#pragma once
#include <maths/float3.hpp>

namespace generator {
namespace tokoro {

struct AstronomicalArguments {
    double l;      // mean anomaly of the Moon
    double lp;     // mean anomaly of the Sun
    double f;      // mean argument of the latitude of the Moon
    double d;      // mean elongation of the Moon from the Sun
    double omega;  // mean longitude of the ascending node of the Moon

    static AstronomicalArguments evaluate(double t_jc) NOEXCEPT;
};

}  // namespace tokoro
}  // namespace generator
