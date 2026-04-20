#include "time.hpp"

#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include "GNSS-SystemTime.h"
EXTERNAL_WARNINGS_POP

// SPARTN epoch per system: Jan 1, 2010 00:00:00 in each system's own time.
// TF009 counts seconds since that moment in the native system time.
// Days from each GNSS epoch to Jan 1 2010 00:00:00 in that system's time:
CONSTEXPR static int64_t SPARTN_EPOCH_GPS_DAYS = 10953;  // GPS epoch: Jan 6 1980
CONSTEXPR static int64_t SPARTN_EPOCH_GST_DAYS = 3785;   // GST epoch: Aug 22 1999
CONSTEXPR static int64_t SPARTN_EPOCH_BDT_DAYS = 1461;   // BDT epoch: Jan 1 2006
CONSTEXPR static int64_t SPARTN_EPOCH_GLO_DAYS = 5114;   // GLO epoch: Jan 1 1996

static SpartnTime spartn_time_from_days_tod(int64_t days, double tod, int64_t epoch_days) {
    auto days_since_2010    = days - epoch_days;
    auto seconds_since_2010 = static_cast<double>(days_since_2010 * ts::DAY_IN_SECONDS) + tod;
    return SpartnTime{seconds_since_2010, static_cast<uint32_t>(seconds_since_2010)};
}

SpartnTime spartn_time_from(GNSS_SystemTime const& epoch_time) {
    auto day_number  = epoch_time.gnss_DayNumber;
    auto time_of_day = static_cast<double>(epoch_time.gnss_TimeOfDay);
    if (epoch_time.gnss_TimeOfDayFrac_msec) {
        time_of_day += static_cast<double>(*epoch_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    }

    switch (epoch_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: {
        auto t = ts::Gps::from_day_tod(day_number, time_of_day);
        return spartn_time_from_days_tod(t.days(), t.time_of_day().as_double(),
                                         SPARTN_EPOCH_GPS_DAYS);
    }
    case GNSS_ID__gnss_id_galileo: {
        auto t = ts::Gst::from_day_tod(day_number, time_of_day);
        return spartn_time_from_days_tod(t.days(), t.time_of_day().as_double(),
                                         SPARTN_EPOCH_GST_DAYS);
    }
    case GNSS_ID__gnss_id_glonass: {
        auto t = ts::Glo::from_day_tod(day_number, time_of_day);
        return spartn_time_from_days_tod(t.days(), t.time_of_day().as_double(),
                                         SPARTN_EPOCH_GLO_DAYS);
    }
    case GNSS_ID__gnss_id_bds: {
        auto t = ts::Bdt::from_day_tod(day_number, time_of_day);
        return spartn_time_from_days_tod(t.days(), t.time_of_day().as_double(),
                                         SPARTN_EPOCH_BDT_DAYS);
    }
    default: return SpartnTime{0, 0};
    }
}
