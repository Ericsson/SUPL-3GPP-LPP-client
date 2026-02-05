#include "time.hpp"

#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include "GNSS-SystemTime.h"
EXTERNAL_WARNINGS_POP

// SPARTN epoch: Jan 1, 2010 00:00:00 GPS time
// = 14610 (days 1970->2010) - 3657 (days 1970->GPS epoch) = 10953 days from GPS epoch
CONSTEXPR static int64_t SPARTN_EPOCH_GPS_DAYS = 10953;

SpartnTime spartn_time_from(GNSS_SystemTime const& epoch_time) {
    auto day_number  = epoch_time.gnss_DayNumber;
    auto time_of_day = static_cast<double>(epoch_time.gnss_TimeOfDay);
    if (epoch_time.gnss_TimeOfDayFrac_msec) {
        time_of_day += static_cast<double>(*epoch_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    }

    ts::Gps gps_time;
    switch (epoch_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: gps_time = ts::Gps::from_day_tod(day_number, time_of_day); break;
    case GNSS_ID__gnss_id_galileo:
        gps_time = ts::Gps(ts::Gst::from_day_tod(day_number, time_of_day));
        break;
    case GNSS_ID__gnss_id_glonass:
        gps_time = ts::Gps(ts::Glo::from_day_tod(day_number, time_of_day));
        break;
    case GNSS_ID__gnss_id_bds:
        gps_time = ts::Gps(ts::Bdt::from_day_tod(day_number, time_of_day));
        break;
    default: return SpartnTime{0, 0};
    }

    auto days_since_2010    = gps_time.days() - SPARTN_EPOCH_GPS_DAYS;
    auto seconds_since_2010 = static_cast<double>(days_since_2010 * ts::DAY_IN_SECONDS) +
                              gps_time.time_of_day().as_double();

    return SpartnTime{
        seconds_since_2010,
        static_cast<uint32_t>(seconds_since_2010 + 0.5),
    };
}
