#pragma once
#include <core/core.hpp>

#include <string>

namespace generator {
namespace tokoro {

// How the gridded SSR corrections (STEC residual, troposphere) are interpolated to the position.
enum class GridInterpolation {
    // Bilinear over the 4 corners of the grid cell, all 4 must have data (default)
    FourPoint,
    // Bilinear when all 4 corners have data, otherwise linear over the triangle of the 3 corners
    // that do, when the position is inside it (Compact SSR: "4 (or 3) reference grids")
    ThreePoint,
    // The value of the nearest corner of the grid cell
    Nearest,
};

NODISCARD char const* grid_interpolation_name(GridInterpolation interpolation) NOEXCEPT;
NODISCARD bool        parse_grid_interpolation(std::string const& name,
                                               GridInterpolation& interpolation) NOEXCEPT;

// `a|b|c` list, for help texts and error messages
NODISCARD std::string grid_interpolation_names();

}  // namespace tokoro
}  // namespace generator
