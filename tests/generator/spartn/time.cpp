#include <doctest/doctest.h>
#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>

#include "time.hpp"

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <GNSS-ID.h>
#include <GNSS-SystemTime.h>
EXTERNAL_WARNINGS_POP

static GNSS_SystemTime make_system_time(long gnss_id, long day, long tod) {
    GNSS_SystemTime st{};
    st.gnss_TimeID.gnss_id     = gnss_id;
    st.gnss_DayNumber          = day;
    st.gnss_TimeOfDay          = tod;
    st.gnss_TimeOfDayFrac_msec = nullptr;
    return st;
}

// =============================================================================
// Reference values from a real SPARTN capture (2025-03-28):
//   2025-03-28T01:04:30+00:00  raw=480819870  OCB-0 GPS
//   2025-03-28T01:04:30+00:00  raw=480819870  OCB-2 GAL
//   2025-03-28T01:04:16+00:00  raw=480819856  OCB-3 BDS
//   2025-03-28T04:04:12+00:00  raw=480830652  OCB-1 GLO
//
// All correspond to UTC 2025-03-28 01:04:12. The displayed UTC times in the
// capture header are the decoded wall-clock times per system, not the raw input.
// =============================================================================

// GPS: day from GPS epoch (Jan 6 1980), tod in GPS time (UTC+18s)
static constexpr long GPS_DAY = 16518;
static constexpr long GPS_TOD = 3870;  // 01:04:30 GPS

// GST: day from GST epoch (Aug 22 1999), tod same rate as GPS
static constexpr long GST_DAY = 9350;
static constexpr long GST_TOD = 3870;

// BDT: day from BDT epoch (Jan 1 2006), tod in BDT time (GPS-14s)
static constexpr long BDT_DAY = 7026;
static constexpr long BDT_TOD = 3856;  // 01:04:16 BDT

// GLO: day from GLO epoch (Jan 1 1996), tod in GLO time (UTC+3h)
static constexpr long GLO_DAY = 10679;
static constexpr long GLO_TOD = 14652;  // 04:04:12 GLO

TEST_CASE("SPARTN time tag: GPS matches real capture") {
    auto st     = make_system_time(GNSS_ID__gnss_id_gps, GPS_DAY, GPS_TOD);
    auto result = spartn_time_from(st);
    CHECK(result.rounded_seconds == 480819870u);
}

TEST_CASE("SPARTN time tag: Galileo matches real capture") {
    auto st     = make_system_time(GNSS_ID__gnss_id_galileo, GST_DAY, GST_TOD);
    auto result = spartn_time_from(st);
    CHECK(result.rounded_seconds == 480819870u);
}

TEST_CASE("SPARTN time tag: BDS matches real capture") {
    auto st     = make_system_time(GNSS_ID__gnss_id_bds, BDT_DAY, BDT_TOD);
    auto result = spartn_time_from(st);
    CHECK(result.rounded_seconds == 480819856u);
}

TEST_CASE("SPARTN time tag: GLONASS matches real capture") {
    auto st     = make_system_time(GNSS_ID__gnss_id_glonass, GLO_DAY, GLO_TOD);
    auto result = spartn_time_from(st);
    CHECK(result.rounded_seconds == 480830652u);
}

TEST_CASE("SPARTN time tag: BDS is 14s less than GPS for same physical moment") {
    // BDT = GPS - 14s, so BDT's 2010 epoch starts 14s later => BDT counter is 14 less
    auto gps_time = ts::Gps::from_week_tow(2300, 300000, 0.0);
    auto bdt_time = ts::Bdt(gps_time);

    auto gps_st = make_system_time(GNSS_ID__gnss_id_gps, gps_time.days(),
                                   static_cast<long>(gps_time.time_of_day().seconds()));
    auto bdt_st = make_system_time(GNSS_ID__gnss_id_bds, bdt_time.days(),
                                   static_cast<long>(bdt_time.time_of_day().seconds()));

    auto gps_result = spartn_time_from(gps_st);
    auto bdt_result = spartn_time_from(bdt_st);

    CHECK(static_cast<int64_t>(gps_result.rounded_seconds) -
              static_cast<int64_t>(bdt_result.rounded_seconds) ==
          14);
}

TEST_CASE("SPARTN time tag: GLO is ~10782s more than GPS for same physical moment") {
    // GLO = UTC+3h, GPS = UTC+18s => GLO epoch is ~10782s earlier => GLO counter is ~10782 more
    // Exact value depends on leap seconds at the reference epoch
    auto gps_time = ts::Gps::from_week_tow(2300, 300000, 0.0);
    auto glo_time = ts::Glo(gps_time);

    auto gps_st = make_system_time(GNSS_ID__gnss_id_gps, gps_time.days(),
                                   static_cast<long>(gps_time.time_of_day().seconds()));
    auto glo_st = make_system_time(GNSS_ID__gnss_id_glonass, glo_time.days(),
                                   static_cast<long>(glo_time.time_of_day().seconds()));

    auto gps_result = spartn_time_from(gps_st);
    auto glo_result = spartn_time_from(glo_st);

    auto diff = static_cast<int64_t>(glo_result.rounded_seconds) -
                static_cast<int64_t>(gps_result.rounded_seconds);

    // 3h = 10800s, minus 18 leap seconds (GPS-UTC at this epoch) = 10782
    CHECK(diff == 10782);
}
