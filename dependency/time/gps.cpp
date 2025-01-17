#include "gps.hpp"
#include "bdt.hpp"
#include "glo.hpp"
#include "gst.hpp"
#include "tai.hpp"
#include "utc.hpp"

#include <array>
#include <stdio.h>

namespace ts {

static int64_t leap_seconds_utc_gps() {
    // TODO(ewasjon): This will not always be correct. Use LeapSeconds::
    // instead.
    constexpr int64_t leap_seconds = 18;
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

Gps::Gps() = default;
Gps::Gps(Timestamp const& timestamp) : tm{timestamp} {}
Gps::Gps(Utc const& time) : tm{utc_2_gps(time.timestamp())} {}
Gps::Gps(Tai const& time) : Gps(Utc(time)) {}
Gps::Gps(Glo const& time) : Gps(Utc(time)) {}
Gps::Gps(Bdt const& time) : Gps(Utc(time)) {}
Gps::Gps(Gst const& time) : Gps(Utc(time)) {}

int64_t Gps::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

Timestamp Gps::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

int64_t Gps::week() const {
    return tm.seconds() / WEEK_IN_SECONDS;
}

Timestamp Gps::time_of_week() const {
    return tm - Timestamp{week() * WEEK_IN_SECONDS};
}

NODISCARD Timestamp Gps::mod_timestamp() const {
    auto timestamp = tm;
    while(timestamp.seconds() > WEEK_IN_SECONDS * 1024) {
        timestamp.subtract(WEEK_IN_SECONDS * 1024);
    }
    return timestamp;
}

Timestamp Gps::difference(Gps const& other) const {
    return tm - other.tm;
}

constexpr static int64_t MONTH_PER_YEAR = 12;
constexpr static int64_t DAYS_PER_4YEAR = DAYS_PER_YEAR * 4 + 1 /* leap day */;
constexpr static std::array<int64_t, 4 * MONTH_PER_YEAR> MONTH_DAYS = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

constexpr static int64_t GPS_START_YEAR = 1980;

// TODO(ewasjon): this is not really tested very well
static TimePoint timepoint_from_timestamp(Timestamp time) {
    time.normalize();
    auto days = time.seconds() / DAY_IN_SECONDS;

    auto month = 0u;
    auto day   = days % DAYS_PER_4YEAR;
    for (; month < MONTH_DAYS.size(); month++) {
        if (day < MONTH_DAYS[month]) {
            break;
        }

        day -= MONTH_DAYS[month];
    }

    auto year = GPS_START_YEAR;
    year += 4 * (days / DAYS_PER_4YEAR);
    year += month / MONTH_PER_YEAR;

    month %= MONTH_PER_YEAR;

    auto tod     = time.seconds() - days * DAY_IN_SECONDS;
    auto hour    = tod / HOUR_IN_SECONDS;
    auto toh     = tod - hour * HOUR_IN_SECONDS;
    auto minutes = toh / MINUTE_IN_SECONDS;
    auto tom     = toh - minutes * MINUTE_IN_SECONDS;
    auto seconds = tom;

    TimePoint epoch{};
    epoch.year    = year;
    epoch.month   = month + 1;
    epoch.day     = day + 1;
    epoch.hour    = hour;
    epoch.minutes = minutes;
    epoch.seconds = static_cast<double>(seconds) + time.fraction();
    return epoch;
}

TimePoint Gps::to_timepoint() const {
    return timepoint_from_timestamp(tm);
}


Gps Gps::now() {
    return Gps{Utc::now()};
}

Gps Gps::from_day_tod(int64_t day, double tod) {
    Timestamp timestamp{};
    timestamp.add(DAY_IN_SECONDS * day);
    timestamp.add(tod);
    return Gps{timestamp};
}

Gps Gps::from_week_tow(int64_t week, int64_t tow, double fractions) {
    Timestamp timestamp{};
    timestamp.add(WEEK_IN_SECONDS * week);
    timestamp.add(tow);
    timestamp.add(fractions);
    return Gps{timestamp};
}

Gps Gps::from_ymdhms(int64_t year, int64_t month, int64_t day, int64_t hour, int64_t min,
                     double seconds) {
    auto int_seconds = static_cast<int64_t>(seconds);
    auto fractions   = seconds - static_cast<double>(int_seconds);

    auto days    = days_from_ymd(year, month, day);
    auto week    = days / 7;
    auto weekday = days % 7;
    auto tod     = hour * HOUR_IN_SECONDS + min * MINUTE_IN_SECONDS + int_seconds;
    auto tow     = weekday * DAY_IN_SECONDS + tod;

    return from_week_tow(week, tow, fractions);
}

// NOTE: The day each month of the year starts with.
constexpr static std::array<int64_t, 12> day_of_year = {
    1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
};

constexpr static int64_t gps_start_year = 1980;
constexpr static int64_t days_per_year  = 365;

int64_t Gps::days_from_ymd(int64_t year, int64_t month, int64_t day) {
    if (year < gps_start_year) {
        year = gps_start_year;
    }

    if (month <= 0) {
        month = 1;
    } else if (month > 12) {
        month = 12;
    }

    auto days = 0LL;

    for (auto i = gps_start_year; i < year; i++) {
        if (i % 4 == 0) {
            days += days_per_year + 1;
        } else {
            days += days_per_year;
        }
    }

    days += day_of_year.at(static_cast<size_t>(month - 1)) - 1;
    days += day;
    days += -1;
    days += (year % 4 == 0 && month >= 3 ? 1 : 0);

    return days;
}

Timestamp Gps::utc_timestamp() const {
    return gps_2_utc(tm);
}

}  // namespace ts
