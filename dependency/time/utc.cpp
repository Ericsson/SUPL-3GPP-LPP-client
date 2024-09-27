#include "utc.hpp"
#include "bdt.hpp"
#include "glo.hpp"
#include "gps.hpp"
#include "gst.hpp"
#include "tai.hpp"

#include <array>
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>

namespace ts {

// NOTE: The day each month of the year starts with.
constexpr static std::array<int64_t, 12> day_of_year = {
    1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
};

constexpr static int32_t gmtime_start_year    = 1900;
constexpr static int64_t utc_start_year       = 1970;
constexpr static int64_t utc_end_year         = 2099;
constexpr static int64_t days_per_year        = 365;
constexpr static double  microseconds2seconds = 1E-6;

static Timestamp utc_from_date(int64_t year, int64_t month, int64_t day, int64_t hour,
                               int64_t minutes, int64_t seconds) {
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

    for (auto i = utc_start_year; i < year; i++) {
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

    int64_t timestamp = 0LL;
    timestamp += days * DAY_IN_SECONDS;
    timestamp += hour * HOUR_IN_SECONDS;
    timestamp += minutes * MINUTE_IN_SECONDS;
    timestamp += seconds;

    return Timestamp{timestamp};
}

struct TimeEpoch {
    int64_t year;
    int64_t month;
    int64_t day;
    int64_t hour;
    int64_t minutes;
    double  seconds;
};

constexpr static int64_t MONTH_PER_YEAR = 12;
constexpr static int64_t DAYS_PER_4YEAR = DAYS_PER_YEAR * 4 + 1 /* leap day */;
constexpr static std::array<int64_t, 4 * MONTH_PER_YEAR> MONTH_DAYS = {
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

    TimeEpoch epoch{};
    epoch.year    = year;
    epoch.month   = month;
    epoch.day     = day;
    epoch.hour    = hour;
    epoch.minutes = minutes;
    epoch.seconds = static_cast<double>(seconds) + time.fraction();
    return epoch;
}

Utc::Utc() = default;
Utc::Utc(Timestamp const& timestamp) : tm{timestamp} {}
Utc::Utc(Tai const& time) : tm(time.utc_timestamp()) {}
Utc::Utc(Gps const& time) : tm(time.utc_timestamp()) {}
Utc::Utc(Glo const& time) : tm(time.utc_timestamp()) {}
Utc::Utc(Gst const& time) : tm(time.utc_timestamp()) {}
Utc::Utc(Bdt const& time) : tm(time.utc_timestamp()) {}

int64_t Utc::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

double Utc::day_of_year() const {
    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);
    auto start = utc_from_date(epoch.year, 0, 0, 0, 0, 0);
    return (ts - start).full_seconds() / DAY_IN_SECONDS;
}

std::string Utc::rtklib_time_string() const {
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

Utc Utc::now() {
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
    auto fractions = static_cast<double>(tv.tv_usec) * microseconds2seconds;

    auto timestamp = utc_from_date(year, month, day, hour, minutes, seconds);
    timestamp.add(fractions);

    return Utc(timestamp);
}

Utc Utc::from_day_tod(int64_t day, double tod) {
    return Utc(Timestamp{static_cast<double>(day * DAY_IN_SECONDS) + tod});
}

}  // namespace ts
