#include "reference_ellipsoid.hpp"

namespace generator {
namespace tokoro {
namespace ellipsoid {
ReferenceEllipsoid const WGS84   = ReferenceEllipsoid::create(6378137.0, 1.0 / 298.257223563);
ReferenceEllipsoid const GRS80   = ReferenceEllipsoid::create(6378137.0, 1.0 / 298.257222101);
ReferenceEllipsoid const PZ90    = ReferenceEllipsoid::create(6378136.0, 1.0 / 298.25);
ReferenceEllipsoid const PZ90_11 = ReferenceEllipsoid::create(6378136.0, 1.0 / 298.257839303);
}  // namespace ellipsoid
}  // namespace tokoro
}  // namespace generator
