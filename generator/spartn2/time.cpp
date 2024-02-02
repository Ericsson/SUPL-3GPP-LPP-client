#include "time.hpp"

#include <utility/time.h>
#include "GNSS-SystemTime.h"

static SPARTN_CONSTEXPR uint32_t SECONDS_IN_DAY        = 86400;
static SPARTN_CONSTEXPR uint16_t DAY_BETWEEN_1970_1980 = 3657;   // Jan 6
static SPARTN_CONSTEXPR uint16_t DAY_BETWEEN_1970_1996 = 9496;   // Jan 1
static SPARTN_CONSTEXPR uint16_t DAY_BETWEEN_1970_1999 = 10825;  // Aug 2
static SPARTN_CONSTEXPR uint16_t DAY_BETWEEN_1970_2006 = 13149;  // Jan 1
static SPARTN_CONSTEXPR uint16_t DAY_BETWEEN_1970_2010 = 14610;  // Jan 1

uint64_t standard_day_number(const long gnss_id, const long day_number) {
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: return day_number + DAY_BETWEEN_1970_1980;
    case GNSS_ID__gnss_id_galileo: return day_number + DAY_BETWEEN_1970_1999;
    case GNSS_ID__gnss_id_glonass: return day_number + DAY_BETWEEN_1970_1996;
    case GNSS_ID__gnss_id_bds: return day_number + DAY_BETWEEN_1970_2006;
    default: SPARTN_UNREACHABLE();
    }
}

SpartnTime spartn_time_from(const GNSS_SystemTime& epoch_time) {
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
