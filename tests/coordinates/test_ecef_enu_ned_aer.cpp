#include <cmath>
#include <coordinates/coordinates.hpp>
#include <doctest/doctest.h>

using namespace coordinates;

struct TestFrame {};

template <>
struct coordinates::FrameTrait<TestFrame> {
    static constexpr Ellipsoid ellipsoid = Ellipsoid::from_a_f(6378137.0, 1.0 / 298.257223563);
};

TEST_CASE("ENU basis orthonormal") {
    auto llh = Llh<TestFrame>::from_degrees(45.0, 10.0, 0);
    auto R   = enu_rotation_matrix(llh.latitude(), llh.longitude());

    auto east  = R.row(0);
    auto north = R.row(1);
    auto up    = R.row(2);

    CHECK(east.norm() == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(north.norm() == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(up.norm() == doctest::Approx(1.0).epsilon(1e-12));

    CHECK(east.dot(north) == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(east.dot(up) == doctest::Approx(0.0).epsilon(1e-12));
    CHECK(north.dot(up) == doctest::Approx(0.0).epsilon(1e-12));

    auto cross = east.cross(north);
    CHECK(cross.x() == doctest::Approx(up.x()).epsilon(1e-12));
    CHECK(cross.y() == doctest::Approx(up.y()).epsilon(1e-12));
    CHECK(cross.z() == doctest::Approx(up.z()).epsilon(1e-12));
}

TEST_CASE("Round trip ECEF -> ENU -> ECEF") {
    Llh<TestFrame>  origin = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    Ecef<TestFrame> ecef_orig{Vector3d(4000000, 3000000, 5000000)};

    auto enu       = ecef_to_enu(ecef_orig, origin);
    auto ecef_back = enu_to_ecef(enu, origin);

    CHECK(ecef_back.x() == doctest::Approx(ecef_orig.x()).epsilon(1e-6));
    CHECK(ecef_back.y() == doctest::Approx(ecef_orig.y()).epsilon(1e-6));
    CHECK(ecef_back.z() == doctest::Approx(ecef_orig.z()).epsilon(1e-6));
}

TEST_CASE("Round trip ENU -> ECEF -> ENU") {
    Llh<TestFrame> origin = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    Enu<TestFrame> enu_orig{Vector3d(100, 200, 50)};

    auto ecef     = enu_to_ecef(enu_orig, origin);
    auto enu_back = ecef_to_enu(ecef, origin);

    CHECK(enu_back.east() == doctest::Approx(enu_orig.east()).epsilon(1e-9));
    CHECK(enu_back.north() == doctest::Approx(enu_orig.north()).epsilon(1e-9));
    CHECK(enu_back.up() == doctest::Approx(enu_orig.up()).epsilon(1e-9));
}

TEST_CASE("Round trip ECEF -> NED -> ECEF") {
    Llh<TestFrame>  origin = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    Ecef<TestFrame> ecef_orig{Vector3d(4000000, 3000000, 5000000)};

    auto ned       = ecef_to_ned(ecef_orig, origin);
    auto ecef_back = ned_to_ecef(ned, origin);

    CHECK(ecef_back.x() == doctest::Approx(ecef_orig.x()).epsilon(1e-6));
    CHECK(ecef_back.y() == doctest::Approx(ecef_orig.y()).epsilon(1e-6));
    CHECK(ecef_back.z() == doctest::Approx(ecef_orig.z()).epsilon(1e-6));
}

TEST_CASE("ENU to NED conversion") {
    Llh<TestFrame>  origin = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    Ecef<TestFrame> ecef{Vector3d(4000000, 3000000, 5000000)};

    auto enu = ecef_to_enu(ecef, origin);
    auto ned = ecef_to_ned(ecef, origin);

    CHECK(ned.north() == doctest::Approx(enu.north()).epsilon(1e-12));
    CHECK(ned.east() == doctest::Approx(enu.east()).epsilon(1e-12));
    CHECK(ned.down() == doctest::Approx(-enu.up()).epsilon(1e-12));
}

TEST_CASE("Round trip ECEF -> AER -> ECEF") {
    Llh<TestFrame>  origin = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    Ecef<TestFrame> ecef_orig{Vector3d(4000000, 3000000, 5000000)};

    auto aer       = ecef_to_aer(ecef_orig, origin);
    auto ecef_back = aer_to_ecef(aer, origin);

    CHECK(ecef_back.x() == doctest::Approx(ecef_orig.x()).epsilon(1e-6));
    CHECK(ecef_back.y() == doctest::Approx(ecef_orig.y()).epsilon(1e-6));
    CHECK(ecef_back.z() == doctest::Approx(ecef_orig.z()).epsilon(1e-6));
}

TEST_CASE("AER degree helpers") {
    auto aer = Aer::from_degrees(45.0, 30.0, 1000.0);

    CHECK(aer.azimuth_deg() == doctest::Approx(45.0).epsilon(1e-12));
    CHECK(aer.elevation_deg() == doctest::Approx(30.0).epsilon(1e-12));
    CHECK(aer.range() == doctest::Approx(1000.0).epsilon(1e-6));
}

TEST_CASE("AER zenith") {
    Llh<TestFrame> origin      = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    auto           origin_ecef = llh_to_ecef(origin);
    auto           R           = enu_rotation_matrix(origin.latitude(), origin.longitude());
    auto           up          = R.row(2);

    Ecef<TestFrame> sat_zenith{origin_ecef.value + 20000000.0 * up.transpose()};
    auto            aer = ecef_to_aer(sat_zenith, origin);

    CHECK(aer.elevation() == doctest::Approx(M_PI / 2).epsilon(1e-6));
}

TEST_CASE("AER azimuth north") {
    Llh<TestFrame> origin      = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    auto           origin_ecef = llh_to_ecef(origin);
    auto           R           = enu_rotation_matrix(origin.latitude(), origin.longitude());
    auto           north       = R.row(1);

    Ecef<TestFrame> sat_north{origin_ecef.value + 1000000.0 * north.transpose()};
    auto            aer = ecef_to_aer(sat_north, origin);

    CHECK(aer.azimuth() == doctest::Approx(0.0).epsilon(1e-3));
}
