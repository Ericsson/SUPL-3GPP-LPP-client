#include <doctest/doctest.h>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/utc.hpp>

TEST_CASE("GLONASS time from day and TOD") {
    auto glo = ts::Glo::from_absolute_day_tod(16750, 85500);
    CHECK(glo.days() == 16750);
    CHECK(glo.time_of_day().seconds() == 85500);
}

TEST_CASE("GLONASS epoch - day 0, TOD 0") {
    auto glo = ts::Glo::from_absolute_day_tod(0, 0);
    CHECK(glo.days() == 0);
    CHECK(glo.time_of_day().seconds() == 0);
}

TEST_CASE("GLONASS 4-year period boundary") {
    auto glo1 = ts::Glo::from_absolute_day_tod(1460, 86399);
    auto glo2 = ts::Glo::from_absolute_day_tod(1461, 0);
    CHECK(glo1.days() == 1460);
    CHECK(glo2.days() == 1461);
}

TEST_CASE("GLONASS from GPS time round-trip") {
    auto gps  = ts::Gps::from_week_tow(2291, 345318, 0.0);
    auto glo  = ts::Glo{gps};
    auto gps2 = ts::Gps{glo};

    CHECK(gps.timestamp().full_seconds() == doctest::Approx(gps2.timestamp().full_seconds()));
}

TEST_CASE("GLONASS day of week") {
    auto glo = ts::Glo::from_day_tod(16750, 85500);
    auto dow = glo.day_of_week();
    CHECK(dow >= 0);
    CHECK(dow < 7);
}

TEST_CASE("GLONASS time equality") {
    auto glo1 = ts::Glo::from_day_tod(16750, 85500);
    auto glo2 = ts::Glo::from_day_tod(16750, 85500);
    CHECK(glo1.timestamp() == glo2.timestamp());
}

TEST_CASE("GLONASS time from timestamp round-trip") {
    auto utc  = ts::Utc::from_date_time(2025, 11, 16, 0, 0, 0.0);
    auto glo  = ts::Glo{utc};
    auto glo2 = ts::Glo::from_absolute_day_tod(glo.days(), glo.time_of_day().full_seconds());

    CHECK(glo.timestamp().full_seconds() == doctest::Approx(glo2.timestamp().full_seconds()));
}

TEST_CASE("GLONASS from_absolute_day_tod - no ambiguity") {
    auto glo = ts::Glo::from_absolute_day_tod(10912, 85500);
    auto utc = ts::Utc{glo};

    // 10912 days from GLONASS epoch (1996-01-01) = 2025-11-16
    CHECK(utc.rtklib_time_string(3).substr(0, 10) == "2025/11/16");
}

TEST_CASE("GLONASS from_period_day_tod - resolves 4-year period") {
    // n_t = 685 could be in period 0, 1, 2, 3, 4, 5, 6, 7...
    // Reference time in 2025 should resolve to period 7
    auto glo_ref = ts::Glo{ts::Utc::from_date_time(2025, 11, 16, 0, 0, 0.0)};
    auto glo     = ts::Glo::from_period_day_tod(685, 85500, glo_ref);
    auto utc     = ts::Utc{glo};

    CHECK(utc.rtklib_time_string(3).substr(0, 10) == "2025/11/16");
}

TEST_CASE("GLONASS from_period_day_tod - chooses closest period") {
    auto glo_ref = ts::Glo{ts::Utc::from_date_time(2025, 11, 16, 0, 0, 0.0)};

    // n_t = 0 (start of period) should be close to reference
    auto glo1 = ts::Glo::from_period_day_tod(0, 0, glo_ref);
    CHECK(glo1.days() == 1461 * 7);

    // n_t = 1460 (end of period) should also be close
    auto glo2 = ts::Glo::from_period_day_tod(1460, 0, glo_ref);
    CHECK(glo2.days() == 1461 * 8 - 1);
}

TEST_CASE("GLONASS from_period_day_tod - handles period boundary") {
    // Reference near end of period 7 (2027-11-14 is day 1413 of period 7)
    auto glo_ref = ts::Glo{ts::Utc::from_date_time(2027, 11, 14, 0, 0, 0.0)};

    // n_t = 1460, tod = 0 (midnight Moscow time) = 21:00 UTC previous day
    auto glo = ts::Glo::from_period_day_tod(1460, 0, glo_ref);
    auto utc = ts::Utc{glo};

    CHECK(utc.rtklib_time_string(3).substr(0, 10) == "2027/12/30");
}

TEST_CASE("GLONASS from_period_day_tod - with noon time of day") {
    auto glo_ref = ts::Glo{ts::Utc::from_date_time(2025, 11, 16, 0, 0, 0.0)};

    // n_t = 685, tod = 12h (noon Moscow time) = 09:00 UTC
    auto glo = ts::Glo::from_period_day_tod(685, 12 * 3600, glo_ref);
    auto utc = ts::Utc{glo};

    CHECK(utc.rtklib_time_string(3).substr(0, 16) == "2025/11/16 09:00");
}
