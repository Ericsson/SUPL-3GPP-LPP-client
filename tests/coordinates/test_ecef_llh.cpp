#include <cmath>
#include <coordinates/coordinates.hpp>
#include <doctest/doctest.h>

using namespace coordinates;

struct TestFrame {};

template <>
struct coordinates::FrameTrait<TestFrame> {
    static constexpr Ellipsoid ellipsoid = Ellipsoid::from_a_f(6378137.0, 1.0 / 298.257223563);
};

TEST_CASE("ECEF to LLH - Equator Prime Meridian") {
    Ecef<TestFrame> ecef{Vector3d(FrameTrait<TestFrame>::ellipsoid.a, 0, 0)};
    auto            llh = ecef_to_llh(ecef);

    CHECK(llh.latitude() == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(llh.longitude() == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(llh.height() == doctest::Approx(0.0).epsilon(1e-6));
}

TEST_CASE("ECEF to LLH - North Pole") {
    Ecef<TestFrame> ecef{Vector3d(0, 0, FrameTrait<TestFrame>::ellipsoid.b)};
    auto            llh = ecef_to_llh(ecef);

    CHECK(llh.latitude() == doctest::Approx(M_PI / 2).epsilon(1e-12));
    CHECK(llh.height() == doctest::Approx(0.0).epsilon(1e-6));
}

TEST_CASE("ECEF to LLH - South Pole") {
    Ecef<TestFrame> ecef{Vector3d(0, 0, -FrameTrait<TestFrame>::ellipsoid.b)};
    auto            llh = ecef_to_llh(ecef);

    CHECK(llh.latitude() == doctest::Approx(-M_PI / 2).epsilon(1e-12));
    CHECK(llh.height() == doctest::Approx(0.0).epsilon(1e-6));
}

TEST_CASE("LLH to ECEF - Equator Prime Meridian") {
    Llh<TestFrame> llh{Vector3d(0, 0, 0)};
    auto           ecef = llh_to_ecef(llh);

    CHECK(ecef.x() == doctest::Approx(FrameTrait<TestFrame>::ellipsoid.a).epsilon(1e-6));
    CHECK(ecef.y() == doctest::Approx(0.0).epsilon(1e-6));
    CHECK(ecef.z() == doctest::Approx(0.0).epsilon(1e-6));
}

TEST_CASE("LLH to ECEF - North Pole") {
    Llh<TestFrame> llh{Vector3d(M_PI / 2, 0, 0)};
    auto           ecef = llh_to_ecef(llh);

    CHECK(ecef.x() == doctest::Approx(0.0).epsilon(1e-6));
    CHECK(ecef.y() == doctest::Approx(0.0).epsilon(1e-6));
    CHECK(ecef.z() == doctest::Approx(FrameTrait<TestFrame>::ellipsoid.b).epsilon(1e-6));
}

TEST_CASE("Round trip ECEF -> LLH -> ECEF") {
    Ecef<TestFrame> ecef_orig{Vector3d(4000000, 3000000, 5000000)};
    auto            llh       = ecef_to_llh(ecef_orig);
    auto            ecef_back = llh_to_ecef(llh);

    CHECK(ecef_back.x() == doctest::Approx(ecef_orig.x()).epsilon(1e-6));
    CHECK(ecef_back.y() == doctest::Approx(ecef_orig.y()).epsilon(1e-6));
    CHECK(ecef_back.z() == doctest::Approx(ecef_orig.z()).epsilon(1e-6));
}

TEST_CASE("Round trip LLH -> ECEF -> LLH") {
    Llh<TestFrame> llh_orig = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    auto           ecef     = llh_to_ecef(llh_orig);
    auto           llh_back = ecef_to_llh(ecef);

    CHECK(llh_back.latitude() == doctest::Approx(llh_orig.latitude()).epsilon(1e-12));
    CHECK(llh_back.longitude() == doctest::Approx(llh_orig.longitude()).epsilon(1e-12));
    CHECK(llh_back.height() == doctest::Approx(llh_orig.height()).epsilon(1e-9));
}

TEST_CASE("LLH degree helpers") {
    auto llh = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);

    CHECK(llh.latitude_deg() == doctest::Approx(45.0).epsilon(1e-12));
    CHECK(llh.longitude_deg() == doctest::Approx(10.0).epsilon(1e-12));
    CHECK(llh.height() == doctest::Approx(100.0).epsilon(1e-6));
}
