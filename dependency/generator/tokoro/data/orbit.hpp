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

struct OrbitCorrection {
    ts::Tai  reference_time;
    uint16_t ssr_iod;
    uint16_t iod;
    Float3   delta;      // {radial, along_track, cross_track}
    Float3   dot_delta;  // {radial, along_track, cross_track}

    NODISCARD bool correction(ts::Tai time, Float3 eph_position, Float3 eph_velocity,
                              Float3& result, Float3* output_radial, Float3* output_along,
                              Float3* output_cross, double* output_delta) const NOEXCEPT;
};

}  // namespace tokoro
}  // namespace generator
