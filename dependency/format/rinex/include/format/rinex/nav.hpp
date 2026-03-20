#pragma once
#include <core/core.hpp>
#include <ephemeris/ephemeris.hpp>

#include <functional>
#include <string>

namespace format {
namespace rinex {

struct NavCallbacks {
    std::function<void(ephemeris::GpsEphemeris const&)> gps;
    std::function<void(ephemeris::GalEphemeris const&)> gal;
    std::function<void(ephemeris::BdsEphemeris const&)> bds;
};

bool parse_nav(std::string const& path, NavCallbacks const& callbacks) NOEXCEPT;

}  // namespace rinex
}  // namespace format
