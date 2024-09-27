#pragma once
#include "timestamp.hpp"

#include <string>

namespace ts {
class GPS_Time;
class GLO_Time;
class GST_Time;
class UTC_Time;
class BDT_Time;
class TAI_Time {
public:
    friend UTC_Time;

    TAI_Time() = default;
    RTCM_EXPLICIT TAI_Time(Timestamp const& timestamp) : tm(timestamp) {}
    RTCM_EXPLICIT TAI_Time(GPS_Time const& time);
    RTCM_EXPLICIT TAI_Time(GLO_Time const& time);
    RTCM_EXPLICIT TAI_Time(GST_Time const& time);
    RTCM_EXPLICIT TAI_Time(UTC_Time const& time);
    RTCM_EXPLICIT TAI_Time(BDT_Time const& time);

    RTCM_NODISCARD Timestamp timestamp() const { return tm; }
    RTCM_NODISCARD std::string rtklib_time_string() const;

    RTCM_NODISCARD static TAI_Time now();

protected:
    RTCM_NODISCARD Timestamp utc_timestamp() const;

private:
    // There is no date for when the TAI timestamp began. Although, TAI - UTC
    // should be equal to the leap seconds since 1972. Which is, as of today
    // (2021-08-13), 37 seconds. Thus, the start date will be the same as UTC (1
    // january 1970).
    Timestamp tm;
};
}  // namespace ts
