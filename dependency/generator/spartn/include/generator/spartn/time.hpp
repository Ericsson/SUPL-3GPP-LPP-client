#ifndef SPARTN_LPP_Time_
#define SPARTN_LPP_Time_

#include <generator/spartn/constants.h>
#include "GNSS-ID.h"

#include <ctime>

class SPARTN_LPP_Time {
public:
    SPARTN_LPP_Time();
    SPARTN_LPP_Time(long id, long dayno, long tod);
    /** Getter function for spartn_time_
     *
     * @return spartn_time_
     */
    uint32_t get_spartn_time() const;

    /** Different constellations use different start days for their reference
     * time, e.g. GPS starts at 1980 whereas Galileo uses 1999. This function
     * normalises these start days to all start at 1970 (same as a UNIX
     * timestamp).
     *
     * @param[in] day_number day delta to be normalised.
     * @param[in] gnss_id    constellation the day delta is for.
     *
     * @return               normalised day delta to 01/01/1970
     */
    static uint64_t standard_day_number(long day_number, GNSS_ID__gnss_id gnss_id);

private:
    /** Used to generate time for the 32-bit version of TF009, who's reference
     * time is 01/01/2010 00:00:00. The resulting time-tag will be stored in
     * spartn_time_.
     */
    void generate_spartn_time();

    GNSS_ID__gnss_id lpp_gnss_id_;
    long             lpp_daynumber_;
    long             lpp_timeofday_;

    uint32_t spartn_time_;
};

#endif
