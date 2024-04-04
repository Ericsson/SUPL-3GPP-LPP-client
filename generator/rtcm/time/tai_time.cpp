#include "tai_time.hpp"
#include "utc_time.hpp"

namespace ts {
static Timestamp utc_2_tai(Timestamp timestamp) {
    timestamp.subtract(LeapSeconds::count());
    return timestamp;
}

static Timestamp tai_2_utc(Timestamp timestamp) {
    timestamp.add(LeapSeconds::count());
    return timestamp;
}

TAI_Time::TAI_Time(GPS_Time const& time) : TAI_Time(UTC_Time(time)) {}
TAI_Time::TAI_Time(GLO_Time const& time) : TAI_Time(UTC_Time(time)) {}
TAI_Time::TAI_Time(GST_Time const& time) : TAI_Time(UTC_Time(time)) {}
TAI_Time::TAI_Time(BDT_Time const& time) : TAI_Time(UTC_Time(time)) {}
TAI_Time::TAI_Time(UTC_Time const& time) : tm(utc_2_tai(time.timestamp())) {}

std::string TAI_Time::rtklib_time_string() const {
    return UTC_Time{*this}.rtklib_time_string();
}

TAI_Time TAI_Time::now() {
    return TAI_Time{UTC_Time::now()};
}

Timestamp TAI_Time::utc_timestamp() const {
    return tai_2_utc(tm);
}
}  // namespace ts
