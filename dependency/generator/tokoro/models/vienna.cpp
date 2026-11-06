#include "vienna.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, vienna);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, vienna)

namespace generator {
namespace tokoro {

double herring_mapping(double elevation, double a, double b, double c) NOEXCEPT {
    auto sine = std::sin(elevation);
    return (1.0 + a / (1.0 + b / (1.0 + c))) / (sine + a / (sine + b / (sine + c)));
}

ViennaMapping vmf1_mapping(double ah, double aw, double mjd, double latitude, double height,
                           double elevation, bool apply_height_correction) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f, %f, %f, %f, %f, %f", ah, aw, mjd, latitude * 180.0 / M_PI, height,
                     elevation * 180.0 / M_PI);

    constexpr double PI = 3.1415926535897932384626433;

    // Reference day is 28 January 1980
    auto doy = mjd - 44239.0 + 1.0 - 28.0;

    // Hydrostatic mapping function
    auto   c0h = 0.062;
    double c11h;
    double c10h;
    double phh;
    if (latitude < 0.0) {
        phh  = PI;
        c11h = 0.007;
        c10h = 0.002;
    } else {
        phh  = 0.0;
        c11h = 0.005;
        c10h = 0.001;
    }

    auto bh = 0.0029;
    auto ch = c0h + ((std::cos(doy / 365.25 * 2.0 * PI + phh) + 1.0) * c11h / 2.0 + c10h) *
                        (1.0 - std::cos(latitude));

    ViennaMapping result{};
    result.hydrostatic = herring_mapping(elevation, ah, bh, ch);

    // Height correction from Niell (1996)
    if (apply_height_correction) {
        auto aht = 2.53e-5;
        auto bht = 5.49e-3;
        auto cht = 1.14e-3;
        result.hydrostatic +=
            (1.0 / std::sin(elevation) - herring_mapping(elevation, aht, bht, cht)) * height /
            1000.0;
    }

    // Wet mapping function
    auto bw    = 0.00146;
    auto cw    = 0.04391;
    result.wet = herring_mapping(elevation, aw, bw, cw);

    VERBOSEF("hydrostatic: %+.14f", result.hydrostatic);
    VERBOSEF("wet:         %+.14f", result.wet);
    return result;
}

}  // namespace tokoro
}  // namespace generator
