#pragma once
#include <generator/idokeido/idokeido.hpp>

namespace idokeido {

class KalmanFilter {
public:
    KalmanFilter() NOEXCEPT;

    void initialize(VectorX const& x0, MatrixX const& P0) NOEXCEPT;

    void predict(MatrixX const& F, MatrixX const& Q) NOEXCEPT;
    void update(MatrixX const& H, MatrixX const& R, VectorX const& z) NOEXCEPT;

    void add_state(Scalar x0, Scalar p0) NOEXCEPT;
    void remove_state(long idx) NOEXCEPT;

    NODISCARD long           size() const NOEXCEPT { return mX.rows(); }
    NODISCARD VectorX const& state() const NOEXCEPT { return mX; }
    NODISCARD MatrixX const& covariance() const NOEXCEPT { return mP; }
    NODISCARD MatrixX&       covariance() NOEXCEPT { return mP; }
    NODISCARD Scalar         state(long idx) const NOEXCEPT { return mX(idx); }
    NODISCARD Scalar&        state(long idx) NOEXCEPT { return mX(idx); }

private:
    VectorX mX;
    MatrixX mP;
};

}  // namespace idokeido
