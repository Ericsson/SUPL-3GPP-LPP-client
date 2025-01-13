#include "mops.hpp"
#include "constant.hpp"

#include <loglet/loglet.hpp>
#include <math.h>
#include <time/utc.hpp>

#define LOGLET_CURRENT_MODULE "tokoro"

namespace generator::tokoro {

// TODO(ewasjon): Investigate ny + DminS
static double interpolate(double const* x, double const* y, int nx, UNUSED int ny, double xi) {
    int    i;
    double dx;

    for (i = 0; i < nx - 1; i++)
        if (xi < x[i + 1]) break;
    dx = (xi - x[i]) / (x[i + 1] - x[i]);

    return y[i] * (1.0 - dx) + y[i + 1] * dx;
}

bool evaluate_mops(ts::Tai const& time, double latitude, Mops& result) {
    VSCOPE_FUNCTIONF("%s, %+.8f", time.rtklib_time_string().c_str(), latitude * constant::RAD2DEG);

    auto                latdeg     = latitude * constant::RAD2DEG;
    double const        dminN      = 28;
    UNUSED double const DminS      = 211; /* 28@north hemisphere, 211@south hemisphere */
    double const        interval[] = {15.0, 30.0, 45.0, 60.0, 75.0};

    /*Meteorological parameter Average table */
    double const pTave[]  = {1013.25, 1017.25, 1015.75, 1011.75, 1013.00};
    double const tTave[]  = {299.65, 294.15, 283.15, 272.15, 263.65};
    double const wTave[]  = {26.31, 21.79, 11.66, 6.78, 4.11};
    double const tlTave[] = {6.30e-3, 6.05e-3, 5.58e-3, 5.39e-3, 4.53e-3};
    double const wlTave[] = {2.77, 3.15, 2.57, 1.81, 1.55};
    /*Meteorological parameter Seasonal Variation table */
    double const pTableS[]  = {0.00, -3.75, -2.25, -1.75, -0.50};
    double const tTableS[]  = {0.00, 7.00, 11.00, 15.00, 14.50};
    double const wTableS[]  = {0.00, 8.85, 7.24, 5.36, 3.39};
    double const tlTableS[] = {0.00e-3, 0.25e-3, 0.32e-3, 0.81e-3, 0.62e-3};
    double const wlTableS[] = {0.00, 0.33, 0.46, 0.74, 0.30};
    /* at 15 30 45 60 75 [deg] */
    double calcPsta[5] = {0}, calcTsta[5] = {0}, calcWsta[5] = {0}, calcTLsta[5] = {0},
           calcWLsta[5] = {0};

    auto doy = ts::Utc{time}.day_of_year();
    for (auto i = 0; i < 5; i++) {
        auto cos_    = std::cos(2.0 * constant::PI * (doy - dminN) / 365.25);
        calcPsta[i]  = pTave[i] - pTableS[i] * cos_;
        calcTsta[i]  = tTave[i] - tTableS[i] * cos_;
        calcWsta[i]  = wTave[i] - wTableS[i] * cos_;
        calcTLsta[i] = tlTave[i] - tlTableS[i] * cos_;
        calcWLsta[i] = wlTave[i] - wlTableS[i] * cos_;
    }

    /* interpolation */
    if (75.0 <= latdeg) {
        result.pressure          = calcPsta[4];
        result.temperature       = calcTsta[4];
        result.water_pressure    = calcWsta[4];
        result.lapse_temperature = calcTLsta[4];
        result.water_vapor       = calcWLsta[4];
    } else if (latdeg <= 15.0) {
        result.pressure          = calcPsta[0];
        result.temperature       = calcTsta[0];
        result.water_pressure    = calcWsta[0];
        result.lapse_temperature = calcTLsta[0];
        result.water_vapor       = calcWLsta[0];
    } else if (15.0 < latdeg && latdeg < 75.0) {
        result.pressure          = interpolate(interval, calcPsta, 5, 5, latdeg);
        result.temperature       = interpolate(interval, calcTsta, 5, 5, latdeg);
        result.water_pressure    = interpolate(interval, calcWsta, 5, 5, latdeg);
        result.lapse_temperature = interpolate(interval, calcTLsta, 5, 5, latdeg);
        result.water_vapor       = interpolate(interval, calcWLsta, 5, 5, latdeg);
    } else {
        return false;
    }

    VERBOSEF("pressure:          %+24.10f", result.pressure);
    VERBOSEF("temperature:       %+24.10f", result.temperature);
    VERBOSEF("water_pressure:    %+24.10f", result.water_pressure);
    VERBOSEF("lapse_temperature: %+24.10f", result.lapse_temperature);
    VERBOSEF("water_vapor:       %+24.10f", result.water_vapor);
    return true;
}

static double hydrostatic_function(double latitude, double ellipsoidal_height, double pressure) {
    VSCOPE_FUNCTIONF("%+.8f, %+.8f, %+.8f", latitude * constant::RAD2DEG, ellipsoidal_height,
                     pressure);
    return 2.2768 /
           (1.0 - 0.00266 * cos(2.0 * latitude * constant::RAD2DEG) -
            (2.8e-7) * ellipsoidal_height) *
           pressure * 0.001;
}

static double wet_function(double temperature, double water_pressure) {
    VSCOPE_FUNCTIONF("%+.8f, %+.8f", temperature, water_pressure);
    return 2.2768 * (1255.0 / temperature + 0.05) * water_pressure * 0.001;
}

bool mops_tropospheric_delay(ts::Tai const& time, double latitude, double ellipsoidal_height,
                             double geoid_height, HydrostaticAndWetDelay& result) {
    VSCOPE_FUNCTIONF("%s, %+.8f, %+.8f, %+.8f", time.rtklib_time_string().c_str(),
                     latitude * constant::RAD2DEG, ellipsoidal_height, geoid_height);

    Mops mops{};
    if (!evaluate_mops(time, latitude, mops)) {
        VERBOSEF("failed to evaluate mops");
        return false;
    }

    auto elevation = ellipsoidal_height - geoid_height;

    auto t0 = mops.temperature;
    auto p0 = mops.pressure;
    auto e0 = mops.water_pressure;

    auto dt = mops.lapse_temperature;
    auto de = mops.water_vapor;

    double const g = 9.80665, Rd = 287.0537625; /* J/(kg.K) */

    auto t = t0 - dt * elevation;
    auto p = p0 * pow(1.0 - dt * elevation / t0, g / (dt * Rd));
    auto e = e0 * pow(1.0 - dt * elevation / t0, (de + 1.0) * g / (dt * Rd));

    VERBOSEF("temperature:    %+.8f", t);
    VERBOSEF("pressure:       %+.8f", p);
    VERBOSEF("water pressure: %+.8f", e);

    auto k0 = 273.15;
    auto tk = t;
    auto tc = tk - k0;

    auto et = 6.11 * pow(tk / k0, -5.3) * exp(25.2 * tc / tk);
    if (e > et) {
        VERBOSEF("water pressure: %+.8f (capped)", et);
        e = et;
    }

    result.hydrostatic = hydrostatic_function(latitude, ellipsoidal_height, p);
    result.wet         = wet_function(t, e);

    VERBOSEF("hydrostatic:    %+.8f", result.hydrostatic);
    VERBOSEF("wet:            %+.8f", result.wet);
    return true;
}

}  // namespace generator::tokoro
