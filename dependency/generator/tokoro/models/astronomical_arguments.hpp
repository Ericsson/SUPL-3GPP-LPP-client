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

    /**
     * @brief Evaluates the astronomical arguments at a given time.
     *
     * This function calculates the astronomical arguments (l, lp, f, d, omega)
     * based on the provided time parameter t, which is in Julian centuries since J2000.
     * The calculations involve polynomial expressions of t and the results are converted
     * to radians and normalized to the range [0, 2π).
     *
     * @param t The time in Julian centuries since J2000.0.
     * @return An AstronomicalArguments object containing the evaluated arguments.
     */
    static AstronomicalArguments evaluate(double t_jc) NOEXCEPT;
};

}  // namespace tokoro
}  // namespace generator
