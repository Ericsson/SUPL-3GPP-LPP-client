#include "gst.hpp"
#include "tai.hpp"
#include "utc.hpp"

namespace ts {

static int64_t leap_seconds_utc_gst() {
    // NOTE(ewasjon): The leapseconds difference between UTC and GST is 13 (because of the begining)
    // and the difference between in leapseconds sinec the start (which is 5 as of 2022:09:22). Or
    // in other words 13 + (TAI - UTC) - 32.
    auto leap_seconds = 13 + (LeapSeconds::count() - 32);
    return leap_seconds;
}

// Calculate the current GPS timestamp from UTC timestamp
// UTC is seconds from: 00:00:00, Jan 1  1970 (including leap-seconds)
// GST is seconds from: 00:00:00, Aug 22 1999
static Timestamp utc_2_gst(Timestamp utc_time) {
    auto leap_seconds      = leap_seconds_utc_gst();
    auto seconds_since_utc = utc_time;
    auto seconds_since_gst = seconds_since_utc;

    // At 00:00:00, Aug 22 1999 GST was ahead of UTC by 13 leapseconds. If UTC == GST(T0) then we
    // -13 seconds. Thus we need to add the difference.
    seconds_since_gst.add(leap_seconds);

    // -233 days because GST time started on the 22nd of August 1999
    constexpr auto DAYS_DIFFERENCE = 233;
    seconds_since_gst.subtract(DAYS_DIFFERENCE * DAY_IN_SECONDS);

    // -7 leap days between 1970 and 1999.
    // (1974, 1978, 1982, 1986, 1990, 1994, 1998)
    constexpr auto LEAP_DAYS = 7;
    seconds_since_gst.subtract(LEAP_DAYS * DAY_IN_SECONDS);

    // -29 years because gps time started 1999 (1999 - 1970)
    constexpr auto YEAR_DIFFERENCE = 29;
    seconds_since_gst.subtract(YEAR_DIFFERENCE * YEAR_IN_SECONDS);
    return seconds_since_gst;
}

static Timestamp gst_2_utc(Timestamp gst) {
    auto temp = utc_2_gst(gst);
    auto diff = gst - temp;
    return gst + diff;
}

Gst::Gst() = default;
Gst::Gst(Timestamp const& timestamp) : mTm{timestamp} {}
Gst::Gst(Utc const& time) : mTm{utc_2_gst(time.timestamp())} {}
Gst::Gst(Tai const& time) : Gst(Utc(time)) {}
Gst::Gst(Glo const& time) : Gst(Utc(time)) {}
Gst::Gst(Gps const& time) : Gst(Utc(time)) {}
Gst::Gst(Bdt const& time) : Gst(Utc(time)) {}

int64_t Gst::days() const {
    return mTm.seconds() / DAY_IN_SECONDS;
}

Timestamp Gst::time_of_day() const {
    return mTm - Timestamp{days() * DAY_IN_SECONDS};
}

int64_t Gst::week() const {
    return mTm.seconds() / WEEK_IN_SECONDS;
}

Timestamp Gst::time_of_week() const {
    return mTm - Timestamp{week() * WEEK_IN_SECONDS};
}

Gst Gst::now() {
    return Gst{Utc::now()};
}

Gst Gst::from_day_tod(int64_t day, double tod) {
    Timestamp timestamp{};
    timestamp.add(DAY_IN_SECONDS * day);
    timestamp.add(tod);
    return Gst{timestamp};
}

Gst Gst::from_week_tow(int64_t week, int64_t tow, double fractions) {
    Timestamp timestamp{};
    timestamp.add(WEEK_IN_SECONDS * week);
    timestamp.add(tow);
    timestamp.add(fractions);
    return Gst{timestamp};
}

Timestamp Gst::utc_timestamp() const {
    return gst_2_utc(mTm);
}

}  // namespace ts
