#include "utc.hpp"
#include "bdt.hpp"
#include "glo.hpp"
#include "gps.hpp"
#include "gst.hpp"
#include "tai.hpp"

#include <array>
#include <cinttypes>
#include <cmath>
#include <ctime>
#include <sys/time.h>

namespace ts {

// NOTE: The day each month of the year starts with.
constexpr static std::array<int64_t, 12> DAY_OF_YEAR = {
    1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335,
};

constexpr static int32_t GMTIME_START_YEAR       = 1900;
constexpr static int64_t UTC_START_YEAR          = 1970;
constexpr static int64_t UTC_END_YEAR            = 2099;
constexpr static double  MICROSECONDS_TO_SECONDS = 1E-6;

static Timestamp utc_from_date(int64_t year, int64_t month, int64_t day, int64_t hour,
                               int64_t minutes, double seconds) {
    assert(year >= UTC_START_YEAR && year < UTC_END_YEAR);
    assert(month > 0 && month <= 12);
    assert(day > 0 && day <= 31);
#if 0
    if (year < UTC_START_YEAR) {
        year = UTC_START_YEAR;
    } else if (year >= UTC_END_YEAR) {
        year = UTC_END_YEAR - 1;
    }

    if (month <= 0) {
        month = 1;
    } else if (month > 12) {
        month = 12;
    }
#endif

    auto days = 0LL;

    for (auto i = UTC_START_YEAR; i < year; i++) {
        if (i % 4 == 0) {
            days += DAYS_PER_YEAR + 1;
        } else {
            days += DAYS_PER_YEAR;
        }
    }

    days += DAY_OF_YEAR.at(static_cast<size_t>(month - 1)) - 1;
    days += day;
    days += -1;
    days += (year % 4 == 0 && month >= 3 ? 1 : 0);

    int64_t timestamp = 0LL;
    timestamp += days * DAY_IN_SECONDS;
    timestamp += hour * HOUR_IN_SECONDS;
    timestamp += minutes * MINUTE_IN_SECONDS;

    int64_t seconds_i = static_cast<int64_t>(seconds);
    timestamp += seconds_i;

    double fraction = seconds - static_cast<double>(seconds_i);
    return Timestamp{timestamp, fraction};
}

constexpr static int64_t MONTH_PER_YEAR = 12;
constexpr static int64_t DAYS_PER_4YEAR = DAYS_PER_YEAR * 4 + 1 /* leap day */;
constexpr static std::array<int64_t, 4 * MONTH_PER_YEAR> MONTH_DAYS = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,  //
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

// TODO(ewasjon): this is not really tested very well
static TimePoint date_from_utc(Timestamp time) {
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

    auto year = UTC_START_YEAR;
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

Utc::Utc() = default;
Utc::Utc(Timestamp const& timestamp) : mTm{timestamp} {}
Utc::Utc(Tai const& time) : mTm(time.utc_timestamp()) {}
Utc::Utc(Gps const& time) : mTm(time.utc_timestamp()) {}
Utc::Utc(Glo const& time) : mTm(time.utc_timestamp()) {}
Utc::Utc(Gst const& time) : mTm(time.utc_timestamp()) {}
Utc::Utc(Bdt const& time) : mTm(time.utc_timestamp()) {}

int64_t Utc::days() const {
    return mTm.seconds() / DAY_IN_SECONDS;
}

double Utc::day_of_year() const {
    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);
    auto start = utc_from_date(epoch.year, 1, 1, 0, 0, 0);
    return (ts - start).full_seconds() / DAY_IN_SECONDS;
}

Timestamp Utc::ut1(double ut1_utc) const {
    return mTm + Timestamp{ut1_utc};
}

// https://www2.mps.mpg.de/homes/fraenz/systems/systems3art/node10.html
double Utc::gmst(double ut1_utc) const {
    auto jd_ut1 = julian_date(ut1_utc);
    auto jd_0   = floor(jd_ut1) - 0.5;  // JD at 0h UT1 (-0.5 because JD starts at noon)

    // Time in Julian centuries since J2000 at 0h UT1
    auto t_0 = (jd_0 - 2451545.0) / 36525.0;

    // Time in Julian centuries since J2000 at UT1
    auto t = (jd_ut1 - 2451545.0) / 36525.0;

    // GMST at 0h UT1
    auto gmst0 = 24110.54841;
    gmst0 += 8640184.812866 * t_0;
    gmst0 += 0.093104 * t * t;
    gmst0 -= 6.2e-6 * t * t * t;

    // GMST at UT1
    auto ut1  = (jd_ut1 - jd_0) * 86400.0;  // seconds difference between 0h UT1 and UT1
    auto gmst = gmst0 + 1.002737909350795 * ut1;
    return fmod(gmst, 86400.0) * 3.1415926535897932 / 43200.0;
}

std::string Utc::rtklib_time_string(int fraction_digits) const {
    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);

    std::array<char, 256> buffer;
    snprintf(buffer.data(), buffer.size(),
             "%04" PRId64 "/%02" PRId64 "/%02" PRId64 " %02" PRId64 ":%02" PRId64 ":%0*.*f",
             epoch.year, epoch.month, epoch.day, epoch.hour, epoch.minutes, fraction_digits + 3,
             fraction_digits, epoch.seconds);

    return std::string{buffer.data()};
}

std::string Utc::rfc3339() const {
    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);

    std::array<char, 256> buffer;
    snprintf(buffer.data(), buffer.size(),
             "%04" PRId64 "-%02" PRId64 "-%02" PRId64 "T%02" PRId64 ":%02" PRId64 ":%06.3fZ",
             epoch.year, epoch.month, epoch.day, epoch.hour, epoch.minutes, epoch.seconds);

    return std::string{buffer.data()};
}

std::string Utc::rinex_string() const {
    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);

    std::array<char, 256> buffer;
    snprintf(buffer.data(), buffer.size(),
             "%04" PRId64 "%02" PRId64 "%02" PRId64 " %02" PRId64 "%02" PRId64 "%02" PRId64 " UTC",
             epoch.year, epoch.month, epoch.day, epoch.hour, epoch.minutes,
             static_cast<int64_t>(epoch.seconds));

    return std::string{buffer.data()};
}

std::string Utc::rinex_filename() const {
    auto ts    = timestamp();
    auto epoch = date_from_utc(ts);

    std::array<char, 256> buffer;
    snprintf(buffer.data(), buffer.size(),
             "%04" PRId64 "%02" PRId64 "%02" PRId64 "%02" PRId64 "%02" PRId64, epoch.year,
             epoch.month, epoch.day, epoch.hour, epoch.minutes);

    return std::string{buffer.data()};
}

Utc Utc::now() {
    struct timeval tv{};
    struct tm      time_info{};

    gettimeofday(&tv, nullptr);
    auto tt = gmtime_r(&tv.tv_sec, &time_info);

    auto year      = tt->tm_year + GMTIME_START_YEAR;
    auto month     = tt->tm_mon + 1;
    auto day       = tt->tm_mday;
    auto hour      = tt->tm_hour;
    auto minutes   = tt->tm_min;
    auto seconds   = tt->tm_sec;
    auto fractions = static_cast<double>(tv.tv_usec) * MICROSECONDS_TO_SECONDS;

    auto timestamp = utc_from_date(year, month, day, hour, minutes, seconds);
    timestamp.add(fractions);

    return Utc(timestamp);
}

Utc Utc::from_day_tod(int64_t day, double tod) {
    return Utc(Timestamp{static_cast<double>(day * DAY_IN_SECONDS) + tod});
}

Utc Utc::from_date_time(int64_t year, int64_t month, int64_t day, int64_t hour, int64_t minute,
                        double second) {
    return Utc(utc_from_date(year, month, day, hour, minute, second));
}

TimePoint Utc::time_point() const {
    return date_from_utc(mTm);
}

Utc Utc::from_time_point(TimePoint const& tp) {
    return Utc(utc_from_date(tp.year, tp.month, tp.day, tp.hour, tp.minutes, tp.seconds));
}

// https://gssc.esa.int/navipedia/index.php/Julian_Date
double Utc::julian_date(double ut1_utc) const {
    auto ts    = this->ut1(ut1_utc);
    auto epoch = date_from_utc(ts);

    auto y = epoch.year;
    auto m = epoch.month;
    auto d = epoch.day;
    if (m <= 2) {
        y -= 1;
        m += 12;
    }

    double jd = 0.0;
    jd += floor(365.25 * static_cast<double>(y));
    jd += floor(30.6001 * static_cast<double>(m + 1));
    jd += static_cast<double>(d);
    jd += static_cast<double>(epoch.hour) / 24.0;
    jd += static_cast<double>(epoch.minutes) / 1440.0;
    jd += epoch.seconds / 86400.0;
    jd += 1720981.5;
    return jd;
}

double Utc::j2000_century(double ut1_utc) const {
    auto jd = julian_date(ut1_utc);
    return (jd - 2451545.0) / 36525.0;
}

}  // namespace ts
