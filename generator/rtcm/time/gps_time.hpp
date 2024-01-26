#pragma once
#include "timestamp.hpp"

namespace ts {
class UTC_Time;
class TAI_Time;
class GPS_Time {
public:
    friend UTC_Time;

    GPS_Time() = default;
    RTCM_EXPLICIT GPS_Time(TsInt day, TsFloat tod);
    RTCM_EXPLICIT GPS_Time(TsInt week, TsInt tow, TsFloat fractions);
    RTCM_EXPLICIT GPS_Time(const Timestamp& timestamp) : tm(timestamp) {}
    RTCM_EXPLICIT GPS_Time(const TAI_Time& time);
    RTCM_EXPLICIT GPS_Time(const UTC_Time& time);

    RTCM_NODISCARD TsInt     days() const;
    RTCM_NODISCARD TsInt     week() const;
    RTCM_NODISCARD Timestamp time_of_day() const;
    RTCM_NODISCARD Timestamp time_of_week() const;
    RTCM_NODISCARD Timestamp timestamp() const { return tm; }

    static GPS_Time now();

protected:
    RTCM_NODISCARD Timestamp utc_timestamp() const;

private:
    Timestamp tm;
};
}  // namespace ts
