#pragma once
#include <generator/idokeido/idokeido.hpp>

#include <time/tai.hpp>

namespace idokeido {

struct KlobucharModelParameters {
    Scalar a[4];
    Scalar b[4];

    Scalar evaluate(ts::Tai const& time, Scalar elevation, Scalar azimuth,
                                   Vector3 const& llh) const NOEXCEPT;
};

}  // namespace idokeido
