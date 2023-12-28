#pragma once
#include "timestamp.hpp"

namespace ts {
class TAI_Time;
class UTC_Time;
class BDT_Time {
public:
    friend UTC_Time;

    BDT_Time() = default;
    RTCM_EXPLICIT BDT_Time(TsInt day, TsFloat tod);
    RTCM_EXPLICIT BDT_Time(TsInt week, TsInt tow, TsFloat fractions);
    RTCM_EXPLICIT BDT_Time(const Timestamp& timestamp) : tm{timestamp} {}
    RTCM_EXPLICIT BDT_Time(const TAI_Time& time);
    RTCM_EXPLICIT BDT_Time(const UTC_Time& time);

    RTCM_NODISCARD TsInt     days() const;
    RTCM_NODISCARD TsInt     week() const;
    RTCM_NODISCARD Timestamp time_of_day() const;
    RTCM_NODISCARD Timestamp time_of_week() const;
    RTCM_NODISCARD Timestamp timestamp() const { return tm; }

    RTCM_NODISCARD static BDT_Time now();

protected:
    RTCM_NODISCARD Timestamp utc_timestamp() const;

private:
    // NOTE: Seconds since the begining of BDT (00:00:00 on January 1st, 2006). BDT is -33 seconds
    // away from TAI as of (2022-09-22) and changes with added or subtracted leap-seconds.
    Timestamp tm;
};
}  // namespace ts
