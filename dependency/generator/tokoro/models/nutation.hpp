#pragma once
#include <maths/float3.hpp>

#include "astronomical_arguments.hpp"

namespace generator {
namespace tokoro {

struct Nutation {
    double d_psi;  // nutation in longitude
    double d_eps;  // nutation in obliquity

    /**
     * @brief Evaluates the nutation for a given time and astronomical arguments.
     *
     * This function calculates the nutation in longitude (d_psi) and obliquity (d_eps)
     * based on the provided time and astronomical arguments using the IAU 1980 nutation model.
     *
     * @param t The time in Julian centuries since J2000.0.
     * @param args The astronomical arguments required for the nutation calculation.
     * @return Nutation The calculated nutation values.
     */
    static Nutation evaluate(double t, AstronomicalArguments const& args) NOEXCEPT;
};

}  // namespace tokoro
}  // namespace generator
