#pragma once
#include <core/core.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <maths/float3.hpp>
#include <time/tai.hpp>

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

struct CorrectionPointInfo {
    long   absolute_index;
    long   array_index;
    bool   is_valid;
    long   latitude_index;
    long   longitude_index;
    Float3 position;
};

struct CorrectionPointSet {
    uint16_t set_id;
    long     grid_point_count;
    double   reference_point_latitude;
    double   reference_point_longitude;
    double   step_of_latitude;
    double   step_of_longitude;
    long     number_of_steps_latitude;
    long     number_of_steps_longitude;
    uint64_t bitmask;

    // Convert array index (the index of the grid point in the array from LPP) to an absolute index
    // that includes invalid grid points.
    NODISCARD bool array_to_index(long array_index, CorrectionPointInfo* result) const NOEXCEPT;

    NODISCARD double latitude_min() const NOEXCEPT { return reference_point_latitude; }
    NODISCARD double latitude_max() const NOEXCEPT {
        return reference_point_latitude +
               static_cast<double>(number_of_steps_latitude) * step_of_latitude;
    }
    NODISCARD double longitude_min() const NOEXCEPT { return reference_point_longitude; }
    NODISCARD double longitude_max() const NOEXCEPT {
        return reference_point_longitude +
               static_cast<double>(number_of_steps_longitude) * step_of_longitude;
    }
};

}  // namespace tokoro
}  // namespace generator
