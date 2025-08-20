#include "sun_moon.hpp"
#include "constant.hpp"
#include "coordinates/eci.hpp"
#include "models/astronomical_arguments.hpp"

#include <loglet/loglet.hpp>
#include <time/utc.hpp>

LOGLET_MODULE2(tokoro, sunmoon);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, sunmoon)

namespace generator {
namespace tokoro {

static void compute_sun_and_moon_position_eci(ts::Tai const& time, Float3& sun,
                                              Float3& moon) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    // TODO(ewasjon): Need UT1-UTC correction term, usually given by Earth Orientation Parameters
    // (EOP). However, UTC is by definition maintained to be within +-0.9s of UT1, so it probably
    // doesn't matter for our purposes.
    auto ut1_utc = 0.0;
    auto t_jc    = ts::Utc{time}.j2000_century(ut1_utc);

    // Get astronomical arguments
    auto args = AstronomicalArguments::evaluate(t_jc);

    // Obliquity of the ecliptic
    auto epsilon = 23.439291 - 0.0130042 * t_jc;
    auto sine    = sin(epsilon * constant::DEG2RAD);
    auto cose    = cos(epsilon * constant::DEG2RAD);
    VERBOSEF("epsilon: %f", epsilon);
    VERBOSEF("sine: %f", sine);
    VERBOSEF("cose: %f", cose);

    {
        // Sun
        auto ms = 357.5277233 + 35999.05034 * t_jc;
        auto ls = 280.460 + 36000.770 * t_jc + 1.914666471 * sin(ms * constant::DEG2RAD) +
                  0.019994643 * sin(2.0 * ms * constant::DEG2RAD);
        auto rs   = constant::AU * (1.000140612 - 0.016708617 * cos(ms * constant::DEG2RAD) -
                                  0.000139589 * cos(2.0 * ms * constant::DEG2RAD));
        auto sinl = sin(ls * constant::DEG2RAD);
        auto cosl = cos(ls * constant::DEG2RAD);

        sun.x = rs * cosl;
        sun.y = rs * cose * sinl;
        sun.z = rs * sine * sinl;
        VERBOSEF("sun: (%f, %f, %f)", sun.x, sun.y, sun.z);
    }

    {
        // Moon
        auto lm = 218.32 + 481267.883 * t_jc + 6.29 * sin(args.l) -
                  1.27 * sin(args.l - 2.0 * args.d) + 0.66 * sin(2.0 * args.d) +
                  0.21 * sin(2.0 * args.l) - 0.19 * sin(args.lp) - 0.11 * sin(2.0 * args.f);
        auto pm = 5.13 * sin(args.f) + 0.28 * sin(args.l + args.f) - 0.28 * sin(args.f - args.l) -
                  0.17 * sin(args.f - 2.0 * args.d);
        auto rm = constant::RE_WGS84 /
                  sin((0.9508 + 0.0518 * cos(args.l) + 0.0095 * cos(args.l - 2.0 * args.d) +
                       0.0078 * cos(2.0 * args.d) + 0.0028 * cos(2.0 * args.l)) *
                      constant::DEG2RAD);

        auto sinl = sin(lm * constant::DEG2RAD);
        auto cosl = cos(lm * constant::DEG2RAD);
        auto sinp = sin(pm * constant::DEG2RAD);
        auto cosp = cos(pm * constant::DEG2RAD);

        moon.x = rm * cosp * cosl;
        moon.y = rm * (cose * cosp * sinl - sine * sinp);
        moon.z = rm * (sine * cosp * sinl + cose * sinp);
        VERBOSEF("moon: (%f, %f, %f)", moon.x, moon.y, moon.z);
    }
}

SunMoonPosition sun_and_moon_position_ecef(ts::Tai const& time) NOEXCEPT {
    VSCOPE_FUNCTIONF("%s", time.rtklib_time_string().c_str());

    Float3 sun_eci{};
    Float3 moon_eci{};
    compute_sun_and_moon_position_eci(time, sun_eci, moon_eci);

    EciEarthParameters earth_params{};
    Mat3               transform{};
    SunMoonPosition    result{};
    eci_to_ecef_matrix(time, earth_params, &transform, &result.gmst);

    result.sun  = transform * sun_eci;
    result.moon = transform * moon_eci;
    return result;
}

}  // namespace tokoro
}  // namespace generator
