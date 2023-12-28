#include "gst_time.hpp"
#include "utc_time.hpp"

namespace ts {
static TsInt leap_seconds_utc_gst() {
    // NOTE(ewasjon): The leapseconds difference between UTC and GST is 13 (because of the begining)
    // and the difference between in leapseconds sinec the start (which is 5 as of 2022:09:22). Or
    // in other words 13 + (TAI - UTC) - 32.
    auto leap_seconds = 13 + (LeapSeconds::count() - 32);
    return leap_seconds;
}

// Calculate the current GPS timestamp from UTC timestamp
// UTC is seconds from: 00:00:00, Jan 1  1970 (including leap-seconds)
// GST is seconds from: 00:00:00, Aug 22 1999
static Timestamp utc_2_gst(Timestamp utc_time) {
    auto leap_seconds      = leap_seconds_utc_gst();
    auto seconds_since_utc = utc_time;
    auto seconds_since_gst = seconds_since_utc;

    // At 00:00:00, Aug 22 1999 GST was ahead of UTC by 13 leapseconds. If UTC == GST(T0) then we
    // -13 seconds. Thus we need to add the difference.
    seconds_since_gst.add(leap_seconds);

    // -233 days because GST time started on the 22nd of August 1999
    constexpr auto days_difference = 233;
    seconds_since_gst.subtract(days_difference * DAY_IN_SECONDS);

    // -7 leap days between 1970 and 1999.
    // (1974, 1978, 1982, 1986, 1990, 1994, 1998)
    constexpr auto leap_days = 7;
    seconds_since_gst.subtract(leap_days * DAY_IN_SECONDS);

    // -29 years because gps time started 1999 (1999 - 1970)
    constexpr auto year_difference = 29;
    seconds_since_gst.subtract(year_difference * YEAR_IN_SECONDS);
    return seconds_since_gst;
}

static Timestamp gst_2_utc(Timestamp gst) {
    auto temp = utc_2_gst(gst);
    auto diff = gst - temp;
    return gst + diff;
}

GST_Time::GST_Time(TsInt day, TsFloat tod) : tm{} {
    tm.add(DAY_IN_SECONDS * day);
    tm.add(tod);
}

GST_Time::GST_Time(TsInt week, TsInt tow, TsFloat fractions) : tm{} {
    tm.add(WEEK_IN_SECONDS * week);
    tm.add(tow);
    tm.add(fractions);
}

GST_Time::GST_Time(const TAI_Time& time) : GST_Time(UTC_Time(time)) {}

GST_Time::GST_Time(const UTC_Time& time) {
    tm = utc_2_gst(time.timestamp());
}

TsInt GST_Time::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

Timestamp GST_Time::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

TsInt GST_Time::week() const {
    return tm.seconds() / WEEK_IN_SECONDS;
}

Timestamp GST_Time::time_of_week() const {
    return tm - Timestamp{week() * WEEK_IN_SECONDS};
}

GST_Time GST_Time::now() {
    return GST_Time{UTC_Time::now()};
}

Timestamp GST_Time::utc_timestamp() const {
    return gst_2_utc(tm);
}
}  // namespace ts
