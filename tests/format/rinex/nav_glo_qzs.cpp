#include <doctest/doctest.h>
#include <format/rinex/nav_reader.hpp>
#include <time/utc.hpp>

TEST_CASE("RINEX nav reader - GLONASS and QZSS records") {
    auto nav = format::rinex::parse_nav_file("tests/corpus/rinex/nav_glo_qzs.nav");
    REQUIRE(nav.qzs.size() == 1);
    REQUIRE(nav.glo.size() == 1);
    CHECK(nav.gps.empty());

    SUBCASE("QZSS J02") {
        auto& eph = nav.qzs[0];
        CHECK(eph.prn == 194);
        CHECK(eph.iodc == 64);
        CHECK(eph.lpp_iod == 64);
        CHECK(eph.af0 == doctest::Approx(-5.798246711490e-04));
        CHECK(eph.a == doctest::Approx(5.153651136400e+03 * 5.153651136400e+03));
    }

    SUBCASE("GLONASS R05") {
        auto& eph = nav.glo[0];
        CHECK(eph.slot_number == 5);
        CHECK(eph.frequency_number == 1);
        CHECK(eph.health == 0);
        // the record stores -TauN
        CHECK(eph.tau_n == doctest::Approx(-2.391310408711e-05));
        CHECK(eph.position.x == doctest::Approx(1.073236718750e+04));
        CHECK(eph.velocity.y == doctest::Approx(-1.364078521729e+00));
        CHECK(eph.acceleration.z == doctest::Approx(-2.793967723846e-09));
        // 00:15 UTC is 03:15 Moscow time, tb = 13 (15 minute index)
        CHECK(eph.lpp_iod == 13);
        CHECK(ts::Utc(eph.reference_time).timestamp().full_seconds() ==
              doctest::Approx(
                  ts::Utc::from_date_time(2026, 6, 6, 0, 15, 0.0).timestamp().full_seconds()));
    }
}
