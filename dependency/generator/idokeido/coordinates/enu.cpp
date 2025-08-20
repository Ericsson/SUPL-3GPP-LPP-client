#include "enu.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(idokeido, enu);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(idokeido, enu)

namespace idokeido {

void enu_basis_from_llh(Vector3 const& llh, Vector3& east, Vector3& north, Vector3& up) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.6f, %+.6f, %+.2f", llh.x(), llh.y(), llh.z());

    auto lat = llh.x();
    auto lon = llh.y();

    east.x() = -sin(lon);
    east.y() = cos(lon);
    east.z() = 0.0;

    north.x() = -sin(lat) * cos(lon);
    north.y() = -sin(lat) * sin(lon);
    north.z() = cos(lat);

    up.x() = cos(lat) * cos(lon);
    up.y() = cos(lat) * sin(lon);
    up.z() = sin(lat);

    VERBOSEF("east:  (%+.4f, %+.4f, %+.4f)", east.x(), east.y(), east.z());
    VERBOSEF("north: (%+.4f, %+.4f, %+.4f)", north.x(), north.y(), north.z());
    VERBOSEF("up:    (%+.4f, %+.4f, %+.4f)", up.x(), up.y(), up.z());
}

void enu_basis_from_xyz(Vector3 const& ecef, Vector3& east, Vector3& north, Vector3& up) NOEXCEPT {
    VSCOPE_FUNCTIONF("%+.4f, %+.4f, %+.4f", ecef.x(), ecef.y(), ecef.z());

    auto r   = ecef.norm();
    auto lat = asin(ecef.z() / r);
    auto lon = atan2(ecef.y(), ecef.x());

    enu_basis_from_llh(Vector3(lat, lon, r), east, north, up);
}

static Matrix3 xyz_to_enu_basis(Vector3 const& xyz) NOEXCEPT {
    auto sin_lat = sin(xyz.x());
    auto cos_lat = cos(xyz.x());
    auto cos_lon = cos(xyz.y());
    auto sin_lon = sin(xyz.y());

    Matrix3 basis;
    basis(0, 0) = -sin_lon;
    basis(0, 1) = cos_lon;
    basis(0, 2) = 0.0;

    basis(1, 0) = -sin_lat * cos_lon;
    basis(1, 1) = -sin_lat * sin_lon;
    basis(1, 2) = cos_lat;

    basis(2, 0) = cos_lat * cos_lon;
    basis(2, 1) = cos_lat * sin_lon;
    basis(2, 2) = sin_lat;

    VERBOSEF("basis: %+.4f %+.4f %+.4f", basis(0, 0), basis(0, 1), basis(0, 2));
    VERBOSEF("       %+.4f %+.4f %+.4f", basis(1, 0), basis(1, 1), basis(1, 2));
    VERBOSEF("       %+.4f %+.4f %+.4f", basis(2, 0), basis(2, 1), basis(2, 2));

    return basis;
}

Vector3 ecef_to_enu_at_llh(Vector3 const& llh, Vector3 const& ecef_vector) {
    VERBOSEF("vector: %+.4f %+.4f %+.4f", ecef_vector.x(), ecef_vector.y(), ecef_vector.z());

    auto basis = xyz_to_enu_basis(llh);
    return basis * ecef_vector;
}

}  // namespace idokeido
