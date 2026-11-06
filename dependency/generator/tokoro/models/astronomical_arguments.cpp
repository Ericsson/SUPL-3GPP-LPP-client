#include "astronomical_arguments.hpp"
#include "constant.hpp"

#include <cmath>
#include <loglet/loglet.hpp>

LOGLET_MODULE2(tokoro, astarg);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(tokoro, astarg)

namespace generator {
namespace tokoro {

static double const gAstronomicalArgumentsDataTable[][5] = {
    /* fundamental arguments (IERS Conventions 2003, eq. 40): l, l', F, D, Omega; coefficients
       of t^0 [deg], t^1..t^4 [arcsec/century^n] */
    {134.96340251, 1717915923.2178, 31.8792, 0.051635, -0.00024470},
    {357.52910918, 129596581.0481, -0.5532, 0.000136, -0.00001149},
    {93.27209062, 1739527262.8478, -12.7512, -0.001037, 0.00000417},
    {297.85019547, 1602961601.2090, -6.3706, 0.006593, -0.00003169},
    {125.04455501, -6962890.5431, 7.4722, 0.007702, -0.00005939},
};

AstronomicalArguments AstronomicalArguments::evaluate(double t) NOEXCEPT {
    VSCOPE_FUNCTIONF("%f", t);

    double time[4];
    time[0] = t;
    time[1] = time[0] * t;
    time[2] = time[1] * t;
    time[3] = time[2] * t;

    AstronomicalArguments args{};
    args.l     = gAstronomicalArgumentsDataTable[0][0] * 3600.0;
    args.lp    = gAstronomicalArgumentsDataTable[1][0] * 3600.0;
    args.f     = gAstronomicalArgumentsDataTable[2][0] * 3600.0;
    args.d     = gAstronomicalArgumentsDataTable[3][0] * 3600.0;
    args.omega = gAstronomicalArgumentsDataTable[4][0] * 3600.0;

    for (int i = 0; i < 4; i++) {
        args.l += gAstronomicalArgumentsDataTable[0][i + 1] * time[i];
        args.lp += gAstronomicalArgumentsDataTable[1][i + 1] * time[i];
        args.f += gAstronomicalArgumentsDataTable[2][i + 1] * time[i];
        args.d += gAstronomicalArgumentsDataTable[3][i + 1] * time[i];
        args.omega += gAstronomicalArgumentsDataTable[4][i + 1] * time[i];
    }

    // normalize to [0, 2*pi)
    auto normalize = [](double arcsec) {
        auto angle = fmod(arcsec * constant::ARCSEC2RAD, 2 * constant::PI);
        if (angle < 0.0) angle += 2 * constant::PI;
        return angle;
    };
    args.l     = normalize(args.l);
    args.lp    = normalize(args.lp);
    args.f     = normalize(args.f);
    args.d     = normalize(args.d);
    args.omega = normalize(args.omega);

    VERBOSEF("l:     %+.14f", args.l);
    VERBOSEF("lp:    %+.14f", args.lp);
    VERBOSEF("f:     %+.14f", args.f);
    VERBOSEF("d:     %+.14f", args.d);
    VERBOSEF("omega: %+.14f", args.omega);
    return args;
}

}  // namespace tokoro
}  // namespace generator
