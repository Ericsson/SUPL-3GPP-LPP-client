#include <cmath>
#include <doctest/doctest.h>
#include <generator/tokoro/coordinate.hpp>
#include <generator/tokoro/reference_ellipsoid.hpp>
#include <maths/float3.hpp>

using namespace generator::tokoro;

TEST_CASE("Coordinate - ECEF to LLH roundtrip") {
    auto& wgs84 = ellipsoid::gWgs84;

    Float3 llh_in{0.9, 0.2, 100.0};
    auto   ecef    = llh_to_ecef(llh_in, wgs84);
    auto   llh_out = ecef_to_llh(ecef, wgs84);

    CHECK(llh_out.x == doctest::Approx(llh_in.x).epsilon(1e-9));
    CHECK(llh_out.y == doctest::Approx(llh_in.y).epsilon(1e-9));
    CHECK(llh_out.z == doctest::Approx(llh_in.z).epsilon(1e-6));
}

TEST_CASE("Coordinate - Known ECEF to LLH") {
    auto& wgs84 = ellipsoid::gWgs84;

    Float3 ecef{4027894.006, 307045.600, 4919474.910};
    auto   llh = ecef_to_llh(ecef, wgs84);

    CHECK(llh.x == doctest::Approx(1.00178).epsilon(0.2));
    CHECK(llh.y == doctest::Approx(0.07636).epsilon(0.2));
    CHECK(llh.z > -100);
    CHECK(llh.z < 200);
}

TEST_CASE("Coordinate - Equator zero longitude") {
    auto& wgs84 = ellipsoid::gWgs84;

    Float3 llh{0.0, 0.0, 0.0};
    auto   ecef = llh_to_ecef(llh, wgs84);

    CHECK(ecef.x == doctest::Approx(wgs84.semi_major_axis));
    CHECK(ecef.y == doctest::Approx(0.0).scale(1.0));
    CHECK(ecef.z == doctest::Approx(0.0).scale(1.0));
}

TEST_CASE("Coordinate - North pole") {
    auto& wgs84 = ellipsoid::gWgs84;

    Float3 llh{M_PI / 2.0, 0.0, 0.0};
    auto   ecef = llh_to_ecef(llh, wgs84);

    CHECK(ecef.x == doctest::Approx(0.0).scale(1.0));
    CHECK(ecef.y == doctest::Approx(0.0).scale(1.0));
    CHECK(ecef.z > 6.3e6);
}

TEST_CASE("Coordinate - Earth ellipsoid flattening") {
    auto& wgs84 = ellipsoid::gWgs84;

    Float3 equator{0.0, 0.0, 0.0};
    Float3 pole{M_PI / 2.0, 0.0, 0.0};

    auto ecef_equator = llh_to_ecef(equator, wgs84);
    auto ecef_pole    = llh_to_ecef(pole, wgs84);

    auto dist_equator = ecef_equator.length();
    auto dist_pole    = ecef_pole.length();

    CHECK(dist_equator == doctest::Approx(6378137.0));
    CHECK(dist_pole == doctest::Approx(6356752.0).epsilon(0.001));
    CHECK(dist_equator > dist_pole);
}
