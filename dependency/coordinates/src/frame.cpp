#include <coordinates/frame/cgcs2000.hpp>
#include <coordinates/frame/etrf.hpp>
#include <coordinates/frame/itrf.hpp>
#include <coordinates/frame/pz90.hpp>
#include <coordinates/frame/sweref99.hpp>
#include <coordinates/frame/wgs84.hpp>

namespace coordinates {

constexpr Ellipsoid FrameTrait<WGS84_G730>::ellipsoid;
constexpr Ellipsoid FrameTrait<WGS84_G873>::ellipsoid;
constexpr Ellipsoid FrameTrait<WGS84_G1150>::ellipsoid;
constexpr Ellipsoid FrameTrait<WGS84_G1674>::ellipsoid;
constexpr Ellipsoid FrameTrait<WGS84_G1762>::ellipsoid;
constexpr Ellipsoid FrameTrait<WGS84_G2139>::ellipsoid;
constexpr Ellipsoid FrameTrait<WGS84_G2296>::ellipsoid;

constexpr Ellipsoid FrameTrait<ETRF89>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF90>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF91>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF92>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF93>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF94>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF96>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF97>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF2000>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF2005>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF2014>::ellipsoid;
constexpr Ellipsoid FrameTrait<ETRF2020>::ellipsoid;

constexpr Ellipsoid FrameTrait<Sweref99>::ellipsoid;
constexpr Ellipsoid FrameTrait<CGCS2000>::ellipsoid;

constexpr Ellipsoid FrameTrait<PZ90>::ellipsoid;
constexpr Ellipsoid FrameTrait<PZ90_02>::ellipsoid;
constexpr Ellipsoid FrameTrait<PZ90_11>::ellipsoid;

constexpr Ellipsoid FrameTrait<ITRF88>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF89>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF90>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF91>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF92>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF93>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF94>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF96>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF97>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF2000>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF2005>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF2008>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF2014>::ellipsoid;
constexpr Ellipsoid FrameTrait<ITRF2020>::ellipsoid;

}  // namespace coordinates
