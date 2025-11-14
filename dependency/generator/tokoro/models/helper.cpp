#include "helper.hpp"

#include <cmath>
#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(tokoro, helper);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, helper)

namespace generator {
namespace tokoro {

static double mapping_function(double el, double a, double b, double c) {
    double sinel = std::sin(el);
    return (1.0 + a / (1.0 + b / (1.0 + c))) / (sinel + (a / (sinel + b / (sinel + c))));
}

static CONSTEXPR double MAP_COEF[][5] = {
    {1.2769934E-3, 1.2683230E-3, 1.2465397E-3, 1.2196049E-3, 1.2045996E-3},
    {2.9153695E-3, 2.9152299E-3, 2.9288445E-3, 2.9022565E-3, 2.9024912E-3},
    {62.610505E-3, 62.837393E-3, 63.721774E-3, 63.824265E-3, 64.258455E-3},

    {0.0000000E-0, 1.2709626E-5, 2.6523662E-5, 3.4000452E-5, 4.1202191E-5},
    {0.0000000E-0, 2.1414979E-5, 3.0160779E-5, 7.2562722E-5, 11.723375E-5},
    {0.0000000E-0, 9.0128400E-5, 4.3497037E-5, 84.795348E-5, 170.37206E-5},

    {5.8021897E-4, 5.6794847E-4, 5.8118019E-4, 5.9727542E-4, 6.1641693E-4},
    {1.4275268E-3, 1.5138625E-3, 1.4572752E-3, 1.5007428E-3, 1.7599082E-3},
    {4.3472961E-2, 4.6729510E-2, 4.3908931E-2, 4.4626982E-2, 5.4736038E-2},
};

static double interpolate_coef(double lat_deg, double const coef[5]) {
    auto lat_abs = std::fabs(lat_deg);
    auto index   = static_cast<int>(lat_abs / 15.0);

    if (index < 0) {
        return coef[0];
    } else if (index >= 4) {
        return coef[4];
    }

    auto coef0 = coef[index];
    auto coef1 = coef[index + 1];
    auto frac  = lat_abs / 15.0 - index;
    return coef0 * (1.0 - frac) + coef1 * frac;
}

static double delta_m(double elevation, double ellipsoidal_height) {
    auto a_ht = 2.53E-5;
    auto b_ht = 5.49E-3;
    auto c_ht = 1.14E-3;
    return ((1.0 / std::sin(elevation)) - mapping_function(elevation, a_ht, b_ht, c_ht)) *
           ellipsoidal_height;
}

HydrostaticAndWetMapping hydrostatic_mapping_function(ts::Tai time, Float3 position,
                                                      double elevation) {
    VSCOPE_FUNCTIONF("%s, (%f, %f, %f), %f", ts::Utc{time}.rtklib_time_string().c_str(),
                     position.x * constant::RAD2DEG, position.y * constant::RAD2DEG, position.z,
                     elevation * constant::RAD2DEG);

    if (elevation < 0.0) {
        return {0.0, 0.0};
    }

    auto day_of_year = ts::Utc{time}.day_of_year();
    auto time_y      = (day_of_year - 28.0) / 365.25;
    // TODO(ewasjon): This is something RTKLIB does but it isn't mentioned in
    // https://gssc.esa.int/navipedia/index.php/Mapping_of_Niell
    if (position.x < 0) {
        time_y += 0.5;
    }

    VERBOSEF("day_of_year: %f", day_of_year);
    VERBOSEF("time_y: %f", time_y);

    auto lat_deg = position.x * constant::RAD2DEG;
    auto cos_y   = std::cos(2.0 * constant::PI * time_y);

    auto a_d =
        interpolate_coef(lat_deg, MAP_COEF[0]) - interpolate_coef(lat_deg, MAP_COEF[3]) * cos_y;
    auto b_d =
        interpolate_coef(lat_deg, MAP_COEF[1]) - interpolate_coef(lat_deg, MAP_COEF[4]) * cos_y;
    auto c_d =
        interpolate_coef(lat_deg, MAP_COEF[2]) - interpolate_coef(lat_deg, MAP_COEF[5]) * cos_y;

    VERBOSEF("a_d: %+f", a_d);
    VERBOSEF("b_d: %+f", b_d);
    VERBOSEF("c_d: %+f", c_d);

    auto height = position.z / 1.0e3;

    auto hydrostatic         = mapping_function(elevation, a_d, b_d, c_d);
    auto hydrostatic_delta_m = delta_m(elevation, height);

    auto a_w = interpolate_coef(lat_deg, MAP_COEF[6]);
    auto b_w = interpolate_coef(lat_deg, MAP_COEF[7]);
    auto c_w = interpolate_coef(lat_deg, MAP_COEF[8]);

    VERBOSEF("a_w: %+f", a_w);
    VERBOSEF("b_w: %+f", b_w);
    VERBOSEF("c_w: %+f", c_w);

    auto wet = mapping_function(elevation, a_w, b_w, c_w);

    VERBOSEF("delta_m: %+f", hydrostatic_delta_m);
    VERBOSEF("hydrostatic: %+f", hydrostatic);
    VERBOSEF("wet: %+f", wet);

    return {hydrostatic, wet};
}

}  // namespace tokoro
}  // namespace generator
