#include "extract.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <GNSS-ID.h>
#include <GNSS-SystemTime.h>
EXTERNAL_WARNINGS_POP

#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(rtcm, extract);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(rtcm, extract)

namespace generator {
namespace rtcm {

long st_day_number(GNSS_SystemTime const& src_time) {
    return src_time.gnss_DayNumber;
}

double st_time_of_day(GNSS_SystemTime const& src_time) {
    return static_cast<double>(src_time.gnss_TimeOfDay);
}

Maybe<double> st_time_of_day_fraction(GNSS_SystemTime const& src_time) {
    if (src_time.gnss_TimeOfDayFrac_msec) {
        return static_cast<double>(*src_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    } else {
        return Maybe<double>();
    }
}

generator::rtcm::GenericGnssId gnss_id(GNSS_SystemTime const& src_time) {
    switch (src_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: return generator::rtcm::GenericGnssId::GPS;
    case GNSS_ID__gnss_id_glonass: return generator::rtcm::GenericGnssId::GLONASS;
    case GNSS_ID__gnss_id_galileo: return generator::rtcm::GenericGnssId::GALILEO;
    case GNSS_ID__gnss_id_bds: return generator::rtcm::GenericGnssId::BEIDOU;
    }

    UNREACHABLE();
#if COMPILER_CANNOT_DEDUCE_UNREACHABLE
    return generator::rtcm::GenericGnssId::GPS;
#endif
}

ts::Tai epoch_time(GNSS_SystemTime const& src_time) {
    auto day_number           = st_day_number(src_time);
    auto time_of_day_seconds  = st_time_of_day(src_time);
    auto time_of_day_fraction = st_time_of_day_fraction(src_time);
    auto time_of_day          = time_of_day_seconds;
    if (time_of_day_fraction.valid) {
        time_of_day += time_of_day_fraction.value;
    }

    auto gnss = gnss_id(src_time);
    switch (gnss) {
    case generator::rtcm::GenericGnssId::GPS:
        return ts::Tai(ts::Gps::from_day_tod(day_number, time_of_day));
    case generator::rtcm::GenericGnssId::GLONASS:
        return ts::Tai(ts::Glo::from_day_tod(day_number, time_of_day));
    case generator::rtcm::GenericGnssId::GALILEO:
        return ts::Tai(ts::Gst::from_day_tod(day_number, time_of_day));
    case generator::rtcm::GenericGnssId::BEIDOU:
        return ts::Tai(ts::Bdt::from_day_tod(day_number, time_of_day));
    }

    return ts::Tai::now();
}

}  // namespace rtcm
}  // namespace generator
