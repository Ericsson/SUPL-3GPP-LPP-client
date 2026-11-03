#include "saastamoinen.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, saastamoinen);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, saastamoinen)

namespace generator {
namespace tokoro {

double water_vapor_pressure(double temperature, double relative_humidity) NOEXCEPT {
    return 6.108 * relative_humidity *
           std::exp((17.15 * temperature - 4684.0) / (temperature - 38.45));
}

SaastamoinenDelay saastamoinen_delay(double latitude, double height, double pressure,
                                     double temperature, double vapor_pressure) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f, %f, %f, %f, %f", latitude * 180.0 / M_PI, height, pressure, temperature,
                     vapor_pressure);

    SaastamoinenDelay delay{};
    delay.hydrostatic =
        0.0022768 * pressure / (1.0 - 0.00266 * std::cos(2.0 * latitude) - 0.00028 * height / 1e3);
    delay.wet = 0.002277 * (1255.0 / temperature + 0.05) * vapor_pressure;
    VERBOSEF("hydrostatic: %+.8f", delay.hydrostatic);
    VERBOSEF("wet:         %+.8f", delay.wet);
    return delay;
}

bool saastamoinen_standard_atmosphere(double latitude, double height, double relative_humidity,
                                      SaastamoinenDelay& delay) NOEXCEPT {
    if (height < -100.0 || height > 1e4) return false;

    auto hgt         = height < 0.0 ? 0.0 : height;
    auto pressure    = 1013.25 * std::pow(1.0 - 2.2557e-5 * hgt, 5.2568);
    auto temperature = 15.0 - 6.5e-3 * hgt + 273.16;
    auto vapor       = water_vapor_pressure(temperature, relative_humidity);
    delay            = saastamoinen_delay(latitude, hgt, pressure, temperature, vapor);
    return true;
}

}  // namespace tokoro
}  // namespace generator
