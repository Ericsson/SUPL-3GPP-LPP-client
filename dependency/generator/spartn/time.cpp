#include "time.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include "GNSS-SystemTime.h"
#pragma GCC diagnostic pop

#include <loglet/loglet.hpp>

LOGLET_MODULE2(spartn, time);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(spartn, time)

CONSTEXPR static uint32_t SECONDS_IN_DAY        = 86400;
CONSTEXPR static uint64_t DAY_BETWEEN_1970_1980 = 3657;   // Jan 6
CONSTEXPR static uint64_t DAY_BETWEEN_1970_1996 = 9496;   // Jan 1
CONSTEXPR static uint64_t DAY_BETWEEN_1970_1999 = 10825;  // Aug 2
CONSTEXPR static uint64_t DAY_BETWEEN_1970_2006 = 13149;  // Jan 1
CONSTEXPR static uint64_t DAY_BETWEEN_1970_2010 = 14610;  // Jan 1

uint64_t standard_day_number(long const gnss_id, long const day_number) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_1980;
    case GNSS_ID__gnss_id_galileo: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_1999;
    case GNSS_ID__gnss_id_glonass: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_1996;
    case GNSS_ID__gnss_id_bds: return static_cast<uint64_t>(day_number) + DAY_BETWEEN_1970_2006;
    default: UNREACHABLE();
    }
}

SpartnTime spartn_time_from(GNSS_SystemTime const& epoch_time) {
    auto time_of_day = static_cast<double>(epoch_time.gnss_TimeOfDay);
    if (epoch_time.gnss_TimeOfDayFrac_msec) {
        time_of_day += static_cast<double>(*epoch_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    }

    auto day_number      = epoch_time.gnss_DayNumber;
    auto gnss_id         = epoch_time.gnss_TimeID.gnss_id;
    auto days_since_1970 = standard_day_number(gnss_id, day_number);
    auto days_since_2010 = days_since_1970 - DAY_BETWEEN_1970_2010;

    auto seconds_since_2010  = days_since_2010 * SECONDS_IN_DAY;
    auto rounded_time_of_day = static_cast<uint32_t>(time_of_day + 0.5);
    auto seconds             = static_cast<uint32_t>(seconds_since_2010) + rounded_time_of_day;

    return SpartnTime{
        static_cast<double>(seconds_since_2010) + time_of_day,
        seconds,
    };
}
