#pragma once
#include <core/core.hpp>

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>
#include <time/tai.hpp>

#include <Eigen/Eigen>

namespace idokeido {

using Scalar = double;

using Vector3 = Eigen::Matrix<Scalar, 3, 1>;
using Vector4 = Eigen::Matrix<Scalar, 4, 1>;
using VectorX = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;

using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
using Matrix4 = Eigen::Matrix<Scalar, 4, 4>;
using MatrixX = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;

namespace constant {
static constexpr Scalar K_PI   = 3.1415926535897932;
static constexpr Scalar K_D2R  = K_PI / 180.0;
static constexpr Scalar K_R2D  = 180.0 / K_PI;
static constexpr Scalar K_R2SC = 1.0 / K_PI;
static constexpr Scalar K_SC2R = K_PI;
static constexpr Scalar K_C    = 299792458.0;
}  // namespace constant

inline Scalar geometric_distance(Vector3 const& a, Vector3 const& b) {
    auto delta    = a - b;
    auto distance = delta.norm();

    // correct for rotation ECEF
    auto dot_omega_e = 7.2921151467e-5;
    auto correction  = dot_omega_e * (a.x() * b.y() - a.y() * b.x()) / constant::K_C;

    return distance + correction;
}

struct RawMeasurement {
    ts::Tai time;

    SatelliteId satellite_id;
    SignalId    signal_id;

    double pseudo_range;   // meters
    double carrier_phase;  // cycles
    double doppler;        // Hz
    double snr;            // dBHz
    double lock_time;      // seconds
};

struct Solution {
    enum class Status {
        None,
        Standard,
    };

    ts::Tai time;
    Status  status;

    double latitude;
    double longitude;
    double altitude;

    size_t satellite_count;
};

enum class RelativisticModel {
    None,
    Broadcast,
    Dotrv,
};

enum class WeightFunction {
    None,
    Snr,
    Elevation,
    Variance,
};

enum class IonosphericMode {
    None,
    Broadcast,
    Dual,
    Ssr,
};

enum class EpochSelection {
    FirstObservation,
    LastObservation,
    MeanObservation,
};

}  // namespace idokeido
