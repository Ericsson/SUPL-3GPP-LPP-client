#include "kalman.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, kalman);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, kalman)

namespace idokeido {

KalmanFilter::KalmanFilter() NOEXCEPT {
    mX = VectorX{};
    mP = MatrixX{};
}

void KalmanFilter::initialize(VectorX const& x0, MatrixX const& P0) NOEXCEPT {
    mX = x0;
    mP = P0;
}

void KalmanFilter::predict(MatrixX const& F, MatrixX const& Q) NOEXCEPT {
    mX = F * mX;
    mP = F * mP * F.transpose() + Q;
}

void KalmanFilter::update(MatrixX const& H, MatrixX const& R, VectorX const& z) NOEXCEPT {
    auto S = (H * mP * H.transpose() + R).eval();
    auto K = (mP * H.transpose() * S.ldlt().solve(MatrixX::Identity(S.rows(), S.cols()))).eval();
    mX     = mX + K * z;
    auto I = MatrixX::Identity(mX.rows(), mX.rows());
    mP     = (I - K * H) * mP;
}

void KalmanFilter::add_state(Scalar x0, Scalar p0) NOEXCEPT {
    auto n = mX.rows();
    mX.conservativeResize(n + 1);
    mX(n) = x0;

    mP.conservativeResize(n + 1, n + 1);
    mP.row(n).setZero();
    mP.col(n).setZero();
    mP(n, n) = p0;
}

void KalmanFilter::remove_state(long idx) NOEXCEPT {
    auto n = mX.rows();
    CORE_ASSERT(idx >= 0 && idx < n, "state index out of range");

    // Shift rows/cols left/up past idx
    if (idx < n - 1) {
        mX.segment(idx, n - 1 - idx)     = mX.segment(idx + 1, n - 1 - idx);
        mP.block(idx, 0, n - 1 - idx, n) = mP.block(idx + 1, 0, n - 1 - idx, n);
        mP.block(0, idx, n, n - 1 - idx) = mP.block(0, idx + 1, n, n - 1 - idx);
    }

    mX.conservativeResize(n - 1);
    mP.conservativeResize(n - 1, n - 1);
}

}  // namespace idokeido
