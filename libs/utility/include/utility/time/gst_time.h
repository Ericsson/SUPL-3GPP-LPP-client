#pragma once
#include <utility/types.h>
#include <utility/time.h>

class TAI_Time;
class UTC_Time;
class GST_Time {
public:
    friend UTC_Time;

    GST_Time() = default;
    explicit GST_Time(s64 day, f64 tod);
    explicit GST_Time(s64 week, s64 tow, f64 fractions);
    explicit GST_Time(const TAI_Time& time);
    explicit GST_Time(const UTC_Time& time);

    NO_DISCARD s64       days() const;
    NO_DISCARD Timestamp time_of_day() const;
    NO_DISCARD s64       week() const;
    NO_DISCARD Timestamp time_of_week() const;
    NO_DISCARD Timestamp timestamp() const { return tm; }

    static GST_Time now();

protected:
    NO_DISCARD Timestamp utc_timestamp() const;

private:
    Timestamp tm;
};
