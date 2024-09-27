#pragma once
#include <cmath>
#include <generator/spartn2/types.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#include <BIT_STRING.h>
#include <GNSS-SignalID.h>
#include <STEC-ResidualSatElement-r16.h>
#pragma GCC diagnostic pop

namespace decode {

inline double ssrUpdateInterval_r15(long value) {
    switch (value) {
    case 0: return 1.0;
    case 1: return 2.0;
    case 2: return 5.0;
    case 3: return 10.0;
    case 4: return 15.0;
    case 5: return 30.0;
    case 6: return 60.0;
    case 7: return 120.0;
    case 8: return 240.0;
    case 9: return 300.0;
    case 10: return 600.0;
    case 11: return 900.0;
    case 12: return 1800.0;
    case 13: return 3600.0;
    case 14: return 7200.0;
    case 15: return 10800.0;
    default: return 10800.0;
    }
}

static SPARTN_CONSTEXPR double ORBIT_RADIAL_RESOLUTION = 0.0001;
static SPARTN_CONSTEXPR double ORBIT_ALONG_RESOLUTION  = 0.0004;
static SPARTN_CONSTEXPR double ORBIT_CROSS_RESOLUTION  = 0.0004;

inline double delta_radial_r15(long value) {
    return static_cast<double>(value) * ORBIT_RADIAL_RESOLUTION;
}

inline double delta_AlongTrack_r15(long value) {
    return static_cast<double>(value) * ORBIT_ALONG_RESOLUTION;
}

inline double delta_CrossTrack_r15(long value) {
    return static_cast<double>(value) * ORBIT_CROSS_RESOLUTION;
}

static SPARTN_CONSTEXPR double CLOCK_C0_RESOLUTION = 0.0001;
static SPARTN_CONSTEXPR double CLOCK_C1_RESOLUTION = 0.000001;
static SPARTN_CONSTEXPR double CLOCK_C2_RESOLUTION = 0.00000002;

inline double delta_Clock_C0_r15(long value) {
    return static_cast<double>(value) * CLOCK_C0_RESOLUTION;
}

inline double delta_Clock_C1_r15(long* value) {
    return value ? (static_cast<double>(*value) * CLOCK_C1_RESOLUTION) : 0.0;
}

inline double delta_Clock_C2_r15(long* value) {
    return value ? (static_cast<double>(*value) * CLOCK_C2_RESOLUTION) : 0.0;
}

inline long signal_id(GNSS_SignalID const& signal_id) {
    if (signal_id.ext1 && signal_id.ext1->gnss_SignalID_Ext_r15) {
        return *signal_id.ext1->gnss_SignalID_Ext_r15;
    } else {
        return signal_id.gnss_SignalID;
    }
}

static SPARTN_CONSTEXPR double PHASE_BIAS_RESOLUTION = 0.001;
static SPARTN_CONSTEXPR double CODE_BIAS_RESOLUTION  = 0.01;

inline double phaseBias_r16(long value) {
    return static_cast<double>(value) * PHASE_BIAS_RESOLUTION;
}

inline double codeBias_r15(long value) {
    return static_cast<double>(value) * CODE_BIAS_RESOLUTION;
}

struct QualityIndicator {
    bool   invalid;
    double value;
};

inline QualityIndicator quality_indicator(BIT_STRING_s& bit_string) {
#if SPARTN_DEBUG_PRINT
    printf("---------- QUALITY INDICATOR %02X %d %d (%d %d)\n", bit_string.buf[0],
           bit_string.buf[0] >> 3, bit_string.buf[0] & 0x7, (bit_string.buf[0] >> 5) & 0x7,
           (bit_string.buf[0] >> 2) & 0x7);
#endif
    // auto cls     = (bit_string.buf[0] >> 3) & 0x7;
    // auto val     = bit_string.buf[0] & 0x7;
    auto cls     = (bit_string.buf[0] >> 5) & 0x7;
    auto val     = (bit_string.buf[0] >> 2) & 0x7;
    auto q       = pow(3, cls) * (1 + static_cast<double>(val) / 4.0) - 1;
    auto q_meter = q / 1000.0;
    return QualityIndicator{cls == 0 && val == 0, q_meter};
}

inline QualityIndicator ssr_URA_r16(BIT_STRING_s ura) {
    return quality_indicator(ura);
}

inline QualityIndicator troposphericDelayQualityIndicator_r16(BIT_STRING_s& bit_string) {
    return quality_indicator(bit_string);
}

static SPARTN_CONSTEXPR double REFERENCE_POINT_LATITUDE_DEG        = 90.0;
static SPARTN_CONSTEXPR double REFERENCE_POINT_LATITUDE_RESOLUTION = 0.00006103515625;  // 2^-14

static SPARTN_CONSTEXPR double REFERENCE_POINT_LONGITUDE_DEG        = 180.0;
static SPARTN_CONSTEXPR double REFERENCE_POINT_LONGITUDE_RESOLUTION = 0.000030517578125;  // 2^-15

inline double referencePointLatitude_r16(long value) {
    return (static_cast<double>(value) * REFERENCE_POINT_LATITUDE_DEG) *
           REFERENCE_POINT_LATITUDE_RESOLUTION;
}

inline double referencePointLongitude_r16(long value) {
    return (static_cast<double>(value) * REFERENCE_POINT_LONGITUDE_DEG) *
           REFERENCE_POINT_LONGITUDE_RESOLUTION;
}

static SPARTN_CONSTEXPR double STEP_OF_LONGITUDE_RESOLUTION = 0.01;
static SPARTN_CONSTEXPR double STEP_OF_LATITUDE_RESOLUTION  = 0.01;

inline double stepOfLatitude_r16(long value) {
    return static_cast<double>(value) * STEP_OF_LONGITUDE_RESOLUTION;
}

inline double stepOfLongitude_r16(long value) {
    return static_cast<double>(value) * STEP_OF_LATITUDE_RESOLUTION;
}

static SPARTN_CONSTEXPR double TROPOSPHERIC_HYDRO_STATIC_DELAY_RESOLUTION = 0.004;
static SPARTN_CONSTEXPR double TROPOSPHERIC_WET_DELAY_RESOLUTION          = 0.004;

inline double tropoHydroStaticVerticalDelay_r16(long value) {
    return static_cast<double>(value) * TROPOSPHERIC_HYDRO_STATIC_DELAY_RESOLUTION;
}

inline double tropoWetVerticalDelay_r16(long value) {
    return static_cast<double>(value) * TROPOSPHERIC_WET_DELAY_RESOLUTION;
}

struct StecQualityIndicator {
    bool   invalid;
    double value;
};

inline StecQualityIndicator stecQualityIndicator_r16(BIT_STRING_s& bit_string) {
    static SPARTN_CONSTEXPR double QUALITY_INDICATOR[64] = {
        33.6664, 30.2992, 26.9319, 23.5647, 20.1974, 16.8301, 13.4629, 12.3405, 11.2180,
        10.0956, 8.9732,  7.8508,  6.7284,  5.6059,  4.4835,  4.1094,  3.7352,  3.3611,
        2.9870,  2.6128,  2.2387,  1.8645,  1.4904,  1.3657,  1.2410,  1.1163,  0.9915,
        0.8668,  0.7421,  0.6174,  0.4927,  0.4511,  0.4096,  0.3680,  0.3264,  0.2848,
        0.2433,  0.2017,  0.1601,  0.1463,  0.1324,  0.1186,  0.1047,  0.0908,  0.0770,
        0.0631,  0.0493,  0.0447,  0.0400,  0.0354,  0.0308,  0.0262,  0.0216,  0.0169,
        0.0123,  0.0108,  0.0092,  0.0077,  0.0062,  0.0046,  0.0031,  0.0015,  0,
    };

    auto value = bit_string.buf[0];

    auto stec_cls = value & 0x7;
    auto stec_val = (value >> 3) & 0x7;
    stec_cls      = ((stec_cls & 0x1) << 2) | ((stec_cls & 0x2) << 0) | ((stec_cls & 0x4) >> 2);
    stec_val      = ((stec_val & 0x1) << 2) | ((stec_val & 0x2) << 0) | ((stec_val & 0x4) >> 2);
    auto index    = (8 * stec_cls) + stec_val;

    assert(index < 64);
    return StecQualityIndicator{
        index == 0,
        QUALITY_INDICATOR[63 - index],
    };
}

static SPARTN_CONSTEXPR double STEC_C00_RESOLUTION = 0.05;
static SPARTN_CONSTEXPR double STEC_C01_RESOLUTION = 0.02;
static SPARTN_CONSTEXPR double STEC_C10_RESOLUTION = 0.02;
static SPARTN_CONSTEXPR double STEC_C11_RESOLUTION = 0.02;

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

static SPARTN_CONSTEXPR double STEC_RESIDUAL_B7_RESOLUTION  = 0.04;
static SPARTN_CONSTEXPR double STEC_RESIDUAL_B16_RESOLUTION = 0.04;

inline double stecResidualCorrection_r16(
    STEC_ResidualSatElement_r16::STEC_ResidualSatElement_r16__stecResidualCorrection_r16
        correction) {
    switch (correction.present) {
    case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b7_r16:
        return static_cast<double>(correction.choice.b7_r16) * STEC_RESIDUAL_B7_RESOLUTION;
    case STEC_ResidualSatElement_r16__stecResidualCorrection_r16_PR_b16_r16:
        return static_cast<double>(correction.choice.b16_r16) * STEC_RESIDUAL_B16_RESOLUTION;
    default: SPARTN_UNREACHABLE();
    }
}

}  // namespace decode
