#include "glo.hpp"
#include "bdt.hpp"
#include "gps.hpp"
#include "gst.hpp"
#include "tai.hpp"
#include "utc.hpp"

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

Glo::Glo() = default;
Glo::Glo(Timestamp const& timestamp) : tm{timestamp} {}
Glo::Glo(Utc const& time) : tm(utc_2_glo(time.timestamp())) {}
Glo::Glo(Tai const& time) : Glo(Utc(time)) {}
Glo::Glo(Gps const& time) : Glo(Utc(time)) {}
Glo::Glo(Gst const& time) : Glo(Utc(time)) {}
Glo::Glo(Bdt const& time) : Glo(Utc(time)) {}

int64_t Glo::days() const {
    return tm.seconds() / DAY_IN_SECONDS;
}

Timestamp Glo::time_of_day() const {
    return tm - Timestamp{days() * DAY_IN_SECONDS};
}

Glo Glo::now() {
    return Glo{Utc::now()};
}

Glo Glo::from_day_tod(int64_t day, double tod) {
    Timestamp timestamp{};
    timestamp.add(DAY_IN_SECONDS * day);
    timestamp.add(tod);
    return Glo{timestamp};
}

Timestamp Glo::utc_timestamp() const {
    return glo_2_utc(tm);
}

}  // namespace ts
