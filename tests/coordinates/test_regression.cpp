#include <coordinates/coordinates.hpp>
#include <coordinates/frame/wgs84.hpp>
#include <doctest/doctest.h>
#include <maths/float3.hpp>

#ifdef INCLUDE_GENERATOR_TOKORO
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
#endif

using namespace coordinates;

#ifdef INCLUDE_GENERATOR_TOKORO
TEST_CASE("Regression vs tokoro - ECEF to LLH") {
    auto& tokoro_ell = generator::tokoro::ellipsoid::gWgs84;

    std::vector<Float3> test_ecef = {
        {coordinates::ellipsoids::Wgs84.a, 0, 0},
        {0, coordinates::ellipsoids::Wgs84.a, 0},
        {4000000, 3000000, 5000000},
        {-2000000, 3000000, 4000000},
    };

    for (auto const& ecef_tokoro : test_ecef) {
        Ecef<TestFrame> ecef_new{Vector3d(ecef_tokoro.x, ecef_tokoro.y, ecef_tokoro.z)};

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
        Llh<TestFrame> llh_new{Vector3d(llh_tokoro.x, llh_tokoro.y, llh_tokoro.z)};

        auto ecef_tokoro = generator::tokoro::llh_to_ecef(llh_tokoro, tokoro_ell);
        auto ecef_new    = llh_to_ecef(llh_new);

        CHECK(ecef_new.x() == doctest::Approx(ecef_tokoro.x).epsilon(1e-6));
        CHECK(ecef_new.y() == doctest::Approx(ecef_tokoro.y).epsilon(1e-6));
        CHECK(ecef_new.z() == doctest::Approx(ecef_tokoro.z).epsilon(1e-6));
    }
}

TEST_CASE("Regression vs tokoro - ECEF to ENU") {
    Float3         origin_llh{M_PI / 4, M_PI / 6, 100};
    Llh<TestFrame> origin_new{Vector3d(origin_llh.x, origin_llh.y, origin_llh.z)};

    std::vector<Float3> test_vectors = {
        {1000, 0, 0}, {0, 1000, 0}, {0, 0, 1000}, {1000, 1000, 1000}, {-500, 500, -500},
    };

    for (auto const& vec : test_vectors) {
        auto enu_tokoro = generator::tokoro::ecef_to_enu_at_llh(origin_llh, vec);

        auto            origin_ecef = llh_to_ecef(origin_new);
        Ecef<TestFrame> target_ecef{origin_ecef.value + Vector3d(vec.x, vec.y, vec.z)};
        auto            enu_new = ecef_to_enu(target_ecef, origin_new);

        CHECK(enu_new.east() == doctest::Approx(enu_tokoro.x).epsilon(1e-6));
        CHECK(enu_new.north() == doctest::Approx(enu_tokoro.y).epsilon(1e-6));
        CHECK(enu_new.up() == doctest::Approx(enu_tokoro.z).epsilon(1e-6));
    }
}
#endif
