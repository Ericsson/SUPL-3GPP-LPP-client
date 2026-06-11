#include <doctest/doctest.h>
#include <format/rinex/nav_reader.hpp>

#include <cmath>

static char const* TEST_NAV_FILE = "tests/corpus/rinex/nav_mixed.nav";

TEST_CASE("RINEX nav reader - parse_nav_file") {
    auto nav = format::rinex::parse_nav_file(TEST_NAV_FILE);

    SUBCASE("counts") {
        CHECK(nav.gps.size() == 2);
        CHECK(nav.gal.size() == 1);
        CHECK(nav.bds.size() == 1);
    }

    SUBCASE("GPS G10 ephemeris") {
        auto& eph = nav.gps[0];
        CHECK(eph.prn == 10);
        CHECK(eph.week_number == 2421);
        CHECK(eph.toc ==
              doctest::Approx(518400.0));  // 2026-06-06 00:00:00 = Saturday DOW=6, TOW=6*86400
        // TOE/TOC should be in seconds of week
        CHECK(eph.toe > 0);
        CHECK(eph.af0 == doctest::Approx(-5.798246711490e-04).epsilon(1e-15));
        CHECK(eph.a > 2.655e7);  // ~26.5M meters (GPS semi-major axis)
        CHECK(eph.e < 0.1);      // eccentricity should be small
        CHECK(eph.iodc > 0);     // should have a valid IODC
        CHECK(eph.iode <= 255);  // IODE is 8-bit
    }

    SUBCASE("GPS G15 ephemeris") {
        auto& eph = nav.gps[1];
        CHECK(eph.prn == 15);
        CHECK(eph.af0 == doctest::Approx(4.177377559240e-04).epsilon(1e-15));
    }

    SUBCASE("Galileo E10 ephemeris") {
        auto& eph = nav.gal[0];
        CHECK(eph.prn == 10);
        CHECK(eph.af0 == doctest::Approx(-7.972235907800e-04).epsilon(1e-15));
        CHECK(eph.a > 2.9e7);  // Galileo orbit ~29.6M meters
        CHECK(eph.iod_nav > 0);
    }

    SUBCASE("BeiDou C11 ephemeris") {
        auto& eph = nav.bds[0];
        CHECK(eph.prn == 11);
        CHECK(eph.af0 == doctest::Approx(-7.046000100672e-04).epsilon(1e-15));
        CHECK(eph.a > 2.6e7);
    }
}

TEST_CASE("RINEX nav reader - nonexistent file") {
    auto nav = format::rinex::parse_nav_file("/nonexistent/path.nav");
    CHECK(nav.gps.empty());
    CHECK(nav.gal.empty());
    CHECK(nav.bds.empty());
}
