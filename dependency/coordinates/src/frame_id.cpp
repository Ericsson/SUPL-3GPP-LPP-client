#include <algorithm>
#include <cctype>
#include <vector>
#include "coordinates/frame.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE(coord);
LOGLET_MODULE2(coord, frame);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(coord, frame)

namespace coordinates {

char const* frame_name(FrameId id) {
    switch (id) {
    case FrameId::WGS84_G730: return "wgs84(g730)";
    case FrameId::WGS84_G873: return "wgs84(g873)";
    case FrameId::WGS84_G1150: return "wgs84(g1150)";
    case FrameId::WGS84_G1674: return "wgs84(g1674)";
    case FrameId::WGS84_G1762: return "wgs84(g1762)";
    case FrameId::WGS84_G2139: return "wgs84(g2139)";
    case FrameId::WGS84_G2296: return "wgs84(g2296)";
    case FrameId::PZ90: return "pz-90";
    case FrameId::PZ90_02: return "pz-90.02";
    case FrameId::PZ90_11: return "pz-90.11";
    case FrameId::CGCS2000: return "cgcs2000";
    case FrameId::ITRF88: return "itrf88";
    case FrameId::ITRF89: return "itrf89";
    case FrameId::ITRF90: return "itrf90";
    case FrameId::ITRF91: return "itrf91";
    case FrameId::ITRF92: return "itrf92";
    case FrameId::ITRF93: return "itrf93";
    case FrameId::ITRF94: return "itrf94";
    case FrameId::ITRF96: return "itrf96";
    case FrameId::ITRF97: return "itrf97";
    case FrameId::ITRF2000: return "itrf2000";
    case FrameId::ITRF2005: return "itrf2005";
    case FrameId::ITRF2008: return "itrf2008";
    case FrameId::ITRF2014: return "itrf2014";
    case FrameId::ITRF2020: return "itrf2020";
    case FrameId::ETRF89: return "etrf89";
    case FrameId::ETRF90: return "etrf90";
    case FrameId::ETRF91: return "etrf91";
    case FrameId::ETRF92: return "etrf92";
    case FrameId::ETRF93: return "etrf93";
    case FrameId::ETRF94: return "etrf94";
    case FrameId::ETRF96: return "etrf96";
    case FrameId::ETRF97: return "etrf97";
    case FrameId::ETRF2000: return "etrf2000";
    case FrameId::ETRF2005: return "etrf2005";
    case FrameId::ETRF2014: return "etrf2014";
    case FrameId::ETRF2020: return "etrf2020";
    case FrameId::Sweref99: return "sweref99";
    }
    return "unknown";
}

FrameId frame_from_gps_week(int week) {
    VSCOPE_FUNCTIONF("week=%d", week);

    if (week < 873) return FrameId::WGS84_G730;
    if (week < 1150) return FrameId::WGS84_G873;
    if (week < 1674) return FrameId::WGS84_G1150;
    if (week < 1762) return FrameId::WGS84_G1674;
    if (week < 2139) return FrameId::WGS84_G1762;
    return FrameId::WGS84_G2139;
}

std::vector<FrameId> all_frames() {
    return {
        FrameId::WGS84_G730,  FrameId::WGS84_G873,  FrameId::WGS84_G1150, FrameId::WGS84_G1674,
        FrameId::WGS84_G1762, FrameId::WGS84_G2139, FrameId::WGS84_G2296, FrameId::PZ90,
        FrameId::PZ90_02,     FrameId::PZ90_11,     FrameId::CGCS2000,    FrameId::ITRF88,
        FrameId::ITRF89,      FrameId::ITRF90,      FrameId::ITRF91,      FrameId::ITRF92,
        FrameId::ITRF93,      FrameId::ITRF94,      FrameId::ITRF96,      FrameId::ITRF97,
        FrameId::ITRF2000,    FrameId::ITRF2005,    FrameId::ITRF2008,    FrameId::ITRF2014,
        FrameId::ITRF2020,    FrameId::ETRF89,      FrameId::ETRF90,      FrameId::ETRF91,
        FrameId::ETRF92,      FrameId::ETRF93,      FrameId::ETRF94,      FrameId::ETRF96,
        FrameId::ETRF97,      FrameId::ETRF2000,    FrameId::ETRF2005,    FrameId::ETRF2014,
        FrameId::ETRF2020,    FrameId::Sweref99,
    };
}

static bool str_equal_case_insensitive(char const* a, char const* b) {
    while (*a && *b) {
        if (std::tolower(static_cast<unsigned char>(*a)) !=
            std::tolower(static_cast<unsigned char>(*b))) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}

bool frame_from_name(char const* name, FrameId& out) {
    VSCOPE_FUNCTIONF("name=%s", name);

    if (str_equal_case_insensitive(name, "wgs84")) {
        out = FrameId::WGS84_G2296;
        return true;
    }
    if (str_equal_case_insensitive(name, "itrf")) {
        out = FrameId::ITRF2020;
        return true;
    }
    if (str_equal_case_insensitive(name, "itrf-current")) {
        out = FrameId::ITRF2020;
        return true;
    }

    auto frames = all_frames();
    for (auto frame : frames) {
        if (str_equal_case_insensitive(name, frame_name(frame))) {
            out = frame;
            return true;
        }
    }
    VERBOSEF("unknown frame name");
    return false;
}

}  // namespace coordinates
