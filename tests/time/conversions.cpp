#include <doctest/doctest.h>
#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>
#include <time/tai.hpp>
#include <time/utc.hpp>

TEST_CASE("UTC to GPS round-trip") {
    auto utc  = ts::Utc::from_date_time(2020, 1, 1, 0, 0, 0.0);
    auto gps  = ts::Gps(utc);
    auto utc2 = ts::Utc(gps);
    CHECK(utc.timestamp().seconds() == utc2.timestamp().seconds());
}

TEST_CASE("TAI to GPS round-trip") {
    auto tai  = ts::Tai(ts::Timestamp(1000000, 0.0));
    auto gps  = ts::Gps(tai);
    auto tai2 = ts::Tai(gps);
    CHECK(tai.timestamp().seconds() == tai2.timestamp().seconds());
}

TEST_CASE("GPS to BDT round-trip") {
    auto gps  = ts::Gps::from_week_tow(2000, 100000, 0.5);
    auto bdt  = ts::Bdt(gps);
    auto gps2 = ts::Gps(bdt);
    CHECK(gps.timestamp().seconds() == gps2.timestamp().seconds());
    CHECK(gps.timestamp().fraction() == doctest::Approx(gps2.timestamp().fraction()));
}

TEST_CASE("GPS to GST round-trip") {
    auto gps  = ts::Gps::from_week_tow(2000, 100000, 0.5);
    auto gst  = ts::Gst(gps);
    auto gps2 = ts::Gps(gst);
    CHECK(gps.timestamp().seconds() == gps2.timestamp().seconds());
    CHECK(gps.timestamp().fraction() == doctest::Approx(gps2.timestamp().fraction()));
}

TEST_CASE("GPS to GLO round-trip") {
    auto gps  = ts::Gps::from_week_tow(2000, 100000, 0.5);
    auto glo  = ts::Glo(gps);
    auto gps2 = ts::Gps(glo);
    CHECK(gps.timestamp().seconds() == gps2.timestamp().seconds());
    CHECK(gps.timestamp().fraction() == doctest::Approx(gps2.timestamp().fraction()));
}

TEST_CASE("UTC to TAI round-trip") {
    auto utc  = ts::Utc::from_date_time(2020, 6, 15, 12, 30, 45.123);
    auto tai  = ts::Tai(utc);
    auto utc2 = ts::Utc(tai);
    CHECK(utc.timestamp().seconds() == utc2.timestamp().seconds());
    CHECK(utc.timestamp().fraction() == doctest::Approx(utc2.timestamp().fraction()));
}

TEST_CASE("UTC to BDT round-trip") {
    auto utc  = ts::Utc::from_date_time(2020, 6, 15, 12, 30, 45.123);
    auto bdt  = ts::Bdt(utc);
    auto utc2 = ts::Utc(bdt);
    CHECK(utc.timestamp().seconds() == utc2.timestamp().seconds());
    CHECK(utc.timestamp().fraction() == doctest::Approx(utc2.timestamp().fraction()));
}

TEST_CASE("UTC to GST round-trip") {
    auto utc  = ts::Utc::from_date_time(2020, 6, 15, 12, 30, 45.123);
    auto gst  = ts::Gst(utc);
    auto utc2 = ts::Utc(gst);
    CHECK(utc.timestamp().seconds() == utc2.timestamp().seconds());
    CHECK(utc.timestamp().fraction() == doctest::Approx(utc2.timestamp().fraction()));
}

TEST_CASE("UTC to GLO round-trip") {
    auto utc  = ts::Utc::from_date_time(2020, 6, 15, 12, 30, 45.123);
    auto glo  = ts::Glo(utc);
    auto utc2 = ts::Utc(glo);
    CHECK(utc.timestamp().seconds() == utc2.timestamp().seconds());
    CHECK(utc.timestamp().fraction() == doctest::Approx(utc2.timestamp().fraction()));
}

TEST_CASE("TAI to BDT round-trip") {
    auto tai  = ts::Tai(ts::Timestamp(1000000, 0.25));
    auto bdt  = ts::Bdt(tai);
    auto tai2 = ts::Tai(bdt);
    CHECK(tai.timestamp().seconds() == tai2.timestamp().seconds());
    CHECK(tai.timestamp().fraction() == doctest::Approx(tai2.timestamp().fraction()));
}

TEST_CASE("TAI to GST round-trip") {
    auto tai  = ts::Tai(ts::Timestamp(1000000, 0.25));
    auto gst  = ts::Gst(tai);
    auto tai2 = ts::Tai(gst);
    CHECK(tai.timestamp().seconds() == tai2.timestamp().seconds());
    CHECK(tai.timestamp().fraction() == doctest::Approx(tai2.timestamp().fraction()));
}

TEST_CASE("TAI to GLO round-trip") {
    auto tai  = ts::Tai(ts::Timestamp(1000000, 0.25));
    auto glo  = ts::Glo(tai);
    auto tai2 = ts::Tai(glo);
    CHECK(tai.timestamp().seconds() == tai2.timestamp().seconds());
    CHECK(tai.timestamp().fraction() == doctest::Approx(tai2.timestamp().fraction()));
}

TEST_CASE("BDT to GST round-trip") {
    auto bdt  = ts::Bdt(ts::Timestamp(500000, 0.75));
    auto gst  = ts::Gst(bdt);
    auto bdt2 = ts::Bdt(gst);
    CHECK(bdt.timestamp().seconds() == bdt2.timestamp().seconds());
    CHECK(bdt.timestamp().fraction() == doctest::Approx(bdt2.timestamp().fraction()));
}

TEST_CASE("BDT to GLO round-trip") {
    auto bdt  = ts::Bdt(ts::Timestamp(500000, 0.75));
    auto glo  = ts::Glo(bdt);
    auto bdt2 = ts::Bdt(glo);
    CHECK(bdt.timestamp().seconds() == bdt2.timestamp().seconds());
    CHECK(bdt.timestamp().fraction() == doctest::Approx(bdt2.timestamp().fraction()));
}

TEST_CASE("GST to GLO round-trip") {
    auto gst  = ts::Gst(ts::Timestamp(500000, 0.75));
    auto glo  = ts::Glo(gst);
    auto gst2 = ts::Gst(glo);
    CHECK(gst.timestamp().seconds() == gst2.timestamp().seconds());
    CHECK(gst.timestamp().fraction() == doctest::Approx(gst2.timestamp().fraction()));
}
