#pragma once
#include <maths/float3.hpp>
#include <generator/tokoro/reference_ellipsoid.hpp>

namespace generator {
namespace tokoro {

Float3 ecef_to_llh(Float3 ecef, ReferenceEllipsoid const& ellipsoid);
Float3 llh_to_ecef(Float3 llh, ReferenceEllipsoid const& ellipsoid);

enum Itrf {
    ITRF1988 = 0,
    ITRF1989 = 1,
    ITRF1990 = 2,
    ITRF1991 = 3,
    ITRF1992 = 4,
    ITRF1993 = 5,
    ITRF1994 = 6,
    ITRF1996 = 7,
    ITRF1997 = 8,
    ITRF2000 = 9,
    ITRF2005 = 10,
    ITRF2008 = 11,
    ITRF2014 = 12,
    ITRF2020 = 13,
};

Float3 itrf_transform(Itrf from, Itrf to, double epoch, Float3 position);

Float3 itrf89_to_etrf89(double epoch, Float3 position);
Float3 etrf89_to_itrf89(double epoch, Float3 position);




}
}  // namespace generator
