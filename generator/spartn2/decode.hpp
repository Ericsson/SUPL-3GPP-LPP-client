#pragma once
#include <generator/spartn2/types.hpp>

#include <GNSS-SignalID.h>

namespace decode {

static SPARTN_CONSTEXPR double ORBIT_RADIAL_RESOLUTION = 0.0001;
static SPARTN_CONSTEXPR double ORBIT_ALONG_RESOLUTION  = 0.0004;
static SPARTN_CONSTEXPR double ORBIT_CROSS_RESOLUTION  = 0.0004;

static double delta_radial_r15(long value) {
    return value * ORBIT_RADIAL_RESOLUTION;
}

static double delta_AlongTrack_r15(long value) {
    return value * ORBIT_ALONG_RESOLUTION;
}

static double delta_CrossTrack_r15(long value) {
    return value * ORBIT_CROSS_RESOLUTION;
}

static SPARTN_CONSTEXPR double CLOCK_C0_RESOLUTION = 0.0001;
static SPARTN_CONSTEXPR double CLOCK_C1_RESOLUTION = 0.000001;
static SPARTN_CONSTEXPR double CLOCK_C2_RESOLUTION = 0.00000002;

static double delta_Clock_C0_r15(long value) {
    return value * CLOCK_C0_RESOLUTION;
}

static double delta_Clock_C1_r15(long* value) {
    return value ? (*value * CLOCK_C1_RESOLUTION) : 0.0;
}

static double delta_Clock_C2_r15(long* value) {
    return value ? (*value * CLOCK_C2_RESOLUTION) : 0.0;
}

static long signal_id(const GNSS_SignalID& signal_id) {
    if (signal_id.ext1 && signal_id.ext1->gnss_SignalID_Ext_r15) {
        return *signal_id.ext1->gnss_SignalID_Ext_r15;
    } else {
        return signal_id.gnss_SignalID;
    }
}

static SPARTN_CONSTEXPR double PHASE_BIAS_RESOLUTION = 0.001;

static double phaseBias_r16(long value) {
    return value * PHASE_BIAS_RESOLUTION;
}

}  // namespace decode
