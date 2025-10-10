#pragma once
#include "idokeido.hpp"

#include <vector>

namespace idokeido {

struct IdwPoint {
    Vector3 position;
    Scalar  residual;
    Scalar  distance;
};

inline Scalar inverse_distance_weight(Vector3 const&,
                                      std::vector<IdwPoint> const& residuals) NOEXCEPT {
    Scalar sum = 0;
    for (auto const& r : residuals) {
        sum += r.residual / r.distance;
    }
    return 1 / sum;
}

}  // namespace idokeido
