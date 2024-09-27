#pragma once
#include "timestamp.hpp"

namespace ts {
class TAI_Time;
class UTC_Time;
class GLO_Time {
public:
    friend UTC_Time;

    GLO_Time() = default;
    RTCM_EXPLICIT GLO_Time(TsInt day, TsFloat tod);
    RTCM_EXPLICIT GLO_Time(Timestamp const& timestamp) : tm{timestamp} {}
    RTCM_EXPLICIT GLO_Time(TAI_Time const& time);
    RTCM_EXPLICIT GLO_Time(UTC_Time const& time);

    RTCM_NODISCARD TsInt     days() const;
    RTCM_NODISCARD TsInt     week() const;
    RTCM_NODISCARD Timestamp time_of_day() const;
    RTCM_NODISCARD Timestamp time_of_week() const;
    RTCM_NODISCARD Timestamp timestamp() const { return tm; }

    static GLO_Time now();

protected:
    RTCM_NODISCARD Timestamp utc_timestamp() const;

private:
    Timestamp tm;
};
}  // namespace ts
