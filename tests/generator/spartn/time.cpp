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
//   raw=480819870  OCB-0 GPS
//   raw=480819870  OCB-2 GAL
//   raw=480819856  OCB-3 BDS
//   raw=480830652  OCB-1 GLO
//
// All correspond to the same physical moment. The LPP server sends all
// epochTime fields in GPS time (gnss_TimeID=GPS). spartn_time_from() returns
// GPS-based seconds; spartn_time_for_gnss() converts for the output message.
//
// GPS day/tod derived from raw=480819870:
//   days_since_2010 = 480819870 / 86400 = 5565, tod = 480819870 % 86400 = 3870
//   GPS day = 5565 + 10953 (SPARTN_EPOCH_GPS_DAYS) = 16518
// =============================================================================

static constexpr long GPS_DAY = 16518;
static constexpr long GPS_TOD = 3870;  // 01:04:30 GPS

TEST_CASE("spartn_time_from: GPS input returns GPS-based seconds") {
    auto st     = make_system_time(GNSS_ID__gnss_id_gps, GPS_DAY, GPS_TOD);
    auto result = spartn_time_from(st);
    CHECK(result.rounded_seconds == 480819870u);
}

TEST_CASE("spartn_time_for_gnss: GPS output unchanged") {
    auto st  = make_system_time(GNSS_ID__gnss_id_gps, GPS_DAY, GPS_TOD);
    auto gps = spartn_time_from(st);
    CHECK(spartn_time_for_gnss(gps, GNSS_ID__gnss_id_gps) == 480819870u);
}

TEST_CASE("spartn_time_for_gnss: GAL output same as GPS") {
    auto st  = make_system_time(GNSS_ID__gnss_id_gps, GPS_DAY, GPS_TOD);
    auto gps = spartn_time_from(st);
    CHECK(spartn_time_for_gnss(gps, GNSS_ID__gnss_id_galileo) == 480819870u);
}

TEST_CASE("spartn_time_for_gnss: BDS output is GPS - 14s") {
    auto st  = make_system_time(GNSS_ID__gnss_id_gps, GPS_DAY, GPS_TOD);
    auto gps = spartn_time_from(st);
    CHECK(spartn_time_for_gnss(gps, GNSS_ID__gnss_id_bds) == 480819856u);
}

TEST_CASE("spartn_time_for_gnss: GLO output is GPS + 10782s") {
    auto st  = make_system_time(GNSS_ID__gnss_id_gps, GPS_DAY, GPS_TOD);
    auto gps = spartn_time_from(st);
    CHECK(spartn_time_for_gnss(gps, GNSS_ID__gnss_id_glonass) == 480830652u);
}
