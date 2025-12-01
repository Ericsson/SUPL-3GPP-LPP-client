#include <coordinates/coordinates.hpp>
#include <doctest/doctest.h>
#include <maths/float3.hpp>

namespace generator::tokoro {
struct ReferenceEllipsoid {
    double semi_major_axis;
    double semi_minor_axis;
    double flattening;
    double eccentricity_sq;
    double eccentricity;

    static ReferenceEllipsoid create(double semi_major_axis, double flattening);
};
Float3 ecef_to_llh(Float3 ecef, ReferenceEllipsoid const& ellipsoid);
Float3 llh_to_ecef(Float3 llh, ReferenceEllipsoid const& ellipsoid);
Float3 ecef_to_enu_at_llh(Float3 llh, Float3 ecef_vector);

namespace ellipsoid {
extern ReferenceEllipsoid const gWgs84;
}
}  // namespace generator::tokoro

namespace idokeido {
using Scalar  = double;
using Vector3 = Eigen::Matrix<Scalar, 3, 1>;

struct ReferenceEllipsoid {
    double semi_major_axis;
    double semi_minor_axis;
    double flattening;
    double eccentricity_sq;
    double eccentricity;
};

Vector3 ecef_to_llh(Vector3 ecef, ReferenceEllipsoid const& ellipsoid);
Vector3 llh_to_ecef(Vector3 llh, ReferenceEllipsoid const& ellipsoid);

namespace ellipsoid {
extern ReferenceEllipsoid const WGS84;
}
}  // namespace idokeido

using namespace coordinates;

struct WGS84 {
    static constexpr Ellipsoid ellipsoid = Ellipsoid::from_a_f(6378137.0, 1.0 / 298.257223563);
};

TEST_CASE("Regression vs tokoro - ECEF to LLH") {
    auto& tokoro_ell = generator::tokoro::ellipsoid::gWgs84;

    std::vector<Float3> test_ecef = {
        {WGS84::ellipsoid.a, 0, 0},
        {0, WGS84::ellipsoid.a, 0},
        {4000000, 3000000, 5000000},
        {-2000000, 3000000, 4000000},
    };

    for (auto const& ecef_tokoro : test_ecef) {
        Ecef<WGS84> ecef_new{Vector3d(ecef_tokoro.x, ecef_tokoro.y, ecef_tokoro.z)};

        auto llh_tokoro = generator::tokoro::ecef_to_llh(ecef_tokoro, tokoro_ell);
        auto llh_new    = ecef_to_llh(ecef_new);

        CHECK(llh_new.latitude() == doctest::Approx(llh_tokoro.x).epsilon(1e-12));
        CHECK(llh_new.longitude() == doctest::Approx(llh_tokoro.y).epsilon(1e-12));
        CHECK(llh_new.height() == doctest::Approx(llh_tokoro.z).epsilon(1e-6));
    }
}

TEST_CASE("Regression vs tokoro - LLH to ECEF") {
    auto& tokoro_ell = generator::tokoro::ellipsoid::gWgs84;

    std::vector<Float3> test_llh = {
        {0, 0, 0},
        {M_PI / 4, M_PI / 6, 100},
        {-M_PI / 6, -M_PI / 3, 1000},
    };

    for (auto const& llh_tokoro : test_llh) {
        Llh<WGS84> llh_new{Vector3d(llh_tokoro.x, llh_tokoro.y, llh_tokoro.z)};

        auto ecef_tokoro = generator::tokoro::llh_to_ecef(llh_tokoro, tokoro_ell);
        auto ecef_new    = llh_to_ecef(llh_new);

        CHECK(ecef_new.x() == doctest::Approx(ecef_tokoro.x).epsilon(1e-6));
        CHECK(ecef_new.y() == doctest::Approx(ecef_tokoro.y).epsilon(1e-6));
        CHECK(ecef_new.z() == doctest::Approx(ecef_tokoro.z).epsilon(1e-6));
    }
}

TEST_CASE("Regression vs tokoro - ECEF to ENU") {
    Float3     origin_llh{M_PI / 4, M_PI / 6, 100};
    Llh<WGS84> origin_new{Vector3d(origin_llh.x, origin_llh.y, origin_llh.z)};

    std::vector<Float3> test_vectors = {
        {1000, 0, 0}, {0, 1000, 0}, {0, 0, 1000}, {1000, 1000, 1000}, {-500, 500, -500},
    };

    for (auto const& vec : test_vectors) {
        auto enu_tokoro = generator::tokoro::ecef_to_enu_at_llh(origin_llh, vec);

        auto        origin_ecef = llh_to_ecef(origin_new);
        Ecef<WGS84> target_ecef{origin_ecef.value + Vector3d(vec.x, vec.y, vec.z)};
        auto        enu_new = ecef_to_enu(target_ecef, origin_new);

        CHECK(enu_new.east() == doctest::Approx(enu_tokoro.x).epsilon(1e-6));
        CHECK(enu_new.north() == doctest::Approx(enu_tokoro.y).epsilon(1e-6));
        CHECK(enu_new.up() == doctest::Approx(enu_tokoro.z).epsilon(1e-6));
    }
}

TEST_CASE("Regression vs idokeido - ECEF to LLH") {
    auto& idokeido_ell = idokeido::ellipsoid::WGS84;

    std::vector<idokeido::Vector3> test_ecef = {
        {WGS84::ellipsoid.a, 0, 0},
        {0, WGS84::ellipsoid.a, 0},
        {4000000, 3000000, 5000000},
        {-2000000, 3000000, 4000000},
    };

    for (auto const& ecef_idokeido : test_ecef) {
        Ecef<WGS84> ecef_new{Vector3d(ecef_idokeido.x(), ecef_idokeido.y(), ecef_idokeido.z())};

        auto llh_idokeido = idokeido::ecef_to_llh(ecef_idokeido, idokeido_ell);
        auto llh_new      = ecef_to_llh(ecef_new);

        CHECK(llh_new.latitude() == doctest::Approx(llh_idokeido.x()).epsilon(1e-12));
        CHECK(llh_new.longitude() == doctest::Approx(llh_idokeido.y()).epsilon(1e-12));
        CHECK(llh_new.height() == doctest::Approx(llh_idokeido.z()).epsilon(1e-6));
    }
}

TEST_CASE("Regression vs idokeido - LLH to ECEF") {
    auto& idokeido_ell = idokeido::ellipsoid::WGS84;

    std::vector<idokeido::Vector3> test_llh = {
        {0, 0, 0},
        {M_PI / 4, M_PI / 6, 100},
        {-M_PI / 6, -M_PI / 3, 1000},
    };

    for (auto const& llh_idokeido : test_llh) {
        Llh<WGS84> llh_new{Vector3d(llh_idokeido.x(), llh_idokeido.y(), llh_idokeido.z())};

        auto ecef_idokeido = idokeido::llh_to_ecef(llh_idokeido, idokeido_ell);
        auto ecef_new      = llh_to_ecef(llh_new);

        CHECK(ecef_new.x() == doctest::Approx(ecef_idokeido.x()).epsilon(1e-6));
        CHECK(ecef_new.y() == doctest::Approx(ecef_idokeido.y()).epsilon(1e-6));
        CHECK(ecef_new.z() == doctest::Approx(ecef_idokeido.z()).epsilon(1e-6));
    }
}
