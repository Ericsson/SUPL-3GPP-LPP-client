#pragma once
#include <utility/types.h>
#include <utility/time.h>

#include <string>

class UTC_Time {
public:
    UTC_Time() = default;
    explicit UTC_Time(const Timestamp& timestamp) : tm{timestamp} {}
    explicit UTC_Time(const TAI_Time& time);
    explicit UTC_Time(const GPS_Time& time);
    explicit UTC_Time(const GLO_Time& time);
    explicit UTC_Time(const GST_Time& time);
    explicit UTC_Time(const BDT_Time& time);

    NO_DISCARD Timestamp timestamp() const { return tm; }
    NO_DISCARD std::string    rtklib_time_string();

    UTC_Time& add(s64 seconds) {
        tm.add(seconds);
        return *this;
    }

    NO_DISCARD static UTC_Time now();

private:
    Timestamp tm;
};
