#pragma once
#include <core/core.hpp>
#include <time/tai.hpp>

struct GNSS_SystemTime;
struct STEC_ResidualSatElement_r16;
struct BIT_STRING_s;

namespace generator {
namespace tokoro {
namespace decode {

CONSTEXPR static double REFERENCE_POINT_LATITUDE_DEG               = 90.0;
CONSTEXPR static double REFERENCE_POINT_LATITUDE_RESOLUTION        = 0.00006103515625;
CONSTEXPR static double REFERENCE_POINT_LONGITUDE_DEG              = 180.0;
CONSTEXPR static double REFERENCE_POINT_LONGITUDE_RESOLUTION       = 0.000030517578125;
CONSTEXPR static double STEP_OF_LONGITUDE_RESOLUTION               = 0.01;
CONSTEXPR static double STEP_OF_LATITUDE_RESOLUTION                = 0.01;
CONSTEXPR static double CLOCK_C0_RESOLUTION                        = 0.0001;
CONSTEXPR static double CLOCK_C1_RESOLUTION                        = 0.000001;
CONSTEXPR static double CLOCK_C2_RESOLUTION                        = 0.00000002;
CONSTEXPR static double ORBIT_RADIAL_RESOLUTION                    = 0.0001;
CONSTEXPR static double ORBIT_ALONG_RESOLUTION                     = 0.0004;
CONSTEXPR static double ORBIT_CROSS_RESOLUTION                     = 0.0004;
CONSTEXPR static double ORBIT_DOT_RADIAL_RESOLUTION                = 0.001;
CONSTEXPR static double ORBIT_DOT_ALONG_RESOLUTION                 = 0.004;
CONSTEXPR static double ORBIT_DOT_CROSS_RESOLUTION                 = 0.004;
CONSTEXPR static double ORBIT_DOT_MM_TO_M                          = 0.001;
CONSTEXPR static double CODE_BIAS_RESOLUTION                       = 0.01;
CONSTEXPR static double PHASE_BIAS_RESOLUTION                      = 0.001;
CONSTEXPR static double TROPOSPHERIC_HYDRO_STATIC_DELAY_RESOLUTION = 0.004;
CONSTEXPR static double TROPOSPHERIC_WET_DELAY_RESOLUTION          = 0.004;
CONSTEXPR static double STEC_C00_RESOLUTION                        = 0.05;
CONSTEXPR static double STEC_C01_RESOLUTION                        = 0.02;
CONSTEXPR static double STEC_C10_RESOLUTION                        = 0.02;
CONSTEXPR static double STEC_C11_RESOLUTION                        = 0.02;
CONSTEXPR static double STEC_RESIDUAL_B7_RESOLUTION                = 0.04;
CONSTEXPR static double STEC_RESIDUAL_B16_RESOLUTION               = 0.04;

double  referencePointLatitude_r16(long value);
double  referencePointLongitude_r16(long value);
double  stepOfLatitude_r16(long value);
double  stepOfLongitude_r16(long value);
long    numberOfStepsLatitude_r16(long value);
long    numberOfStepsLongitude_r16(long value);
long    day_number(GNSS_SystemTime const& src_time);
double  time_of_day(GNSS_SystemTime const& src_time);
double  time_of_day_fraction(GNSS_SystemTime const& src_time);
ts::Tai epochTime_r15(GNSS_SystemTime const& src_time);
long    iod_ssr_r16(long value);
double  delta_Clock_C0_r15(long value);
double  delta_Clock_C1_r15(long* value);
double  delta_Clock_C2_r15(long* value);
double  ssrUpdateInterval_r15(long value);
double  delta_radial_r15(long value);
double  delta_AlongTrack_r15(long value);
double  delta_CrossTrack_r15(long value);
double  dot_delta_radial_r15(long* value);
double  dot_delta_AlongTrack_r15(long* value);
double  dot_delta_CrossTrack_r15(long* value);
double  codeBias_r15(long value);
double  phaseBias_r16(long value);
double  tropoHydroStaticVerticalDelay_r16(long value);
double  tropoWetVerticalDelay_r16(long value);
double  stec_C00_r16(long value);
double  stec_C01_r16(long* value);
double  stec_C10_r16(long* value);
double  stec_C11_r16(long* value);
double  stecResidualCorrection_r16(void const* correction);

struct StecQualityIndicator {
    bool   invalid;
    double value;
    int    cls;
    int    val;
};

StecQualityIndicator stecQualityIndicator_r16(BIT_STRING_s& bit_string);

}  // namespace decode
}  // namespace tokoro
}  // namespace generator
