#include <utility/time.h>
#include <array>
#include <time.h>
#include <sys/time.h>

// NOTE: The day each month of the year starts with.
constexpr static std::array<s64, 12> day_of_year = {
    1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
};

constexpr static s32 gmtime_start_year    = 1900;
constexpr static s64 utc_start_year       = 1970;
constexpr static s64 utc_end_year         = 2099;
constexpr static s64 days_per_year        = 365;
constexpr static f64 microseconds2seconds = 1E-6;

static Timestamp utc_from_date(s64 year, s64 month, s64 day, s64 hour, s64 minutes, s64 seconds) {
    if (year < utc_start_year) {
        year = utc_start_year;
    } else if (year >= utc_end_year) {
        year = utc_end_year - 1;
    }

    if (month <= 0) {
        month = 1;
    } else if (month > 12) {
        month = 12;
    }

    auto days = 0LL;

    for (s64 i = utc_start_year; i < year; i++) {
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

    s64 timestamp = 0LL;
    timestamp += days * DAY_IN_SECONDS;
    timestamp += hour * HOUR_IN_SECONDS;
    timestamp += minutes * MINUTE_IN_SECONDS;
    timestamp += seconds;

    return Timestamp{timestamp};
}

struct TimeEpoch {
    s64 year;
    s64 month;
    s64 day;
    s64 hour;
    s64 minutes;
    f64 seconds;
};

constexpr static s64 MONTH_PER_YEAR = 12;
constexpr static s64 DAYS_PER_4YEAR = DAYS_PER_YEAR * 4 + 1 /* leap day */;
constexpr static std::array<s64, 4 * MONTH_PER_YEAR> MONTH_DAYS = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

// TODO(ewasjon): this is not really tested very well
static TimeEpoch date_from_utc(Timestamp time) {
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

    auto year = utc_start_year;
    year += 4 * (days / DAYS_PER_4YEAR);
    year += month / MONTH_PER_YEAR;

    month %= MONTH_PER_YEAR;

    auto tod     = time.seconds() - days * DAY_IN_SECONDS;
    auto hour    = tod / HOUR_IN_SECONDS;
    auto toh     = tod - hour * HOUR_IN_SECONDS;
    auto minutes = toh / MINUTE_IN_SECONDS;
    auto tom     = toh - minutes * MINUTE_IN_SECONDS;
    auto seconds = tom;

    return TimeEpoch{
        .year    = year,
        .month   = month,
        .day     = day,
        .hour    = hour,
        .minutes = minutes,
        .seconds = seconds + time.fraction(),
    };
}

UTC_Time::UTC_Time(s64 days, f64 tod) : tm(static_cast<f64>(days * DAY_IN_SECONDS) + tod) {}
UTC_Time::UTC_Time(const TAI_Time& time) : tm(time.utc_timestamp()) {}
UTC_Time::UTC_Time(const GPS_Time& time) : tm(time.utc_timestamp()) {}
UTC_Time::UTC_Time(const GLO_Time& time) : tm(time.utc_timestamp()) {}
UTC_Time::UTC_Time(const GST_Time& time) : tm(time.utc_timestamp()) {}
UTC_Time::UTC_Time(const BDT_Time& time) : tm(time.utc_timestamp()) {}

s64 UTC_Time::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

std::string UTC_Time::rtklib_time_string() const {
    constexpr int fraction_digits = 12;

    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);

    std::array<char, 256> buffer;
    snprintf(buffer.data(), buffer.size(),
             "%04" PRId64 "/%02" PRId64 "/%02" PRId64 " %02" PRId64 ":%02" PRId64 ":%0*.*f",
             epoch.year, epoch.month + 1, epoch.day + 1, epoch.hour, epoch.minutes,
             fraction_digits + 3, fraction_digits, epoch.seconds);

    return std::string{buffer.data()};
}

UTC_Time UTC_Time::now() {
    struct timeval tv {};
    struct tm      tm {};

    gettimeofday(&tv, nullptr);
    auto tt = gmtime_r(&tv.tv_sec, &tm);

    auto year      = tt->tm_year + gmtime_start_year;
    auto month     = tt->tm_mon + 1;
    auto day       = tt->tm_mday;
    auto hour      = tt->tm_hour;
    auto minutes   = tt->tm_min;
    auto seconds   = tt->tm_sec;
    auto fractions = static_cast<f64>(tv.tv_usec) * microseconds2seconds;

    auto timestamp = utc_from_date(year, month, day, hour, minutes, seconds);
    timestamp.add(fractions);

    return UTC_Time(timestamp);
}
