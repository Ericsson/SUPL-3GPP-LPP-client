#include <doctest/doctest.h>
#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <GNSS-ID.h>
#include <GNSS-SystemTime.h>
EXTERNAL_WARNINGS_POP

struct SpartnTime {
    double   seconds;
    uint32_t rounded_seconds;
};

// Current implementation from dependency/generator/spartn/time.cpp
namespace current_impl {

static constexpr uint32_t SECONDS_IN_DAY        = 86400;
static constexpr uint64_t DAY_BETWEEN_1970_1980 = 3657;
static constexpr uint64_t DAY_BETWEEN_1970_1996 = 9496;
static constexpr uint64_t DAY_BETWEEN_1970_1999 = 10825;
static constexpr uint64_t DAY_BETWEEN_1970_2006 = 13149;
static constexpr uint64_t DAY_BETWEEN_1970_2010 = 14610;

static uint64_t standard_day_number(long gnss_id, long day_number) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_1980;
    case GNSS_ID__gnss_id_galileo: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_1999;
    case GNSS_ID__gnss_id_glonass: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_1996;
    case GNSS_ID__gnss_id_bds: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_2006;
    default: return 0;
    }
}

SpartnTime spartn_time_from(GNSS_SystemTime const& epoch_time) {
    auto time_of_day = static_cast<double>(epoch_time.gnss_TimeOfDay);
    if (epoch_time.gnss_TimeOfDayFrac_msec) {
        time_of_day += static_cast<double>(*epoch_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    }

    auto day_number      = epoch_time.gnss_DayNumber;
    auto gnss_id         = epoch_time.gnss_TimeID.gnss_id;
    auto days_since_1970 = standard_day_number(gnss_id, day_number);
    auto days_since_2010 = days_since_1970 - DAY_BETWEEN_1970_2010;

    auto seconds_since_2010  = days_since_2010 * SECONDS_IN_DAY;
    auto rounded_time_of_day = static_cast<uint32_t>(time_of_day + 0.5);
    auto seconds             = static_cast<uint32_t>(seconds_since_2010) + rounded_time_of_day;

    return SpartnTime{
        static_cast<double>(seconds_since_2010) + time_of_day,
        seconds,
    };
}

}  // namespace current_impl

// Fixed implementation using time utilities
namespace fixed_impl {

// SPARTN epoch: Jan 1, 2010 00:00:00 GPS time
// = 14610 (days 1970->2010) - 3657 (days 1970->GPS epoch) = 10953 days from GPS epoch
static constexpr int64_t SPARTN_EPOCH_GPS_DAYS = 10953;

SpartnTime spartn_time_from(GNSS_SystemTime const& epoch_time) {
    auto day_number  = epoch_time.gnss_DayNumber;
    auto time_of_day = static_cast<double>(epoch_time.gnss_TimeOfDay);
    if (epoch_time.gnss_TimeOfDayFrac_msec) {
        time_of_day += static_cast<double>(*epoch_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    }

    ts::Gps gps_time;
    switch (epoch_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: gps_time = ts::Gps::from_day_tod(day_number, time_of_day); break;
    case GNSS_ID__gnss_id_galileo:
        gps_time = ts::Gps(ts::Gst::from_day_tod(day_number, time_of_day));
        break;
    case GNSS_ID__gnss_id_glonass:
        gps_time = ts::Gps(ts::Glo::from_day_tod(day_number, time_of_day));
        break;
    case GNSS_ID__gnss_id_bds:
        gps_time = ts::Gps(ts::Bdt::from_day_tod(day_number, time_of_day));
        break;
    default: return SpartnTime{0, 0};
    }

    auto days_since_2010    = gps_time.days() - SPARTN_EPOCH_GPS_DAYS;
    auto seconds_since_2010 = static_cast<double>(days_since_2010 * ts::DAY_IN_SECONDS) +
                              gps_time.time_of_day().as_double();

    return SpartnTime{
        seconds_since_2010,
        static_cast<uint32_t>(seconds_since_2010 + 0.5),
    };
}

}  // namespace fixed_impl

// Helper to create GNSS_SystemTime
static GNSS_SystemTime make_system_time(long gnss_id, long day, long tod,
                                        long* frac_msec = nullptr) {
    GNSS_SystemTime st{};
    st.gnss_TimeID.gnss_id     = gnss_id;
    st.gnss_DayNumber          = day;
    st.gnss_TimeOfDay          = tod;
    st.gnss_TimeOfDayFrac_msec = frac_msec;
    return st;
}

// =============================================================================
// NO REGRESSION TESTS - Fixed impl must match current impl for GPS/GAL/GLO
// =============================================================================

TEST_CASE("No regression: GPS - fixed matches current") {
    for (int week = 2000; week <= 2400; week += 100) {
        for (int tow = 0; tow <= 600000; tow += 100000) {
            auto gps     = ts::Gps::from_week_tow(week, tow, 0.0);
            auto st      = make_system_time(GNSS_ID__gnss_id_gps, gps.days(),
                                            static_cast<long>(gps.time_of_day().seconds()));
            auto current = current_impl::spartn_time_from(st);
            auto fixed   = fixed_impl::spartn_time_from(st);

            INFO("GPS week=", week, " tow=", tow);
            CHECK(current.rounded_seconds == fixed.rounded_seconds);
        }
    }
}

TEST_CASE("No regression: Galileo - fixed matches current") {
    for (int week = 1000; week <= 1400; week += 100) {
        for (int tow = 0; tow <= 600000; tow += 100000) {
            auto gst     = ts::Gst::from_week_tow(week, tow, 0.0);
            auto st      = make_system_time(GNSS_ID__gnss_id_galileo, gst.days(),
                                            static_cast<long>(gst.time_of_day().seconds()));
            auto current = current_impl::spartn_time_from(st);
            auto fixed   = fixed_impl::spartn_time_from(st);

            INFO("GAL week=", week, " tow=", tow);
            CHECK(current.rounded_seconds == fixed.rounded_seconds);
        }
    }
}

TEST_CASE("No regression: GLONASS - current impl is BUGGY (ignores UTC+3 and leap seconds)") {
    // GLONASS time = UTC + 3 hours (Moscow time)
    // Current implementation ignores this offset and leap seconds
    // The difference should be approximately 3h - leap_seconds ≈ 10782 seconds

    auto glo = ts::Glo::from_day_tod(8000, 0);
    auto st  = make_system_time(GNSS_ID__gnss_id_glonass, 8000, 0);

    auto current = current_impl::spartn_time_from(st);
    auto fixed   = fixed_impl::spartn_time_from(st);

    auto diff =
        static_cast<int64_t>(current.rounded_seconds) - static_cast<int64_t>(fixed.rounded_seconds);

    INFO("Current: ", current.rounded_seconds);
    INFO("Fixed: ", fixed.rounded_seconds);
    INFO("Difference: ", diff, " seconds (expected ~10782 = 3h - 18 leap seconds)");

    // Current impl is wrong by approximately 3 hours minus leap seconds
    // This is because it ignores the Moscow timezone offset
    CHECK(diff > 10000);  // Approximately 3 hours
    CHECK(diff < 11000);
}

// =============================================================================
// BUG FIX TEST - Same physical instant must produce same SPARTN time
// =============================================================================

TEST_CASE("Bug fix: BDS time offset corrected") {
    // Test multiple time points
    for (int week = 2200; week <= 2400; week += 50) {
        for (int tow = 0; tow <= 600000; tow += 150000) {
            // Create GPS time as reference
            auto gps_time = ts::Gps::from_week_tow(week, tow, 0.0);

            // Convert to BDS time (same physical instant)
            auto bds_time = ts::Bdt(gps_time);

            // Create system times
            auto gps_st = make_system_time(GNSS_ID__gnss_id_gps, gps_time.days(),
                                           static_cast<long>(gps_time.time_of_day().seconds()));
            auto bds_st = make_system_time(GNSS_ID__gnss_id_bds, bds_time.days(),
                                           static_cast<long>(bds_time.time_of_day().seconds()));

            // Current impl has the bug - 14 second difference
            auto gps_current  = current_impl::spartn_time_from(gps_st);
            auto bds_current  = current_impl::spartn_time_from(bds_st);
            auto current_diff = static_cast<int64_t>(gps_current.rounded_seconds) -
                                static_cast<int64_t>(bds_current.rounded_seconds);

            // Fixed impl should produce same result
            auto gps_fixed = fixed_impl::spartn_time_from(gps_st);
            auto bds_fixed = fixed_impl::spartn_time_from(bds_st);

            INFO("week=", week, " tow=", tow);
            INFO("Current: GPS=", gps_current.rounded_seconds, " BDS=", bds_current.rounded_seconds,
                 " diff=", current_diff);
            INFO("Fixed: GPS=", gps_fixed.rounded_seconds, " BDS=", bds_fixed.rounded_seconds);

            // Verify current impl has the 14-second bug
            CHECK(current_diff == 14);

            // Verify fixed impl produces matching times
            CHECK(gps_fixed.rounded_seconds == bds_fixed.rounded_seconds);
        }
    }
}

// =============================================================================
// FRACTIONAL SECONDS TEST
// =============================================================================

TEST_CASE("Debug: understand the epoch difference") {
    auto gps = ts::Gps::from_week_tow(2000, 0, 0.0);

    // What ts::Gps::days() returns
    auto gps_days = gps.days();

    // What current impl computes for this GPS time
    // Input: gnss_DayNumber from LPP = days since Jan 6, 1980 = gps_days
    // current_impl: days_since_1970 = gps_days + 3657
    //               days_since_2010 = days_since_1970 - 14610 = gps_days - 10953
    auto current_days_since_2010 = gps_days - 10953;
    auto current_seconds         = current_days_since_2010 * 86400;

    INFO("gps.days() = ", gps_days);
    INFO("current_days_since_2010 = ", current_days_since_2010);
    INFO("current_seconds = ", current_seconds);

    // Now test with actual current_impl
    auto st     = make_system_time(GNSS_ID__gnss_id_gps, gps_days, 0);
    auto result = current_impl::spartn_time_from(st);

    INFO("current_impl result = ", result.rounded_seconds);
    INFO("difference = ", static_cast<int64_t>(result.rounded_seconds) - current_seconds);

    CHECK(result.rounded_seconds == static_cast<uint32_t>(current_seconds));
}
