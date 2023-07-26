#include <utility/time.h>

static s64 leap_seconds_utc_bdt() {
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

BDT_Time::BDT_Time(s64 day, f64 tod) : tm{} {
    tm.add(DAY_IN_SECONDS * day);
    tm.add(tod);
}

BDT_Time::BDT_Time(s64 week, s64 tow, f64 fractions) : tm{} {
    tm.add(WEEK_IN_SECONDS * week);
    tm.add(tow);
    tm.add(fractions);
}

BDT_Time::BDT_Time(const TAI_Time& time) : BDT_Time(UTC_Time(time)) {}
BDT_Time::BDT_Time(const UTC_Time& time) : tm(utc_2_bdt(time.timestamp())) {}

s64 BDT_Time::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

s64 BDT_Time::week() const {
    return tm.seconds() / WEEK_IN_SECONDS;
}

Timestamp BDT_Time::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

BDT_Time BDT_Time::now() {
    return BDT_Time{UTC_Time::now()};
}

Timestamp BDT_Time::utc_timestamp() const {
    return bdt_2_utc(tm);
}
