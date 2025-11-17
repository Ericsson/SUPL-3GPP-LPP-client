#include <doctest/doctest.h>
#include <time/gps.hpp>

TEST_CASE("GPS time constants") {
    CHECK(ts::DAY_IN_SECONDS == 86400);
    CHECK(ts::WEEK_IN_SECONDS == 604800);
    CHECK(ts::HOUR_IN_SECONDS == 3600);
}

TEST_CASE("GPS time from week and TOW") {
    auto gps = ts::Gps::from_week_tow(2000, 86400, 0.0);
    CHECK(gps.week() == 2000);
    CHECK(gps.time_of_week().seconds() == 86400);
}

TEST_CASE("GPS epoch - week 0, TOW 0") {
    auto gps = ts::Gps::from_week_tow(0, 0, 0.0);
    CHECK(gps.week() == 0);
    CHECK(gps.time_of_week().seconds() == 0);
    CHECK(gps.timestamp().seconds() == 0);
}

TEST_CASE("GPS end of week boundary") {
    auto gps = ts::Gps::from_week_tow(1, 604799, 0.999);
    CHECK(gps.week() == 1);
    CHECK(gps.time_of_week().seconds() == 604799);
}

TEST_CASE("GPS week rollover - first rollover week 1024") {
    auto gps = ts::Gps::from_week_tow(1024, 0, 0.0);
    CHECK(gps.week() == 1024);
    auto mod_ts = gps.mod_timestamp();
    CHECK(mod_ts.seconds() <= ts::WEEK_IN_SECONDS * 1024);
}

TEST_CASE("GPS week rollover - second rollover week 2048") {
    auto gps = ts::Gps::from_week_tow(2048, 0, 0.0);
    CHECK(gps.week() == 2048);
}

TEST_CASE("GPS from date - GPS epoch Jan 6, 1980") {
    auto gps = ts::Gps::from_ymdhms(1980, 1, 6, 0, 0, 0.0);
    CHECK(gps.week() == 0);
    CHECK(gps.days() == 5);
}

TEST_CASE("GPS from date - leap year Feb 29, 2020") {
    auto gps = ts::Gps::from_ymdhms(2020, 2, 29, 12, 30, 45.5);
    CHECK(gps.timestamp().fraction() == doctest::Approx(0.5));
}

TEST_CASE("GPS time difference") {
    auto gps1 = ts::Gps::from_week_tow(2000, 0, 0.0);
    auto gps2 = ts::Gps::from_week_tow(2001, 0, 0.0);
    auto diff = gps2.difference(gps1);
    CHECK(diff.seconds() == ts::WEEK_IN_SECONDS);
}

TEST_CASE("GPS sub-second precision") {
    auto gps = ts::Gps::from_week_tow(2000, 100, 0.123456789);
    CHECK(gps.timestamp().fraction() == doctest::Approx(0.123456789));
}
