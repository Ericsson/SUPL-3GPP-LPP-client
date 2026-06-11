#pragma once
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>

#include <string>
#include <vector>

namespace format {
namespace rinex {

struct NavData {
    std::vector<ephemeris::GpsEphemeris> gps;
    std::vector<ephemeris::GalEphemeris> gal;
    std::vector<ephemeris::BdsEphemeris> bds;
};

/// Parse a RINEX 3/4 navigation file. Returns all decoded ephemerides.
NavData parse_nav_file(std::string const& path);

}  // namespace rinex
}  // namespace format
