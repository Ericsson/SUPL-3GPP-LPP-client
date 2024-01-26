#pragma once
#include <utility/time.h>
#include <utility/types.h>

#include <string>

class GPS_Time;
class GLO_Time;
class GST_Time;
class UTC_Time;
class TAI_Time {
public:
    friend UTC_Time;

    TAI_Time() = default;
    explicit TAI_Time(const Timestamp& timestamp) : tm(timestamp) {}
    explicit TAI_Time(const GPS_Time& time);
    explicit TAI_Time(const GLO_Time& time);
    explicit TAI_Time(const GST_Time& time);
    explicit TAI_Time(const UTC_Time& time);
    explicit TAI_Time(const BDT_Time& time);

    NO_DISCARD Timestamp timestamp() const { return tm; }
    NO_DISCARD std::string rtklib_time_string() const;

    NO_DISCARD static TAI_Time now();

protected:
    NO_DISCARD Timestamp utc_timestamp() const;

private:
    // There is no date for when the TAI timestamp began. Although, TAI - UTC
    // should be equal to the leap seconds since 1972. Which is, as of today
    // (2021-08-13), 37 seconds. Thus, the start date will be the same as UTC (1
    // january 1970).
    Timestamp tm;
};
