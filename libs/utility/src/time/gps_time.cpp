#include <utility/time.h>

static s64 leap_seconds_utc_gps() {
    // TODO(ewasjon): This will not always be correct. Use LeapSeconds::
    // instead.
    constexpr s64 leap_seconds = 18;
    return leap_seconds;
}

// Calculate the current GPS timestamp from UTC timestamp
// UTC is seconds from: 00:00, Jan 1 1970 (including leap-seconds)
// GPS is seconds from: 00:00, Jan 6 1980
static Timestamp utc_2_gps(Timestamp utc_time) {
    auto leap_seconds      = leap_seconds_utc_gps();
    auto seconds_since_utc = utc_time;
    auto seconds_since_gps = seconds_since_utc;

    // GPS doesn't include leap-seconds
    seconds_since_gps.add(leap_seconds);

    // -5 days because gps time started on the 6th of January 1980
    constexpr auto day_difference = 5;
    seconds_since_gps.subtract(day_difference * DAY_IN_SECONDS);

    // -2 days because leap days between 1970 and 1980
    constexpr auto leap_days = 2;
    seconds_since_gps.subtract(leap_days * DAY_IN_SECONDS);

    // -10 years because gps time started 1980 (1980 - 1970)
    constexpr auto year_difference = 10;
    seconds_since_gps.subtract(year_difference * YEAR_IN_SECONDS);
    return seconds_since_gps;
}

static Timestamp gps_2_utc(Timestamp gst) {
    auto temp = utc_2_gps(gst);
    auto diff = gst - temp;
    assert(diff.fraction() == 0.0);
    return gst + diff;
}

GPS_Time::GPS_Time(s64 day, f64 tod) : tm{} {
    tm.add(DAY_IN_SECONDS * day);
    tm.add(tod);
}

GPS_Time::GPS_Time(s64 week, s64 tow, f64 fractions) : tm{} {
    tm.add(WEEK_IN_SECONDS * week);
    tm.add(tow);
    tm.add(fractions);
}

GPS_Time::GPS_Time(const TAI_Time& time) : GPS_Time(UTC_Time(time)) {}
GPS_Time::GPS_Time(const UTC_Time& time) : tm{utc_2_gps(time.timestamp())} {}

s64 GPS_Time::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

Timestamp GPS_Time::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

s64 GPS_Time::week() const {
    return tm.seconds() / WEEK_IN_SECONDS;
}

Timestamp GPS_Time::time_of_week() const {
    return tm - Timestamp{week() * WEEK_IN_SECONDS};
}

GPS_Time GPS_Time::now() {
    return GPS_Time{UTC_Time::now()};
}

Timestamp GPS_Time::utc_timestamp() const {
    return gps_2_utc(tm);
}
