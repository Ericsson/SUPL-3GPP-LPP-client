#include <doctest/doctest.h>
#include <ephemeris/gps.hpp>
#include <time/gps.hpp>

TEST_CASE("GPS ephemeris - is_valid week crossover") {
    // Real scenario from G25 in dataset 20260606_224927:
    // Ephemeris: week 2422, TOE = 0 (= 2026-06-07 00:00:00, start of week 2422)
    // Query time: 2026-06-06 23:35:30 (= week 2421, TOW = 604530)
    // The ephemeris should be valid (23:35 is only 24.5 min before TOE, well within ±2h)
    // but the week number check rejects it.

    ephemeris::GpsEphemeris eph{};
    eph.prn               = 25;
    eph.week_number       = 2422;  // week containing TOE
    eph.toe               = 0.0;   // start of week 2422 = 2026-06-07 00:00:00
    eph.toc               = 0.0;
    eph.fit_interval_flag = false;                          // ±4h validity window
    eph.a                 = 5153682.0 * 5153682.0 * 1e-38;  // placeholder

    // Query at 23:35:30 on June 6 = week 2421, TOW = 6*86400 + 23*3600 + 35*60 + 30 = 604530
    auto query_time = ts::Gps::from_week_tow(2421, 604530, 0.0);

    // This SHOULD be valid: the time difference is only -1470 seconds (24.5 minutes before TOE)
    // which is well within the ±4h (14400s) fit interval.
    CHECK(eph.is_valid(query_time));
}
