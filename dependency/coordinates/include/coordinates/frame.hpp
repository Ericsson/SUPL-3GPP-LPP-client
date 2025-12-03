#pragma once
#include <vector>

namespace coordinates {

enum class FrameId {
    WGS84_G730,
    WGS84_G873,
    WGS84_G1150,
    WGS84_G1674,
    WGS84_G1762,
    WGS84_G2139,
    WGS84_G2296,

    PZ90,
    PZ90_02,
    PZ90_11,

    CGCS2000,

    ITRF88,
    ITRF89,
    ITRF90,
    ITRF91,
    ITRF92,
    ITRF93,
    ITRF94,
    ITRF96,
    ITRF97,
    ITRF2000,
    ITRF2005,
    ITRF2008,
    ITRF2014,
    ITRF2020,

    ETRF89,
    ETRF90,
    ETRF91,
    ETRF92,
    ETRF93,
    ETRF94,
    ETRF96,
    ETRF97,
    ETRF2000,
    ETRF2005,
    ETRF2014,
    ETRF2020,

    Sweref99,
};

enum class FrameTemporalModel { TimeDependent, FixedEpoch, Atemporal };

template <typename Frame>
struct FrameTrait;

char const* frame_name(FrameId id);
FrameId     frame_from_gps_week(int week);

std::vector<FrameId> all_frames();
bool                 frame_from_name(char const* name, FrameId& out);

}  // namespace coordinates
