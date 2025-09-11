#pragma once
#include <core/core.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <GNSS-SystemTime.h>
#include <STEC-ResidualSatElement-r16.h>
#pragma GCC diagnostic pop

#include <time/bdt.hpp>
#include <time/glo.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {
namespace decode {

CONSTEXPR static double REFERENCE_POINT_LATITUDE_DEG        = 90.0;
CONSTEXPR static double REFERENCE_POINT_LATITUDE_RESOLUTION = 0.00006103515625;  // 2^-14

CONSTEXPR static double REFERENCE_POINT_LONGITUDE_DEG        = 180.0;
CONSTEXPR static double REFERENCE_POINT_LONGITUDE_RESOLUTION = 0.000030517578125;  // 2^-15

inline double referencePointLatitude_r16(long value) {
    return (static_cast<double>(value) * REFERENCE_POINT_LATITUDE_DEG) *
           REFERENCE_POINT_LATITUDE_RESOLUTION;
}

inline double referencePointLongitude_r16(long value) {
    return (static_cast<double>(value) * REFERENCE_POINT_LONGITUDE_DEG) *
           REFERENCE_POINT_LONGITUDE_RESOLUTION;
}

CONSTEXPR static double STEP_OF_LONGITUDE_RESOLUTION = 0.01;
CONSTEXPR static double STEP_OF_LATITUDE_RESOLUTION  = 0.01;

inline double stepOfLatitude_r16(long value) {
    return static_cast<double>(value) * STEP_OF_LONGITUDE_RESOLUTION;
}

inline double stepOfLongitude_r16(long value) {
    return static_cast<double>(value) * STEP_OF_LATITUDE_RESOLUTION;
}

inline long numberOfStepsLatitude_r16(long value) {
    return value;
}

inline long numberOfStepsLongitude_r16(long value) {
    return value;
}

inline long day_number(GNSS_SystemTime const& src_time) {
    return src_time.gnss_DayNumber;
}

inline double time_of_day(GNSS_SystemTime const& src_time) {
    return static_cast<double>(src_time.gnss_TimeOfDay);
}

inline double time_of_day_fraction(GNSS_SystemTime const& src_time) {
    if (src_time.gnss_TimeOfDayFrac_msec) {
        return static_cast<double>(*src_time.gnss_TimeOfDayFrac_msec) / 1000.0;
    } else {
        return 0.0;
    }
}

inline ts::Tai epochTime_r15(GNSS_SystemTime const& src_time) {
    auto day_number           = decode::day_number(src_time);
    auto time_of_day_seconds  = decode::time_of_day(src_time);
    auto time_of_day_fraction = decode::time_of_day_fraction(src_time);
    auto time_of_day          = time_of_day_seconds + time_of_day_fraction;

    switch (src_time.gnss_TimeID.gnss_id) {
    case GNSS_ID__gnss_id_gps: return ts::Tai(ts::Gps::from_day_tod(day_number, time_of_day));
    case GNSS_ID__gnss_id_glonass: return ts::Tai(ts::Glo::from_day_tod(day_number, time_of_day));
    case GNSS_ID__gnss_id_galileo: return ts::Tai(ts::Gst::from_day_tod(day_number, time_of_day));
    case GNSS_ID__gnss_id_bds: return ts::Tai(ts::Bdt::from_day_tod(day_number, time_of_day));
    default: return ts::Tai::now();
    }
}

inline long iod_ssr_r16(long value) {
    return value;
}

static CONSTEXPR double CLOCK_C0_RESOLUTION = 0.0001;
static CONSTEXPR double CLOCK_C1_RESOLUTION = 0.000001;
static CONSTEXPR double CLOCK_C2_RESOLUTION = 0.00000002;

inline double delta_Clock_C0_r15(long value) {
    return static_cast<double>(value) * CLOCK_C0_RESOLUTION;
}

inline double delta_Clock_C1_r15(long* value) {
    return value ? (static_cast<double>(*value) * CLOCK_C1_RESOLUTION) : 0.0;
}

inline double delta_Clock_C2_r15(long* value) {
    return value ? (static_cast<double>(*value) * CLOCK_C2_RESOLUTION) : 0.0;
}

inline double ssrUpdateInterval_r15(long value) {
    if (value <= 0) return 1.0;
    if (value == 1) return 2.0;
    if (value == 2) return 5.0;
    if (value == 3) return 10.0;
    if (value == 4) return 15.0;
    if (value == 5) return 30.0;
    if (value == 6) return 60.0;
    if (value == 7) return 120.0;
    if (value == 8) return 240.0;
    if (value == 9) return 300.0;
    if (value == 10) return 600.0;
    if (value == 11) return 900.0;
    if (value == 12) return 1800.0;
    if (value == 13) return 3600.0;
    if (value == 14) return 7200.0;
    if (value == 15) return 10800.0;
    return 10800.0;
}

static CONSTEXPR double ORBIT_RADIAL_RESOLUTION = 0.0001;
static CONSTEXPR double ORBIT_ALONG_RESOLUTION  = 0.0004;
static CONSTEXPR double ORBIT_CROSS_RESOLUTION  = 0.0004;

inline double delta_radial_r15(long value) {
    return static_cast<double>(value) * ORBIT_RADIAL_RESOLUTION;
}

inline double delta_AlongTrack_r15(long value) {
    return static_cast<double>(value) * ORBIT_ALONG_RESOLUTION;
}

inline double delta_CrossTrack_r15(long value) {
    return static_cast<double>(value) * ORBIT_CROSS_RESOLUTION;
}

static CONSTEXPR double ORBIT_DOT_RADIAL_RESOLUTION = 0.001;
static CONSTEXPR double ORBIT_DOT_ALONG_RESOLUTION  = 0.004;
static CONSTEXPR double ORBIT_DOT_CROSS_RESOLUTION  = 0.004;
static CONSTEXPR double ORBIT_DOT_MM_TO_M           = 0.001;

inline double dot_delta_radial_r15(long* value) {
    if (value) {
        return static_cast<double>(*value) * ORBIT_RADIAL_RESOLUTION * ORBIT_DOT_MM_TO_M;
    } else {
        return 0.0;
    }
}

inline double dot_delta_AlongTrack_r15(long* value) {
    if (value) {
        return static_cast<double>(*value) * ORBIT_ALONG_RESOLUTION * ORBIT_DOT_MM_TO_M;
    } else {
        return 0.0;
    }
}

inline double dot_delta_CrossTrack_r15(long* value) {
    if (value) {
        return static_cast<double>(*value) * ORBIT_CROSS_RESOLUTION * ORBIT_DOT_MM_TO_M;
    } else {
        return 0.0;
    }
}

static CONSTEXPR double CODE_BIAS_RESOLUTION = 0.01;

inline double codeBias_r15(long value) {
    return static_cast<double>(value) * CODE_BIAS_RESOLUTION;
}

static CONSTEXPR double PHASE_BIAS_RESOLUTION = 0.001;

inline double phaseBias_r16(long value) {
    return static_cast<double>(value) * PHASE_BIAS_RESOLUTION;
}

static CONSTEXPR double TROPOSPHERIC_HYDRO_STATIC_DELAY_RESOLUTION = 0.004;
static CONSTEXPR double TROPOSPHERIC_WET_DELAY_RESOLUTION          = 0.004;

inline double tropoHydroStaticVerticalDelay_r16(long value) {
    return static_cast<double>(value) * TROPOSPHERIC_HYDRO_STATIC_DELAY_RESOLUTION;
}

inline double tropoWetVerticalDelay_r16(long value) {
    return static_cast<double>(value) * TROPOSPHERIC_WET_DELAY_RESOLUTION;
}

static CONSTEXPR double STEC_C00_RESOLUTION = 0.05;
static CONSTEXPR double STEC_C01_RESOLUTION = 0.02;
static CONSTEXPR double STEC_C10_RESOLUTION = 0.02;
static CONSTEXPR double STEC_C11_RESOLUTION = 0.02;

inline double stec_C00_r16(long value) {
    return static_cast<double>(value) * STEC_C00_RESOLUTION;
}

inline double stec_C01_r16(long* value) {
    return (value ? static_cast<double>(*value) : 0) * STEC_C01_RESOLUTION;
}

inline double stec_C10_r16(long* value) {
    return (value ? static_cast<double>(*value) : 0) * STEC_C10_RESOLUTION;
}

inline double stec_C11_r16(long* value) {
    return (value ? static_cast<double>(*value) : 0) * STEC_C11_RESOLUTION;
}

static CONSTEXPR double STEC_RESIDUAL_B7_RESOLUTION  = 0.04;
static CONSTEXPR double STEC_RESIDUAL_B16_RESOLUTION = 0.04;

inline double stecResidualCorrection_r16(
    STEC_ResidualSatElement_r16::STEC_ResidualSatElement_r16__stecResidualCorrection_r16
        correction) {
    switch (correction.present) {
    case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b7_r16:
        return static_cast<double>(correction.choice.b7_r16) * STEC_RESIDUAL_B7_RESOLUTION;
    case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b16_r16:
        return static_cast<double>(correction.choice.b16_r16) * STEC_RESIDUAL_B16_RESOLUTION;
    case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_NOTHING:
        CORE_UNREACHABLE();
        CORE_UNREACHABLE_CASE;
    }
}

struct StecQualityIndicator {
    bool   invalid;
    double value;
    int    cls;
    int    val;
};

inline StecQualityIndicator stecQualityIndicator_r16(BIT_STRING_s& bit_string) {
    static CONSTEXPR double QUALITY_INDICATOR[64] = {
        33.6664, 30.2992, 26.9319, 23.5647, 20.1974, 16.8301, 13.4629, 12.3405, 11.2180,
        10.0956, 8.9732,  7.8508,  6.7284,  5.6059,  4.4835,  4.1094,  3.7352,  3.3611,
        2.9870,  2.6128,  2.2387,  1.8645,  1.4904,  1.3657,  1.2410,  1.1163,  0.9915,
        0.8668,  0.7421,  0.6174,  0.4927,  0.4511,  0.4096,  0.3680,  0.3264,  0.2848,
        0.2433,  0.2017,  0.1601,  0.1463,  0.1324,  0.1186,  0.1047,  0.0908,  0.0770,
        0.0631,  0.0493,  0.0447,  0.0400,  0.0354,  0.0308,  0.0262,  0.0216,  0.0169,
        0.0123,  0.0108,  0.0092,  0.0077,  0.0062,  0.0046,  0.0031,  0.0015,  0,
    };

    auto value = bit_string.buf[0];
    auto cls   = (value >> 5) & 0x7;
    auto val   = (value >> 2) & 0x7;
    auto index = (8 * cls) + val;

    assert(index < 64);
    return StecQualityIndicator{
        index == 0,
        QUALITY_INDICATOR[63 - index],
        cls,
        val,
    };
}

}  // namespace decode
}  // namespace tokoro
}  // namespace generator
