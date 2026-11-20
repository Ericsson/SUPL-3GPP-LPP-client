#pragma once
#include <core/core.hpp>

#include <string>

namespace generator {
namespace tokoro {

// How the gridded SSR corrections (STEC residual, troposphere) are interpolated to the position.
//
// For a regular grid (arrayOfCorrectionPoints) the position is interpolated within the grid cell
// that contains it. A list of correction points (listOfCorrectionPoints) has no cells, so there
// the nearest points are used instead.
enum class GridInterpolation {
    // Grid: bilinear over the 4 corners of the cell, all 4 must have data (default)
    // List: inverse distance weighting (1/d^2) of the 4 nearest points, which must surround the
    //       position
    FourPoint,
    // Grid: bilinear when all 4 corners have data, otherwise linear over the triangle of the 3
    //       corners that do, when the position is inside it (Compact SSR: "4 (or 3) reference
    //       grids")
    // List: linear over the smallest triangle of the nearest points that contains the position
    ThreePoint,
    // The value of the nearest point
    Nearest,
};

// What to do with a position that interpolation can not handle (outside of the grid, or not
// surrounded by enough points with data)
enum class GridExtrapolation {
    // The correction is not available (default)
    None,
    // The value of the nearest point with data
    Nearest,
};

NODISCARD char const* grid_interpolation_name(GridInterpolation interpolation) NOEXCEPT;
NODISCARD bool        parse_grid_interpolation(std::string const& name,
                                               GridInterpolation& interpolation) NOEXCEPT;

NODISCARD char const* grid_extrapolation_name(GridExtrapolation extrapolation) NOEXCEPT;
NODISCARD bool        parse_grid_extrapolation(std::string const& name,
                                               GridExtrapolation& extrapolation) NOEXCEPT;

// `a|b|c` lists, for help texts and error messages
NODISCARD std::string grid_interpolation_names();
NODISCARD std::string grid_extrapolation_names();

}  // namespace tokoro
}  // namespace generator
