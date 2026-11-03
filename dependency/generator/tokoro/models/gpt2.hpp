#pragma once
#include <core/core.hpp>

#include <string>
#include <vector>

#include "generator/tokoro/troposphere.hpp"

namespace generator {
namespace tokoro {

// GPT2 5x5 degree grid (Lagler et al. 2013). Every grid point has the mean and the annual and
// semi-annual amplitudes (a0, A1, B1, A2, B2) of the pressure, temperature, specific humidity,
// temperature lapse rate and the VMF1 coefficients ah and aw.
struct Gpt2Grid {
    struct Node {
        double pressure[5];     // [Pa]
        double temperature[5];  // [K]
        double humidity[5];     // [kg/kg]
        double lapse[5];        // [K/m]
        double ah[5];
        double aw[5];
        double undulation;  // [m]
        double height;      // orthometric height of the grid point [m]
    };

    std::vector<Node> nodes;  // 36 x 72, from the north pole to the south pole, from 0 to 360 deg
};

struct Gpt2Values {
    double pressure;              // [hPa]
    double temperature;           // [C]
    double water_vapor_pressure;  // [hPa]
    double ah;
    double aw;
    double undulation;  // [m]
};

NODISCARD bool gpt2_load(Gpt2Grid& grid, std::string const& path);

// mjd: modified julian date, latitude/longitude: [rad], height: ellipsoidal height [m]
NODISCARD bool gpt2_evaluate(Gpt2Grid const& grid, double mjd, double latitude, double longitude,
                             double height, Gpt2Values& values) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator
