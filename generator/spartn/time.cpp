#include "time.hpp"

SPARTN_LPP_Time::SPARTN_LPP_Time() {
    const std::time_t t = std::time(0);
    this->spartn_time_  = t - Constants::second_delta_1970_2010;
}

SPARTN_LPP_Time::SPARTN_LPP_Time(const long id, const long dayno, const long tod) {
    this->lpp_gnss_id_   = (GNSS_ID__gnss_id)id;
    this->lpp_daynumber_ = dayno;
    this->lpp_timeofday_ = tod;

    this->generate_spartn_time();
}

uint32_t SPARTN_LPP_Time::get_spartn_time() const {
    return this->spartn_time_;
}

void SPARTN_LPP_Time::generate_spartn_time() {
    const auto standard_day_number =
        SPARTN_LPP_Time::standard_day_number(this->lpp_daynumber_, this->lpp_gnss_id_);
    const uint32_t days_since_2010 = standard_day_number - Constants::day_delta_1970_2010;

    this->spartn_time_ =
        (days_since_2010 * Constants::seconds_in_day) + (uint32_t)this->lpp_timeofday_;
}

uint64_t SPARTN_LPP_Time::standard_day_number(const long             day_number,
                                              const GNSS_ID__gnss_id gnss_id) {
    uint16_t day_delta = 0;
    switch (gnss_id) {
    case GNSS_ID__gnss_id_gps: {
        day_delta = Constants::day_delta_1970_1980;
        break;
    }
    case GNSS_ID__gnss_id_galileo: {
        day_delta = Constants::day_delta_1970_1999;
        break;
    }
    case GNSS_ID__gnss_id_glonass: {
        day_delta = Constants::day_delta_1970_1996;
        break;
    }
    case GNSS_ID__gnss_id_bds: {
        day_delta = Constants::day_delta_1970_2006;
        break;
    }
    default: {
        day_delta = 0;
        break;
    }
    }
    return day_number + day_delta;
}
