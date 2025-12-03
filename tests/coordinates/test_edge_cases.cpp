#include <cmath>
#include <coordinates/coordinates.hpp>
#include <doctest/doctest.h>

using namespace coordinates;
using TestFrame = WGS84_G1762;

TEST_CASE("Date line crossing - positive longitude") {
    Llh<TestFrame> llh      = Llh<TestFrame>::from_degrees(0.0, 179.9, 0.0);
    auto           ecef     = llh_to_ecef(llh);
    auto           llh_back = ecef_to_llh(ecef);

    CHECK(llh_back.longitude_deg() == doctest::Approx(179.9).epsilon(1e-9));
}

TEST_CASE("Date line crossing - negative longitude") {
    Llh<TestFrame> llh      = Llh<TestFrame>::from_degrees(0.0, -179.9, 0.0);
    auto           ecef     = llh_to_ecef(llh);
    auto           llh_back = ecef_to_llh(ecef);

    CHECK(llh_back.longitude_deg() == doctest::Approx(-179.9).epsilon(1e-9));
}

TEST_CASE("High altitude") {
    Llh<TestFrame> llh      = Llh<TestFrame>::from_degrees(45.0, 10.0, 100000.0);
    auto           ecef     = llh_to_ecef(llh);
    auto           llh_back = ecef_to_llh(ecef);

    CHECK(llh_back.latitude_deg() == doctest::Approx(45.0).epsilon(1e-9));
    CHECK(llh_back.longitude_deg() == doctest::Approx(10.0).epsilon(1e-9));
    CHECK(llh_back.height() == doctest::Approx(100000.0).epsilon(1e-6));
}

TEST_CASE("Negative altitude") {
    Llh<TestFrame> llh      = Llh<TestFrame>::from_degrees(45.0, 10.0, -100.0);
    auto           ecef     = llh_to_ecef(llh);
    auto           llh_back = ecef_to_llh(ecef);

    CHECK(llh_back.latitude_deg() == doctest::Approx(45.0).epsilon(1e-9));
    CHECK(llh_back.longitude_deg() == doctest::Approx(10.0).epsilon(1e-9));
    CHECK(llh_back.height() == doctest::Approx(-100.0).epsilon(1e-6));
}

TEST_CASE("ECEF operators") {
    Ecef<TestFrame> a{Vector3d(1000, 2000, 3000)};
    Ecef<TestFrame> b{Vector3d(100, 200, 300)};

    auto sum = a + b;
    CHECK(sum.x() == doctest::Approx(1100).epsilon(1e-12));
    CHECK(sum.y() == doctest::Approx(2200).epsilon(1e-12));
    CHECK(sum.z() == doctest::Approx(3300).epsilon(1e-12));

    auto diff = a - b;
    CHECK(diff.x() == doctest::Approx(900).epsilon(1e-12));
    CHECK(diff.y() == doctest::Approx(1800).epsilon(1e-12));
    CHECK(diff.z() == doctest::Approx(2700).epsilon(1e-12));

    auto scaled = 2.0 * a;
    CHECK(scaled.x() == doctest::Approx(2000).epsilon(1e-12));
    CHECK(scaled.y() == doctest::Approx(4000).epsilon(1e-12));
    CHECK(scaled.z() == doctest::Approx(6000).epsilon(1e-12));

    double dist = distance(a, b);
    CHECK(dist == doctest::Approx(std::sqrt(900 * 900 + 1800 * 1800 + 2700 * 2700)).epsilon(1e-6));
}

TEST_CASE("AER azimuth wrapping") {
    Llh<TestFrame> origin      = Llh<TestFrame>::from_degrees(45.0, 10.0, 100.0);
    auto           origin_ecef = llh_to_ecef(origin);
    auto           R           = enu_rotation_matrix(origin.latitude(), origin.longitude());
    auto           east        = R.row(0);

    Ecef<TestFrame> sat_east{origin_ecef.value + 1000000.0 * east.transpose()};
    auto            aer = ecef_to_aer(sat_east, origin);

    CHECK(aer.azimuth() >= 0.0);
    CHECK(aer.azimuth() < 2 * M_PI);
    CHECK(aer.azimuth_deg() == doctest::Approx(90.0).epsilon(1.0));
}
