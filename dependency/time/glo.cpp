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
    constexpr auto LEAP_DAYS = 6;
    timestamp.subtract(LEAP_DAYS * DAY_IN_SECONDS);

    // -26 years because glonass time started 1996 (1996 - 1970)
    constexpr auto YEAR_DIFFERENCE = 26;
    timestamp.subtract(YEAR_DIFFERENCE * YEAR_IN_SECONDS);
    return timestamp;
}

static Timestamp glo_2_utc(Timestamp timestamp) {
    auto temp = utc_2_glo(timestamp);
    auto diff = timestamp - temp;
    return timestamp + diff;
}

Glo::Glo() = default;
Glo::Glo(Timestamp const& timestamp) : mTm{timestamp} {}
Glo::Glo(Utc const& time) : mTm(utc_2_glo(time.timestamp())) {}
Glo::Glo(Tai const& time) : Glo(Utc(time)) {}
Glo::Glo(Gps const& time) : Glo(Utc(time)) {}
Glo::Glo(Gst const& time) : Glo(Utc(time)) {}
Glo::Glo(Bdt const& time) : Glo(Utc(time)) {}

int64_t Glo::days() const {
    return mTm.seconds() / DAY_IN_SECONDS;
}

Timestamp Glo::time_of_day() const {
    return mTm - Timestamp{days() * DAY_IN_SECONDS};
}

Glo Glo::now() {
    return Glo{Utc::now()};
}

Glo Glo::from_absolute_day_tod(int64_t day, double tod) {
    Timestamp timestamp{};
    timestamp.add(DAY_IN_SECONDS * day);
    timestamp.add(tod);
    return Glo{timestamp};
}

Glo Glo::from_period_day_tod(int64_t day, double tod, Glo const& reference) {
    // GLONASS n_t is day within 4-year period (1461 days)
    // Normalize day to [0, 1460]
    CONSTEXPR static int64_t FOUR_YEAR_PERIOD = 1461;

    while (day < 0) {
        day += FOUR_YEAR_PERIOD;
    }
    while (day >= FOUR_YEAR_PERIOD) {
        day -= FOUR_YEAR_PERIOD;
    }

    // Find which 4-year period contains the reference time
    auto ref_days          = reference.days();
    auto ref_day_in_period = ref_days % FOUR_YEAR_PERIOD;
    auto period_start      = ref_days - ref_day_in_period;

    // Construct time in reference's period
    auto absolute_days = period_start + day;

    Timestamp timestamp{};
    timestamp.add(DAY_IN_SECONDS * absolute_days);
    timestamp.add(tod);
    return Glo{timestamp};
}

Timestamp Glo::utc_timestamp() const {
    return glo_2_utc(mTm);
}

}  // namespace ts
