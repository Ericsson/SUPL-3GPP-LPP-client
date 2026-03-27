#pragma once
#include <core/core.hpp>
#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/glo.hpp>
#include <ephemeris/gps.hpp>

#include <functional>
#include <string>

namespace format {
namespace rinex {

struct NavCallbacks {
    std::function<void(ephemeris::GpsEphemeris const&)> gps;
    std::function<void(ephemeris::GalEphemeris const&)> gal;
    std::function<void(ephemeris::BdsEphemeris const&)> bds;
    std::function<void(ephemeris::GloEphemeris const&)> glo;
    std::function<void(double[4], double[4])>           klobuchar;      // GPS alpha[4], beta[4]
    std::function<void(double[4], double[4])>           bds_klobuchar;  // BDS alpha[4], beta[4]
};

bool parse_nav(std::string const& path, NavCallbacks const& callbacks) NOEXCEPT;

}  // namespace rinex
}  // namespace format
