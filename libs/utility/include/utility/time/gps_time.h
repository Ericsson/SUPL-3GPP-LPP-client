#pragma once
#include <utility/types.h>
#include <utility/time.h>

class GPS_Time {
public:
    friend UTC_Time;

    GPS_Time() = default;
    explicit GPS_Time(s64 day, f64 tod);
    explicit GPS_Time(s64 week, s64 tow, f64 fractions);
    explicit GPS_Time(const Timestamp& timestamp) : tm(timestamp) {}
    explicit GPS_Time(const TAI_Time& time);
    explicit GPS_Time(const UTC_Time& time);

    NO_DISCARD s64       days() const;
    NO_DISCARD Timestamp time_of_day() const;
    NO_DISCARD s64       week() const;
    NO_DISCARD Timestamp time_of_week() const;
    NO_DISCARD Timestamp timestamp() const { return tm; }

    static GPS_Time now();

protected:
    NO_DISCARD Timestamp utc_timestamp() const;

private:
    Timestamp tm;
};
