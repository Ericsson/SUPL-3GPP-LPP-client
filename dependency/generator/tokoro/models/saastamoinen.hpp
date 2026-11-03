#pragma once
#include <core/core.hpp>

namespace generator {
namespace tokoro {

struct SaastamoinenDelay {
    double hydrostatic;
    double wet;
};

// Water vapor pressure [hPa] for a relative humidity (0..1) at the temperature [K]
NODISCARD double water_vapor_pressure(double temperature, double relative_humidity) NOEXCEPT;

// Saastamoinen zenith delays [m]
//  latitude:             geodetic latitude [rad]
//  height:               ellipsoidal height [m]
//  pressure:             [hPa]
//  temperature:          [K]
//  water_vapor_pressure: [hPa]
NODISCARD SaastamoinenDelay saastamoinen_delay(double latitude, double height, double pressure,
                                               double temperature,
                                               double water_vapor_pressure) NOEXCEPT;

// Saastamoinen zenith delays using the standard atmosphere (pressure and temperature as function
// of the height) and a fixed relative humidity, the same atmosphere as RTKLIB `tropmodel`. Only
// valid for heights from -100 m to 10 km.
NODISCARD bool saastamoinen_standard_atmosphere(double latitude, double height,
                                                double             relative_humidity,
                                                SaastamoinenDelay& delay) NOEXCEPT;

}  // namespace tokoro
}  // namespace generator
