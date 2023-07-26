#pragma once
#include <utility/types.h>
#include <utility/time.h>

class TAI_Time;
class UTC_Time;
class GLO_Time {
public:
    friend UTC_Time;

    GLO_Time() = default;
    explicit GLO_Time(s64 day, f64 tod);
    explicit GLO_Time(const Timestamp& timestamp) : tm{timestamp} {}
    explicit GLO_Time(const TAI_Time& time);
    explicit GLO_Time(const UTC_Time& time);

    NO_DISCARD s64       days() const;
    NO_DISCARD Timestamp time_of_day() const;
    NO_DISCARD Timestamp timestamp() const { return tm; }

    static GLO_Time now();

protected:
    NO_DISCARD Timestamp utc_timestamp() const;

private:
    Timestamp tm;
};
