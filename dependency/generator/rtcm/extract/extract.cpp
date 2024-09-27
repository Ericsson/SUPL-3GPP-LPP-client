#include "extract.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <GNSS-ID.h>
#include <GNSS-SystemTime.h>
#pragma GCC diagnostic pop

namespace decode {

long day_number(GNSS_SystemTime const& src_time) {
    return src_time.gnss_DayNumber;
}

double time_of_day(GNSS_SystemTime const& src_time) {
    return static_cast<double>(src_time.gnss_TimeOfDay);
}

Maybe<double> time_of_day_fraction(GNSS_SystemTime const& src_time) {
    if (src_time.gnss_TimeOfDayFrac_msec) {
        return static_cast<double>(*src_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    } else {
        return Maybe<double>();
    }
}

GenericGnssId gnss_id(GNSS_SystemTime const& src_time) {
    switch (src_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: return GenericGnssId::GPS;
    case GNSS_ID__gnss_id_glonass: return GenericGnssId::GLONASS;
    case GNSS_ID__gnss_id_galileo: return GenericGnssId::GALILEO;
    case GNSS_ID__gnss_id_bds: return GenericGnssId::BEIDOU;
    }

    RTCM_UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    return GenericGnssId::GPS;
#endif
}

ts::TAI_Time epoch_time(GNSS_SystemTime const& src_time) {
    auto day_number           = decode::day_number(src_time);
    auto time_of_day_seconds  = decode::time_of_day(src_time);
    auto time_of_day_fraction = decode::time_of_day_fraction(src_time);
    auto time_of_day          = time_of_day_seconds;
    if (time_of_day_fraction.valid) {
        time_of_day += time_of_day_fraction.value;
    }

    auto gnss = decode::gnss_id(src_time);
    switch (gnss) {
    case GenericGnssId::GPS: return ts::TAI_Time(ts::GPS_Time(day_number, time_of_day));
    case GenericGnssId::GLONASS: return ts::TAI_Time(ts::GLO_Time(day_number, time_of_day));
    case GenericGnssId::GALILEO: return ts::TAI_Time(ts::GST_Time(day_number, time_of_day));
    case GenericGnssId::BEIDOU: return ts::TAI_Time(ts::BDT_Time(day_number, time_of_day));
    }

    return ts::TAI_Time::now();
}

}  // namespace decode
