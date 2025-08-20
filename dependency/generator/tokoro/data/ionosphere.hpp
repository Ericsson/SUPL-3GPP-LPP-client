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

struct IonosphericCorrection {
    double grid_residual;
    double polynomial_residual;
    double vtec_grid_residual;
    double vtec_polynomial_residual;
    double quality;
    bool   quality_valid;
};

struct IonosphericPolynomial {
    double c00;
    double c01;
    double c10;
    double c11;
    double reference_point_latitude;
    double reference_point_longitude;
    double quality_indicator;
    bool quality_indicator_valid;
};

struct IonosphereGridPoint {
    Float3 position;
    double ionospheric;
};

struct IonosphereGrid {
    std::unordered_map<GridIndex, IonosphereGridPoint> points;
};

}  // namespace tokoro
}  // namespace generator
