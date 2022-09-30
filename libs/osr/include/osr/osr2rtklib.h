#pragma once

#include <rtklib.h>
#include "osr/osr.h"

constexpr static const char* msm_sig_gps[32] = {
    /* GPS: ref [13] table 3.5-87, ref [14][15] table 3.5-91 */
    "",   "1C", "1P", "1W", "1Y", "1M", "",   "2C",
    "2P", "2W", "2Y", "2M", /*  1-12 */
    "",   "",   "2S", "2L", "2X", "",   "",   "",
    "",   "5I", "5Q", "5X",                         /* 13-24 */
    "",   "",   "",   "",   "",   "1S", "1L", "1X", /* 25-32 */
};

constexpr static const char* msm_sig_glo[32] = {
    /* GLONASS: ref [13] table 3.5-93, ref [14][15] table 3.5-97 */
    "",   "1C", "1P", "", "", "", "", "2C", "2P", "", "3I",
    "3Q", "3X", "",   "", "", "", "", "",   "",   "", "",
    "",   "",   "",   "", "", "", "", "",   "",   "",
};

constexpr static const char* msm_sig_gal[32] = {
    /* Galileo: ref [15] table 3.5-100 */
    "",   "1C", "1A", "1B", "1X", "1Z", "",   "6C", "6A", "6B", "6X",
    "6Z", "",   "7I", "7Q", "7X", "",   "8I", "8Q", "8X", "",   "5I",
    "5Q", "5X", "",   "",   "",   "",   "",   "",   "",   "",
};

constexpr static const char* msm_sig_qzs[32] = {
    /* QZSS: ref [15] table 3.5-103 */
    "",   "1C", "", "",   "",   "",   "", "",   "6S", "6L", "6X",
    "",   "",   "", "2S", "2L", "2X", "", "",   "",   "",   "5I",
    "5Q", "5X", "", "",   "",   "",   "", "1S", "1L", "1X",
};

constexpr static const char* msm_sig_sbs[32] = {
    /* SBAS: ref [13] table 3.5-T+005 */
    "", "1C", "", "", "", "",   "",   "",   "", "", "", "", "", "", "", "",
    "", "",   "", "", "", "5I", "5Q", "5X", "", "", "", "", "", "", "", "",
};

constexpr static const char* msm_sig_cmp[32] = {
    /* BeiDou: ref [15] table 3.5-106 */
    "", "2I", "2Q", "2X", "",   "", "", "6I", "6Q", "6X", "",
    "", "",   "7I", "7Q", "7X", "", "", "",   "",   "",   "",
    "", "",   "",   "",   "",   "", "", "",   "",   "",
};

void osr2rtklib_convert(rtcm_t* rtcm, int system, RTK_Satellite** satellites,
                        int satellite_count);
