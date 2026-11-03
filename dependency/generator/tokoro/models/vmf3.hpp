#pragma once
#include <core/core.hpp>

#include <string>
#include <vector>

#include "generator/tokoro/troposphere.hpp"

namespace generator {
namespace tokoro {

// Gridded VMF3 (Landskron and Boehm 2018). The grid files contain, for every grid point, the
// hydrostatic and wet coefficients ah and aw and the zenith hydrostatic and wet delays for one
// epoch (every 6 hours). The orography file contains the height of every grid point.
struct Vmf3Grid {
    struct Value {
        double ah;
        double aw;
        double zhd;
        double zwd;
    };

    struct Epoch {
        double             mjd;
        std::vector<Value> values;  // in the order of the grid file
    };

    std::vector<double> latitudes;   // sorted, [deg]
    std::vector<double> longitudes;  // sorted, [0, 360) [deg]
    // grid file row of the grid point (latitude index * longitude count + longitude index)
    std::vector<size_t> rows;
    // latitude and longitude of the rows of the grid file, [deg]
    std::vector<double> row_latitude;
    std::vector<double> row_longitude;
    std::vector<Epoch>  epochs;     // sorted by time
    std::vector<double> orography;  // in the order of the grid file, [m]
};

NODISCARD bool vmf3_load_grid(Vmf3Grid& grid, std::string const& path);
NODISCARD bool vmf3_load_orography(Vmf3Grid& grid, std::string const& path);

// Interpolated and height reduced values for a station
struct Vmf3Values {
    double zhd;  // zenith hydrostatic delay [m]
    double zwd;  // zenith wet delay [m]
    double ah;
    double aw;
};

// mjd: modified julian date, latitude/longitude: [rad], height: ellipsoidal height [m]
NODISCARD bool vmf3_evaluate(Vmf3Grid const& grid, double mjd, double latitude, double longitude,
                             double height, Vmf3Values& values) NOEXCEPT;

struct Vmf3Mapping {
    double hydrostatic;
    double wet;
};

// VMF3 mapping function with the coefficients ah and aw of `vmf3_evaluate`.
//  elevation: [rad]
NODISCARD Vmf3Mapping vmf3_mapping(double ah, double aw, double mjd, double latitude,
                                   double longitude, double height, double elevation,
                                   bool apply_height_correction) NOEXCEPT;

namespace vmf3_coefficients {
extern double const anm_bh[91][5];
extern double const anm_bw[91][5];
extern double const anm_ch[91][5];
extern double const anm_cw[91][5];
extern double const bnm_bh[91][5];
extern double const bnm_bw[91][5];
extern double const bnm_ch[91][5];
extern double const bnm_cw[91][5];
}  // namespace vmf3_coefficients

}  // namespace tokoro
}  // namespace generator
