#include "bdt.hpp"
#include "glo.hpp"
#include "gps.hpp"
#include "gst.hpp"
#include "tai.hpp"
#include "utc.hpp"

namespace ts {

static int64_t leap_seconds_utc_bdt() {
    // NOTE(ewasjon): There are 33 leapseconds between the start of UTC and the start of BDT, after
    // that BDT includes leapseconds similar to UTC.
    auto leap_seconds = LeapSeconds::count() - 33;
    return leap_seconds;
}

static Timestamp utc_2_bdt(Timestamp timestamp) {
    auto leap_seconds      = leap_seconds_utc_bdt();
    auto seconds_since_utc = timestamp;
    auto seconds_since_bdt = seconds_since_utc;

    seconds_since_bdt.add(leap_seconds);

    // -9 days because leap days between 1970 and 2006
    constexpr auto leap_days = 9;
    seconds_since_bdt.subtract(leap_days * DAY_IN_SECONDS);

    // -36 years because bdt time started 2006 (2006 - 1970)
    constexpr auto year_difference = 36;
    seconds_since_bdt.subtract(year_difference * YEAR_IN_SECONDS);
    return seconds_since_bdt;
}

static Timestamp bdt_2_utc(Timestamp timestamp) {
    auto temp = utc_2_bdt(timestamp);
    auto diff = timestamp - temp;
    assert(diff.fraction() == 0.0);
    return timestamp + diff;
}

Bdt::Bdt() = default;
Bdt::Bdt(Timestamp const& timestamp) : tm{timestamp} {}
Bdt::Bdt(Utc const& time) : tm(utc_2_bdt(time.timestamp())) {}
Bdt::Bdt(Tai const& time) : Bdt(Utc(time)) {}
Bdt::Bdt(Glo const& time) : Bdt(Utc(time)) {}
Bdt::Bdt(Gst const& time) : Bdt(Utc(time)) {}
Bdt::Bdt(Gps const& time) : Bdt(Utc(time)) {}

int64_t Bdt::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

int64_t Bdt::week() const {
    return tm.seconds() / WEEK_IN_SECONDS;
}

Timestamp Bdt::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

Timestamp Bdt::time_of_week() const {
    return tm - Timestamp{week() * WEEK_IN_SECONDS};
}

Bdt Bdt::now() {
    return Bdt{Utc::now()};
}

Bdt Bdt::from_day_tod(int64_t day, double tod) {
    Timestamp tm{};
    tm.add(DAY_IN_SECONDS * day);
    tm.add(tod);
    return Bdt{tm};
}

Bdt Bdt::from_week_tow(int64_t week, int64_t tow, double fractions) {
    Timestamp tm{};
    tm.add(WEEK_IN_SECONDS * week);
    tm.add(tow);
    tm.add(fractions);
    return Bdt{tm};
}

Timestamp Bdt::utc_timestamp() const {
    return bdt_2_utc(tm);
}

}  // namespace ts
