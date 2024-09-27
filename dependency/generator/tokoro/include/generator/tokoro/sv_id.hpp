#pragma once
#include <core/core.hpp>

#include <functional>

namespace generator {
namespace tokoro {

enum class GnssId : uint8_t {
    GPS = 0,
};

struct SvId {
    GnssId  gnss_id;
    uint8_t satellite_id;  // PRN

    bool operator==(SvId const& other) const NOEXCEPT {
        return gnss_id == other.gnss_id && satellite_id == other.satellite_id;
    }

    char const* name() const NOEXCEPT {
        switch (gnss_id) {
        case GnssId::GPS:
            switch (satellite_id) {
            case 1: return "GPS-01";
            case 2: return "GPS-02";
            case 3: return "GPS-03";
            case 4: return "GPS-04";
            case 5: return "GPS-05";
            case 6: return "GPS-06";
            case 7: return "GPS-07";
            case 8: return "GPS-08";
            case 9: return "GPS-09";
            case 10: return "GPS-10";
            case 11: return "GPS-11";
            case 12: return "GPS-12";
            case 13: return "GPS-13";
            case 14: return "GPS-14";
            case 15: return "GPS-15";
            case 16: return "GPS-16";
            case 17: return "GPS-17";
            case 18: return "GPS-18";
            case 19: return "GPS-19";
            case 20: return "GPS-20";
            case 21: return "GPS-21";
            case 22: return "GPS-22";
            case 23: return "GPS-23";
            case 24: return "GPS-24";
            case 25: return "GPS-25";
            case 26: return "GPS-26";
            case 27: return "GPS-27";
            case 28: return "GPS-28";
            case 29: return "GPS-29";
            case 30: return "GPS-30";
            case 31: return "GPS-31";
            case 32: return "GPS-32";
            default: return "GPS-??";
            }
            break;
        }
    }
};
}  // namespace tokoro
}  // namespace generator

namespace std {
template <>
struct hash<generator::tokoro::SvId> {
    std::size_t operator()(generator::tokoro::SvId const& sv_id) const NOEXCEPT {
        return static_cast<std::size_t>(sv_id.gnss_id) << 8 | sv_id.satellite_id;
    }
};
}  // namespace std
