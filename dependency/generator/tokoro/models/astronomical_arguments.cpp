#include "astronomical_arguments.hpp"
#include "constant.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, astarg);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, astarg)

namespace generator {
namespace tokoro {

static double const ASTRONOMICAL_ARGUMENTS_DATA[][5] = {
    /* coefficients for iau 1980 nutation */
    {134.96340251, 1717915923.2178, 31.8792, 0.051635, -0.00024470},
    {357.52910918, 129596581.0481, -0.5532, 0.000136, -0.00001149},
    {93.27209062, 1739527262.8478, -12.7512, -0.001037, 0.00000417},
    {297.85019547, 1602961601.2090, -6.3706, 0.006593, -0.00003169},
    {125.04455501, -6962890.2665, 7.4722, 0.007702, -0.00005939},
};

AstronomicalArguments AstronomicalArguments::evaluate(double t) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f", t);

    double time[4];
    time[0] = t;
    time[1] = time[0] * t;
    time[2] = time[1] * t;
    time[3] = time[2] * t;

    AstronomicalArguments args{};
    args.l     = ASTRONOMICAL_ARGUMENTS_DATA[0][0] * 3600.0;
    args.lp    = ASTRONOMICAL_ARGUMENTS_DATA[1][0] * 3600.0;
    args.f     = ASTRONOMICAL_ARGUMENTS_DATA[2][0] * 3600.0;
    args.d     = ASTRONOMICAL_ARGUMENTS_DATA[3][0] * 3600.0;
    args.omega = ASTRONOMICAL_ARGUMENTS_DATA[4][0] * 3600.0;

    for (int i = 0; i < 4; i++) {
        args.l += ASTRONOMICAL_ARGUMENTS_DATA[0][i + 1] * time[i];
        args.lp += ASTRONOMICAL_ARGUMENTS_DATA[1][i + 1] * time[i];
        args.f += ASTRONOMICAL_ARGUMENTS_DATA[2][i + 1] * time[i];
        args.d += ASTRONOMICAL_ARGUMENTS_DATA[3][i + 1] * time[i];
        args.omega += ASTRONOMICAL_ARGUMENTS_DATA[4][i + 1] * time[i];
    }

    args.l     = fmod(args.l * constant::ARCSEC2RAD, 2 * constant::PI);
    args.lp    = fmod(args.lp * constant::ARCSEC2RAD, 2 * constant::PI);
    args.f     = fmod(args.f * constant::ARCSEC2RAD, 2 * constant::PI);
    args.d     = fmod(args.d * constant::ARCSEC2RAD, 2 * constant::PI);
    args.omega = fmod(args.omega * constant::ARCSEC2RAD, 2 * constant::PI);

    VERBOSEF("l:     %+.14f", args.l);
    VERBOSEF("lp:    %+.14f", args.lp);
    VERBOSEF("f:     %+.14f", args.f);
    VERBOSEF("d:     %+.14f", args.d);
    VERBOSEF("omega: %+.14f", args.omega);
    return args;
}

}  // namespace tokoro
}  // namespace generator
