#include "coordinate.hpp"
#include "constant.hpp"

#include <math.h>

#include <loglet/loglet.hpp>

#define LOGLET_CURRENT_MODULE "coord"

namespace generator {
namespace tokoro {

Float3 ecef_to_llh(Float3 ecef, ReferenceEllipsoid const& ellipsoid) {
    auto x = ecef.x;
    auto y = ecef.y;
    auto z = ecef.z;

    auto a  = ellipsoid.semi_major_axis;
    auto b  = ellipsoid.semi_minor_axis;
    auto e2 = ellipsoid.eccentricity_sq;

    auto lon = std::atan2(y, x);

    auto p   = std::sqrt(x * x + y * y);
    auto phi = std::atan2(z, p * (1.0 - e2));

    auto tolerance = 1e-12;
    for (;;) {
        auto sin_phi = std::sin(phi);
        auto N       = a / std::sqrt(1.0 - e2 * sin_phi * sin_phi);
        auto phi_n   = std::atan2(z + N * e2 * sin_phi, p);
        if (std::abs(phi_n - phi) < tolerance) {
            phi = phi_n;

            auto h = p / std::cos(phi) - N;
            return Float3{phi * constant::RAD2DEG, lon * constant::RAD2DEG, h};
        }

        phi = phi_n;
    }
}

Float3 llh_to_ecef(Float3 llh, ReferenceEllipsoid const& ellipsoid) {
    auto lat = llh.x * constant::DEG2RAD;
    auto lon = llh.y * constant::DEG2RAD;
    auto h   = llh.z;

    auto d = ellipsoid.eccentricity * std::sin(lat);
    auto N = ellipsoid.semi_major_axis / std::sqrt(1.0 - d * d);

    auto x = (N + h) * std::cos(lat) * std::cos(lon);
    auto y = (N + h) * std::cos(lat) * std::sin(lon);
    auto z = (N * (1.0 - ellipsoid.eccentricity_sq) + h) * std::sin(lat);
    return Float3{x, y, z};
}

struct ItrfParameter {
    double tx;
    double ty;
    double tz;
    double d;
    double rx;
    double ry;
    double rz;
    double epoch;
    double dtx;
    double dty;
    double dtz;
    double dd;
    double drx;
    double dry;
    double drz;

    ItrfParameter inverse() const {
        return ItrfParameter{-tx,  -ty,  -tz,  -d,  -rx,  -ry,  -rz, epoch,
                             -dtx, -dty, -dtz, -dd, -drx, -dry, -drz};
    }
};

static ItrfParameter ITRF_2020_to_2020_PARAMETER = {
    0.0, 0.0, 0.0, 0.0, 0.00, 0.00, 0.00, 2015.0, 0.0, 0.0, 0.0, 0.00, 0.00, 0.00, 0.00,
};
static ItrfParameter ITRF_2020_to_2014_PARAMETER = {
    -1.4, -0.9, 1.4, -0.42, 0.00, 0.00, 0.00, 2015.0, 0.0, -0.1, 0.2, 0.00, 0.00, 0.00, 0.00,
};
static ItrfParameter ITRF_2020_to_2008_PARAMETER = {
    0.2, 1.0, 3.3, -0.29, 0.00, 0.00, 0.00, 2015.0, 0.0, -0.1, 0.1, 0.03, 0.00, 0.00, 0.00,
};
static ItrfParameter ITRF_2020_to_2005_PARAMETER = {
    2.7, 0.1, -1.4, 0.65, 0.00, 0.00, 0.00, 2015.0, 0.3, -0.1, 0.1, 0.03, 0.00, 0.00, 0.00,
};
static ItrfParameter ITRF_2020_to_2000_PARAMETER = {
    -0.2, 0.8, -34.2, 2.25, 0.00, 0.00, 0.00, 2015.0, 0.1, 0.0, -1.7, 0.11, 0.00, 0.00, 0.00,
};
static ItrfParameter ITRF_2020_to_97_PARAMETER = {
    6.5, -3.9, -77.9, 3.98, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_96_PARAMETER = {
    6.5, -3.9, -77.9, 3.98, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_94_PARAMETER = {
    6.5, -3.9, -77.9, 3.98, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_93_PARAMETER = {
    -65.8, 1.9, -71.3, 4.47, -3.36, -4.33, 0.75, 2015.0, 2.8, -0.2, -2.3, 0.12, -0.11, -0.19, 0.07,
};
static ItrfParameter ITRF_2020_to_92_PARAMETER = {
    14.5, -1.9, -85.9, 3.27, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_91_PARAMETER = {
    26.5, 12.1, -91.9, 4.67, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_90_PARAMETER = {
    24.5, 8.1, -107.9, 4.97, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_89_PARAMETER = {
    29.5, 32.1, -145.9, 8.37, 0.00, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};
static ItrfParameter ITRF_2020_to_88_PARAMETER = {
    24.5, -3.9, -169.9, 11.47, 0.10, 0.00, 0.36, 2015.0, 0.1, -0.6, -3.1, 0.12, 0.00, 0.00, 0.02,
};

static Float3 itrf_transform_parameter(ItrfParameter const& parameter, double epoch,
                                       Float3 position) {
    VSCOPE_FUNCTIONF("%f", epoch);

    auto x = position.x * 1000.0;
    auto y = position.y * 1000.0;
    auto z = position.z * 1000.0;
    auto t = epoch - parameter.epoch;
    VERBOSEF("x=%g, y=%g, z=%g, t=%f", x, y, z, t);

    auto tx = parameter.tx + parameter.dtx * t;
    auto ty = parameter.ty + parameter.dty * t;
    auto tz = parameter.tz + parameter.dtz * t;
    auto d  = parameter.d + parameter.dd * t;
    auto rx = parameter.rx + parameter.drx * t;
    auto ry = parameter.ry + parameter.dry * t;
    auto rz = parameter.rz + parameter.drz * t;
    VERBOSEF("tx=%gmm, ty=%gmm, tz=%gmm", tx, ty, tz, d, rx, ry, rz);

    // d is in ppb
    d *= 1e-9;
    VERBOSEF("d=%g", d);

    // rx, ry, rz are in 0.001 arcsec
    rx *= constant::ARCSEC2RAD * 0.001;
    ry *= constant::ARCSEC2RAD * 0.001;
    rz *= constant::ARCSEC2RAD * 0.001;
    VERBOSEF("rx=%gr, ry=%gr, rz=%gr", rx, ry, rz);

    auto x1 = x + tx + d * x - rz * y + ry * z;
    auto y1 = y + ty + rz * x + d * y - rx * z;
    auto z1 = z + tz - ry * x + rx * y + d * z;
    VERBOSEF("x1=%g, y1=%g, z1=%g", x1, y1, z1);

    return Float3{x1 / 1000.0, y1 / 1000.0, z1 / 1000.0};
}

static ItrfParameter const& itrf2020_to_parameter(Itrf to) {
    switch (to) {
    case Itrf::ITRF2020: return ITRF_2020_to_2020_PARAMETER;
    case Itrf::ITRF2014: return ITRF_2020_to_2014_PARAMETER;
    case Itrf::ITRF2008: return ITRF_2020_to_2008_PARAMETER;
    case Itrf::ITRF2005: return ITRF_2020_to_2005_PARAMETER;
    case Itrf::ITRF2000: return ITRF_2020_to_2000_PARAMETER;
    case Itrf::ITRF1997: return ITRF_2020_to_97_PARAMETER;
    case Itrf::ITRF1996: return ITRF_2020_to_96_PARAMETER;
    case Itrf::ITRF1994: return ITRF_2020_to_94_PARAMETER;
    case Itrf::ITRF1993: return ITRF_2020_to_93_PARAMETER;
    case Itrf::ITRF1992: return ITRF_2020_to_92_PARAMETER;
    case Itrf::ITRF1991: return ITRF_2020_to_91_PARAMETER;
    case Itrf::ITRF1990: return ITRF_2020_to_90_PARAMETER;
    case Itrf::ITRF1989: return ITRF_2020_to_89_PARAMETER;
    case Itrf::ITRF1988: return ITRF_2020_to_88_PARAMETER;
    default: CORE_UNREACHABLE(); return ITRF_2020_to_2020_PARAMETER;
    }
}

static Float3 itrf2020_to(Itrf to, double epoch, Float3 position) {
    return itrf_transform_parameter(itrf2020_to_parameter(to), epoch, position);
}

static Float3 itrf2020_from(Itrf from, double epoch, Float3 position) {
    return itrf_transform_parameter(itrf2020_to_parameter(from).inverse(), epoch, position);
}

Float3 itrf_transform(Itrf from, Itrf to, double epoch, Float3 position) {
    switch (from) {
    case Itrf::ITRF2020: return itrf2020_to(to, epoch, position);
    case Itrf::ITRF2014:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF2014, epoch, position));
    case Itrf::ITRF2008:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF2008, epoch, position));
    case Itrf::ITRF2005:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF2005, epoch, position));
    case Itrf::ITRF2000:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF2000, epoch, position));
    case Itrf::ITRF1997:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1997, epoch, position));
    case Itrf::ITRF1996:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1996, epoch, position));
    case Itrf::ITRF1994:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1994, epoch, position));
    case Itrf::ITRF1993:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1993, epoch, position));
    case Itrf::ITRF1992:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1992, epoch, position));
    case Itrf::ITRF1991:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1991, epoch, position));
    case Itrf::ITRF1990:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1990, epoch, position));
    case Itrf::ITRF1989:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1989, epoch, position));
    case Itrf::ITRF1988:
        return itrf2020_to(to, epoch, itrf2020_from(Itrf::ITRF1988, epoch, position));
    default: CORE_UNREACHABLE(); return position;
    }
}

static ItrfParameter ITRF89_TO_ETRF89_PARAMETER = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1989.0, 0.0, 0.0, 0.0, 0.0, 0.11, 0.57, -0.71
};

Float3 itrf89_to_etrf89(double epoch, Float3 position) {
    return itrf_transform_parameter(ITRF89_TO_ETRF89_PARAMETER, epoch, position);
}

Float3 etrf89_to_itrf89(double epoch, Float3 position) {
    return itrf_transform_parameter(ITRF89_TO_ETRF89_PARAMETER.inverse(), epoch, position);
}

}  // namespace tokoro
}  // namespace generator
