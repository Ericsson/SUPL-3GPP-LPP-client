#include "tai.hpp"
#include "bdt.hpp"
#include "glo.hpp"
#include "gps.hpp"
#include "gst.hpp"
#include "utc.hpp"

namespace ts {

static Timestamp utc_2_tai(Timestamp timestamp) {
    timestamp.subtract(LeapSeconds::count());
    return timestamp;
}

static Timestamp tai_2_utc(Timestamp timestamp) {
    timestamp.add(LeapSeconds::count());
    return timestamp;
}

Tai::Tai() = default;
Tai::Tai(Timestamp const& timestamp) : tm{timestamp} {}
Tai::Tai(Utc const& time) : tm(utc_2_tai(time.timestamp())) {}
Tai::Tai(Gps const& time) : Tai(Utc(time)) {}
Tai::Tai(Glo const& time) : Tai(Utc(time)) {}
Tai::Tai(Gst const& time) : Tai(Utc(time)) {}
Tai::Tai(Bdt const& time) : Tai(Utc(time)) {}

std::string Tai::rtklib_time_string() const {
    return Utc{*this}.rtklib_time_string();
}

Tai Tai::operator+(Timestamp delta) const {
    return Tai{tm + delta};
}

Timestamp Tai::difference(Tai const& other) const {
    return tm - other.tm;
}

Tai Tai::now() {
    return Tai{Utc::now()};
}

Timestamp Tai::utc_timestamp() const {
    return tai_2_utc(tm);
}

}  // namespace ts
