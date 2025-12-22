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

TEST_CASE("TAI scalar operators") {
    auto tai    = ts::Tai(ts::Timestamp(1000000, 0.0));
    auto result = tai + 3600.5;
    CHECK(result.timestamp().seconds() == 1003600);
    CHECK(result.timestamp().fraction() == doctest::Approx(0.5));

    result = tai - 100.25;
    CHECK(result.timestamp().seconds() == 999899);
    CHECK(result.timestamp().fraction() == doctest::Approx(0.75));
}

TEST_CASE("TAI += and -= operators") {
    auto tai = ts::Tai(ts::Timestamp(1000000, 0.0));
    tai += 3600.5;
    CHECK(tai.timestamp().seconds() == 1003600);
    CHECK(tai.timestamp().fraction() == doctest::Approx(0.5));

    tai -= 3600.5;
    CHECK(tai.timestamp().seconds() == 1000000);
    CHECK(tai.timestamp().fraction() == doctest::Approx(0.0));
}

TEST_CASE("TAI operator- returns double") {
    auto tai1  = ts::Tai(ts::Timestamp(1000000, 0.0));
    auto tai2  = ts::Tai(ts::Timestamp(1003600, 0.5));
    auto delta = tai2 - tai1;
    CHECK(delta == doctest::Approx(3600.5));
}

TEST_CASE("TAI comparison operators") {
    auto tai1 = ts::Tai(ts::Timestamp(1000000, 0.0));
    auto tai2 = ts::Tai(ts::Timestamp(1000000, 0.5));
    auto tai3 = ts::Tai(ts::Timestamp(1000000, 0.0));

    CHECK(tai1 < tai2);
    CHECK(tai1 <= tai2);
    CHECK(tai2 > tai1);
    CHECK(tai2 >= tai1);
    CHECK(tai1 == tai3);
    CHECK(tai1 != tai2);
}

TEST_CASE("UTC scalar operators") {
    auto utc    = ts::Utc::from_date_time(2020, 1, 1, 0, 0, 0.0);
    auto result = utc + 3600.5;
    CHECK((result - utc) == doctest::Approx(3600.5));

    result = utc - 100.25;
    CHECK((utc - result) == doctest::Approx(100.25));
}

TEST_CASE("UTC comparison operators") {
    auto utc1 = ts::Utc::from_date_time(2020, 1, 1, 0, 0, 0.0);
    auto utc2 = ts::Utc::from_date_time(2020, 1, 1, 1, 0, 0.0);
    auto utc3 = ts::Utc::from_date_time(2020, 1, 1, 0, 0, 0.0);

    CHECK(utc1 < utc2);
    CHECK(utc1 <= utc2);
    CHECK(utc2 > utc1);
    CHECK(utc2 >= utc1);
    CHECK(utc1 == utc3);
    CHECK(utc1 != utc2);
}

TEST_CASE("GST scalar operators") {
    auto gst    = ts::Gst::from_week_tow(1000, 0, 0.0);
    auto result = gst + 3600.5;
    CHECK((result - gst) == doctest::Approx(3600.5));
}

TEST_CASE("GST comparison operators") {
    auto gst1 = ts::Gst::from_week_tow(1000, 0, 0.0);
    auto gst2 = ts::Gst::from_week_tow(1000, 3600, 0.0);

    CHECK(gst1 < gst2);
    CHECK(gst2 > gst1);
    CHECK(gst1 != gst2);
}

TEST_CASE("BDT scalar operators") {
    auto bdt    = ts::Bdt::from_week_tow(500, 0, 0.0);
    auto result = bdt + 3600.5;
    CHECK((result - bdt) == doctest::Approx(3600.5));
}

TEST_CASE("BDT comparison operators") {
    auto bdt1 = ts::Bdt::from_week_tow(500, 0, 0.0);
    auto bdt2 = ts::Bdt::from_week_tow(500, 3600, 0.0);

    CHECK(bdt1 < bdt2);
    CHECK(bdt2 > bdt1);
    CHECK(bdt1 != bdt2);
}

TEST_CASE("GLO scalar operators") {
    auto glo    = ts::Glo::from_absolute_day_tod(1000, 0.0);
    auto result = glo + 3600.5;
    CHECK((result - glo) == doctest::Approx(3600.5));
}

TEST_CASE("GLO comparison operators") {
    auto glo1 = ts::Glo::from_absolute_day_tod(1000, 0.0);
    auto glo2 = ts::Glo::from_absolute_day_tod(1000, 3600.0);

    CHECK(glo1 < glo2);
    CHECK(glo2 > glo1);
    CHECK(glo1 != glo2);
}
