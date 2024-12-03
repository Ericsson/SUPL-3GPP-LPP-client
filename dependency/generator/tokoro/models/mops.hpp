#pragma once
#include <core/core.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct Mops {
    double pressure;
    double temperature;
    double water_pressure;
    double lapse_temperature;
    double water_vapor;
};

bool evaluate_mops(ts::Tai const& time, double latitude, Mops& result);

struct HydrostaticAndWetDelay {
    double hydrostatic;
    double wet;
};

bool mops_tropospheric_delay(ts::Tai const& time, double latitude, double ellipsoidal_height,
                             double geoid_height, HydrostaticAndWetDelay& result);

}  // namespace tokoro
}  // namespace generator
