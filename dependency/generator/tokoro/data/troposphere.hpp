#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <maths/float3.hpp>
#include <time/tai.hpp>

#include "grid.hpp"
#include "sv_id.hpp"

#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>

struct GNSS_SSR_OrbitCorrections_r15;
struct GNSS_SSR_ClockCorrections_r15;
struct GNSS_SSR_CodeBias_r15;
struct GNSS_SSR_PhaseBias_r16;
struct GNSS_SSR_STEC_Correction_r16;
struct GNSS_SSR_GriddedCorrection_r16;

namespace generator {
namespace tokoro {

struct TroposphericCorrection {
    double wet;
    double dry;

    // Horizontal slope of the zenith delays across the grid cell of the position, [m/m]
    bool   has_gradient;
    double dry_north;
    double dry_east;
    double wet_north;
    double wet_east;
};

struct TroposphereGridPoint {
    Float3                 position;
    TroposphericCorrection tropospheric;
};

struct TroposphereGrid {
    std::unordered_map<GridIndex, TroposphereGridPoint> points;
};

}  // namespace tokoro
}  // namespace generator
