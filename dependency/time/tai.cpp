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
Tai::Tai(Timestamp const& timestamp) : mTm{timestamp} {}
Tai::Tai(Utc const& time) : mTm(utc_2_tai(time.timestamp())) {}
Tai::Tai(Gps const& time) : Tai(Utc(time)) {}
Tai::Tai(Glo const& time) : Tai(Utc(time)) {}
Tai::Tai(Gst const& time) : Tai(Utc(time)) {}
Tai::Tai(Bdt const& time) : Tai(Utc(time)) {}

std::string Tai::rtklib_time_string(int fraction_digits) const {
    return Utc{*this}.rtklib_time_string(fraction_digits);
}

Tai Tai::operator+(Timestamp delta) const {
    return Tai{mTm + delta};
}

Timestamp Tai::difference(Tai const& other) const {
    return mTm - other.mTm;
}

double Tai::difference_seconds(Tai const& other) const {
    return mTm.full_seconds() - other.mTm.full_seconds();
}

Tai Tai::now() {
    return Tai{Utc::now()};
}

Timestamp Tai::utc_timestamp() const {
    return tai_2_utc(mTm);
}

}  // namespace ts
