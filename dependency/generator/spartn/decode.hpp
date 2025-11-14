#pragma once
#include <core/core.hpp>

struct BIT_STRING_s;
struct GNSS_SignalID;
struct STEC_ResidualSatElement_r16;
struct STEC_ResidualCorrection_r16;

namespace decode {

CONSTEXPR static double ORBIT_RADIAL_RESOLUTION                    = 0.0001;
CONSTEXPR static double ORBIT_ALONG_RESOLUTION                     = 0.0004;
CONSTEXPR static double ORBIT_CROSS_RESOLUTION                     = 0.0004;
CONSTEXPR static double CLOCK_C0_RESOLUTION                        = 0.0001;
CONSTEXPR static double CLOCK_C1_RESOLUTION                        = 0.000001;
CONSTEXPR static double CLOCK_C2_RESOLUTION                        = 0.00000002;
CONSTEXPR static double PHASE_BIAS_RESOLUTION                      = 0.001;
CONSTEXPR static double CODE_BIAS_RESOLUTION                       = 0.01;
CONSTEXPR static double REFERENCE_POINT_LATITUDE_DEG               = 90.0;
CONSTEXPR static double REFERENCE_POINT_LATITUDE_RESOLUTION        = 0.00006103515625;
CONSTEXPR static double REFERENCE_POINT_LONGITUDE_DEG              = 180.0;
CONSTEXPR static double REFERENCE_POINT_LONGITUDE_RESOLUTION       = 0.000030517578125;
CONSTEXPR static double STEP_OF_LONGITUDE_RESOLUTION               = 0.01;
CONSTEXPR static double STEP_OF_LATITUDE_RESOLUTION                = 0.01;
CONSTEXPR static double TROPOSPHERIC_HYDRO_STATIC_DELAY_RESOLUTION = 0.004;
CONSTEXPR static double TROPOSPHERIC_WET_DELAY_RESOLUTION          = 0.004;
CONSTEXPR static double STEC_C00_RESOLUTION                        = 0.05;
CONSTEXPR static double STEC_C01_RESOLUTION                        = 0.02;
CONSTEXPR static double STEC_C10_RESOLUTION                        = 0.02;
CONSTEXPR static double STEC_C11_RESOLUTION                        = 0.02;
CONSTEXPR static double STEC_RESIDUAL_B7_RESOLUTION                = 0.04;
CONSTEXPR static double STEC_RESIDUAL_B16_RESOLUTION               = 0.04;

double ssr_update_interval_r15(long value);
double delta_radial_r15(long value);
double delta_along_track_r15(long value);
double delta_cross_track_r15(long value);
double delta_clock_c0_r15(long value);
double delta_clock_c1_r15(long* value);
double delta_clock_c2_r15(long* value);
long   signal_id(GNSS_SignalID const& signal_id);
double phase_bias_r16(long value);
double code_bias_r15(long value);

struct QualityIndicator {
    bool   invalid;
    double value;
};

QualityIndicator quality_indicator(BIT_STRING_s& bit_string);
QualityIndicator ssr_ura_r16(BIT_STRING_s ura);
QualityIndicator tropospheric_delay_quality_indicator_r16(BIT_STRING_s& bit_string);
double           reference_point_latitude_r16(long value);
double           reference_point_longitude_r16(long value);
double           step_of_latitude_r16(long value);
double           step_of_longitude_r16(long value);
double           tropo_hydro_static_vertical_delay_r16(long value);
double           tropo_wet_vertical_delay_r16(long value);

struct StecQualityIndicator {
    bool   invalid;
    double value;
};

StecQualityIndicator stec_quality_indicator_r16(BIT_STRING_s& bit_string);
double               stec_c00_r16(long value);
double               stec_c01_r16(long* value);
double               stec_c10_r16(long* value);
double               stec_c11_r16(long* value);
double               stec_residual_correction_r16(STEC_ResidualSatElement_r16 const& element);

}  // namespace decode
