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

FrameEpoch frame_epoch(FrameId id) {
    switch (id) {
    case FrameId::WGS84_G730:
        return {FrameTrait<WGS84_G730>::temporal_model, FrameTrait<WGS84_G730>::reference_epoch};
    case FrameId::WGS84_G873:
        return {FrameTrait<WGS84_G873>::temporal_model, FrameTrait<WGS84_G873>::reference_epoch};
    case FrameId::WGS84_G1150:
        return {FrameTrait<WGS84_G1150>::temporal_model, FrameTrait<WGS84_G1150>::reference_epoch};
    case FrameId::WGS84_G1674:
        return {FrameTrait<WGS84_G1674>::temporal_model, FrameTrait<WGS84_G1674>::reference_epoch};
    case FrameId::WGS84_G1762:
        return {FrameTrait<WGS84_G1762>::temporal_model, FrameTrait<WGS84_G1762>::reference_epoch};
    case FrameId::WGS84_G2139:
        return {FrameTrait<WGS84_G2139>::temporal_model, FrameTrait<WGS84_G2139>::reference_epoch};
    case FrameId::WGS84_G2296:
        return {FrameTrait<WGS84_G2296>::temporal_model, FrameTrait<WGS84_G2296>::reference_epoch};
    case FrameId::ETRF89:
        return {FrameTrait<ETRF89>::temporal_model, FrameTrait<ETRF89>::reference_epoch};
    case FrameId::ETRF90:
        return {FrameTrait<ETRF90>::temporal_model, FrameTrait<ETRF90>::reference_epoch};
    case FrameId::ETRF91:
        return {FrameTrait<ETRF91>::temporal_model, FrameTrait<ETRF91>::reference_epoch};
    case FrameId::ETRF92:
        return {FrameTrait<ETRF92>::temporal_model, FrameTrait<ETRF92>::reference_epoch};
    case FrameId::ETRF93:
        return {FrameTrait<ETRF93>::temporal_model, FrameTrait<ETRF93>::reference_epoch};
    case FrameId::ETRF94:
        return {FrameTrait<ETRF94>::temporal_model, FrameTrait<ETRF94>::reference_epoch};
    case FrameId::ETRF96:
        return {FrameTrait<ETRF96>::temporal_model, FrameTrait<ETRF96>::reference_epoch};
    case FrameId::ETRF97:
        return {FrameTrait<ETRF97>::temporal_model, FrameTrait<ETRF97>::reference_epoch};
    case FrameId::ETRF2000:
        return {FrameTrait<ETRF2000>::temporal_model, FrameTrait<ETRF2000>::reference_epoch};
    case FrameId::ETRF2005:
        return {FrameTrait<ETRF2005>::temporal_model, FrameTrait<ETRF2005>::reference_epoch};
    case FrameId::ETRF2014:
        return {FrameTrait<ETRF2014>::temporal_model, FrameTrait<ETRF2014>::reference_epoch};
    case FrameId::ETRF2020:
        return {FrameTrait<ETRF2020>::temporal_model, FrameTrait<ETRF2020>::reference_epoch};
    case FrameId::Sweref99:
        return {FrameTrait<Sweref99>::temporal_model, FrameTrait<Sweref99>::reference_epoch};
    case FrameId::CGCS2000:
        return {FrameTrait<CGCS2000>::temporal_model, FrameTrait<CGCS2000>::reference_epoch};
    case FrameId::PZ90:
        return {FrameTrait<PZ90>::temporal_model, FrameTrait<PZ90>::reference_epoch};
    case FrameId::PZ90_02:
        return {FrameTrait<PZ90_02>::temporal_model, FrameTrait<PZ90_02>::reference_epoch};
    case FrameId::PZ90_11:
        return {FrameTrait<PZ90_11>::temporal_model, FrameTrait<PZ90_11>::reference_epoch};
    case FrameId::ITRF88:
        return {FrameTrait<ITRF88>::temporal_model, FrameTrait<ITRF88>::reference_epoch};
    case FrameId::ITRF89:
        return {FrameTrait<ITRF89>::temporal_model, FrameTrait<ITRF89>::reference_epoch};
    case FrameId::ITRF90:
        return {FrameTrait<ITRF90>::temporal_model, FrameTrait<ITRF90>::reference_epoch};
    case FrameId::ITRF91:
        return {FrameTrait<ITRF91>::temporal_model, FrameTrait<ITRF91>::reference_epoch};
    case FrameId::ITRF92:
        return {FrameTrait<ITRF92>::temporal_model, FrameTrait<ITRF92>::reference_epoch};
    case FrameId::ITRF93:
        return {FrameTrait<ITRF93>::temporal_model, FrameTrait<ITRF93>::reference_epoch};
    case FrameId::ITRF94:
        return {FrameTrait<ITRF94>::temporal_model, FrameTrait<ITRF94>::reference_epoch};
    case FrameId::ITRF96:
        return {FrameTrait<ITRF96>::temporal_model, FrameTrait<ITRF96>::reference_epoch};
    case FrameId::ITRF97:
        return {FrameTrait<ITRF97>::temporal_model, FrameTrait<ITRF97>::reference_epoch};
    case FrameId::ITRF2000:
        return {FrameTrait<ITRF2000>::temporal_model, FrameTrait<ITRF2000>::reference_epoch};
    case FrameId::ITRF2005:
        return {FrameTrait<ITRF2005>::temporal_model, FrameTrait<ITRF2005>::reference_epoch};
    case FrameId::ITRF2008:
        return {FrameTrait<ITRF2008>::temporal_model, FrameTrait<ITRF2008>::reference_epoch};
    case FrameId::ITRF2014:
        return {FrameTrait<ITRF2014>::temporal_model, FrameTrait<ITRF2014>::reference_epoch};
    case FrameId::ITRF2020:
        return {FrameTrait<ITRF2020>::temporal_model, FrameTrait<ITRF2020>::reference_epoch};
    }
    return {FrameTemporalModel::TimeDependent, 0.0};
}

}  // namespace coordinates
