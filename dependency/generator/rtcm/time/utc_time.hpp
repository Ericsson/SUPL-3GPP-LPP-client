#pragma once
#include "timestamp.hpp"

#include <string>

namespace ts {
class TAI_Time;
class GPS_Time;
class GLO_Time;
class GST_Time;
class BDT_Time;
class UTC_Time {
public:
    UTC_Time() = default;
    RTCM_EXPLICIT UTC_Time(Timestamp const& timestamp) : tm{timestamp} {}
    RTCM_EXPLICIT UTC_Time(TAI_Time const& time);
    RTCM_EXPLICIT UTC_Time(GPS_Time const& time);
    RTCM_EXPLICIT UTC_Time(GLO_Time const& time);
    RTCM_EXPLICIT UTC_Time(GST_Time const& time);
    RTCM_EXPLICIT UTC_Time(BDT_Time const& time);

    RTCM_NODISCARD Timestamp timestamp() const { return tm; }
    RTCM_NODISCARD std::string rtklib_time_string() const;

    UTC_Time& add(TsInt seconds) {
        tm.add(seconds);
        return *this;
    }

    RTCM_NODISCARD static UTC_Time now();

private:
    Timestamp tm;
};
}  // namespace ts
