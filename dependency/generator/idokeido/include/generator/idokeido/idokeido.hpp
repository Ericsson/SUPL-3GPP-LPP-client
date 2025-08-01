#pragma once
#include <core/core.hpp>

#include <Eigen/Eigen>

namespace idokeido {

using Scalar = double;

using Vector3 = Eigen::Matrix<Scalar, 3, 1>;
using Vector4 = Eigen::Matrix<Scalar, 4, 1>;

using MatrixX = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;

namespace constant {
static constexpr Scalar pi = 3.1415926535897932;
static constexpr Scalar d2r = pi / 180.0;
static constexpr Scalar r2d = 180.0 / pi;
static constexpr Scalar r2sc = 1.0 / pi;
static constexpr Scalar sc2r = pi;
static constexpr Scalar c = 299792458.0;
}

}  // namespace idokeido

