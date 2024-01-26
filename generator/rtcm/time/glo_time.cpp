#include "glo_time.hpp"
#include "utc_time.hpp"

namespace ts {
static Timestamp utc_2_glo(Timestamp timestamp) {
    timestamp.add(3 * HOUR_IN_SECONDS);

    // -6 leap days between 1970 and 1996.
    // (1974, 1978, 1982, 1986, 1990, 1994)
    constexpr auto leap_days = 6;
    timestamp.subtract(leap_days * DAY_IN_SECONDS);

    // -26 years because glonass time started 1996 (1996 - 1970)
    constexpr auto year_difference = 26;
    timestamp.subtract(year_difference * YEAR_IN_SECONDS);
    return timestamp;
}

static Timestamp glo_2_utc(Timestamp timestamp) {
    auto temp = utc_2_glo(timestamp);
    auto diff = timestamp - temp;
    return timestamp + diff;
}

GLO_Time::GLO_Time(TsInt day, TsFloat tod) : tm{} {
    tm.add(DAY_IN_SECONDS * day);
    tm.add(tod);
}

GLO_Time::GLO_Time(const TAI_Time& time) : GLO_Time(UTC_Time(time)) {}
GLO_Time::GLO_Time(const UTC_Time& time) : tm(utc_2_glo(time.timestamp())) {}

TsInt GLO_Time::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

TsInt GLO_Time::week() const {
    return tm.seconds() / WEEK_IN_SECONDS;
}

Timestamp GLO_Time::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

Timestamp GLO_Time::time_of_week() const {
    return tm - Timestamp{week() * WEEK_IN_SECONDS};
}

GLO_Time GLO_Time::now() {
    return GLO_Time{UTC_Time::now()};
}

Timestamp GLO_Time::utc_timestamp() const {
    return glo_2_utc(tm);
}
}  // namespace ts
