#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "sv_id.hpp"

#include <generator/rtcm/satellite_id.hpp>
#include <generator/rtcm/signal_id.hpp>

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_STEC_Correction_r16;
struct GNSS_SSR_GriddedCorrection_r16;

namespace generator {
namespace tokoro {

struct CodeBiasCorrection {
    uint16_t ssr_iod;
    double   bias;
};

struct PhaseBiasCorrection {
    uint16_t ssr_iod;
    double   bias;
};

struct SignalCorrection {
    std::unordered_map<SignalId, CodeBiasCorrection>  code_bias;
    std::unordered_map<SignalId, PhaseBiasCorrection> phase_bias;
};


}  // namespace tokoro
}  // namespace generator
