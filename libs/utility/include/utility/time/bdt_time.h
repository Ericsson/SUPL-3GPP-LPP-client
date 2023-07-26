#pragma once
#include <utility/types.h>
#include <utility/time.h>

class TAI_Time;
class UTC_Time;
class BDT_Time {
public:
    friend UTC_Time;

    BDT_Time() = default;
    explicit BDT_Time(s64 day, f64 tod);
    explicit BDT_Time(s64 week, s64 tow, f64 fractions);
    explicit BDT_Time(const Timestamp& timestamp) : tm{timestamp} {}
    explicit BDT_Time(const TAI_Time& time);
    explicit BDT_Time(const UTC_Time& time);

    NO_DISCARD s64       days() const;
    NO_DISCARD s64       week() const;
    NO_DISCARD Timestamp time_of_day() const;
    NO_DISCARD Timestamp timestamp() const { return tm; }

    NO_DISCARD static BDT_Time now();

protected:
    NO_DISCARD Timestamp utc_timestamp() const;

private:
    // NOTE: Seconds since the begining of BDT (00:00:00 on January 1st, 2006). BDT is -33 seconds away from TAI
    // as of (2022-09-22) and changes with added or subtracted leap-seconds.
    Timestamp tm;
};
