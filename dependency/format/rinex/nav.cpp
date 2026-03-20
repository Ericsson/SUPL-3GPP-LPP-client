#include "nav.hpp"

#include <loglet/loglet.hpp>
#include <time/bdt.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>
#include <time/utc.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

LOGLET_MODULE2(format, rinex_nav);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, rinex_nav)

namespace format {
namespace rinex {

static double parse_d(std::string const& s) {
    // RINEX uses 'D' as exponent separator instead of 'E'
    std::string t = s;
    for (auto& c : t)
        if (c == 'D' || c == 'd') c = 'E';
    return std::stod(t);
}

static double field(std::string const& line, size_t col, size_t width = 19) {
    if (col + width > line.size()) return 0.0;
    auto s = line.substr(col, width);
    // trim
    size_t start = s.find_first_not_of(' ');
    if (start == std::string::npos) return 0.0;
    return parse_d(s.substr(start));
}

static bool parse_gps(std::ifstream& f, std::string const& line0, ephemeris::GpsEphemeris& eph) {
    // line0: "Gxx YYYY MM DD HH MM SS.SSSSSSSSS  af0  af1  af2"
    int    prn, year, month, day, hour, min;
    double sec;
    if (std::sscanf(line0.c_str(), "G%2d %4d %2d %2d %2d %2d %lf", &prn, &year, &month, &day, &hour,
                    &min, &sec) < 7)
        return false;

    double af0 = field(line0, 23);
    double af1 = field(line0, 42);
    double af2 = field(line0, 61);

    std::string l[7];
    for (int i = 0; i < 7; ++i) {
        if (!std::getline(f, l[i])) return false;
    }

    eph     = {};
    eph.prn = static_cast<uint8_t>(prn);
    eph.af0 = af0;
    eph.af1 = af1;
    eph.af2 = af2;

    auto toc_gps = ts::Gps::from_ymdhms(year, month, day, hour, min, sec);
    eph.toc      = toc_gps.time_of_week().as_double();

    // Broadcast orbit 1
    eph.iode    = static_cast<uint8_t>(field(l[0], 4));
    eph.crs     = field(l[0], 23);
    eph.delta_n = field(l[0], 42);
    eph.m0      = field(l[0], 61);

    // Broadcast orbit 2
    eph.cuc      = field(l[1], 4);
    eph.e        = field(l[1], 23);
    eph.cus      = field(l[1], 42);
    double sqrtA = field(l[1], 61);
    eph.a        = sqrtA * sqrtA;

    // Broadcast orbit 3
    eph.toe    = field(l[2], 4);
    eph.cic    = field(l[2], 23);
    eph.omega0 = field(l[2], 42);
    eph.cis    = field(l[2], 61);

    // Broadcast orbit 4
    eph.i0        = field(l[3], 4);
    eph.crc       = field(l[3], 23);
    eph.omega     = field(l[3], 42);
    eph.omega_dot = field(l[3], 61);

    // Broadcast orbit 5
    eph.idot           = field(l[4], 4);
    eph.ca_or_p_on_l2  = static_cast<uint8_t>(field(l[4], 23));
    eph.week_number    = static_cast<uint16_t>(field(l[4], 42));
    eph.l2_p_data_flag = static_cast<bool>(field(l[4], 61));

    // Broadcast orbit 6
    eph.ura_index = static_cast<uint8_t>(field(l[5], 4));
    eph.sv_health = static_cast<uint8_t>(field(l[5], 23));
    eph.tgd       = field(l[5], 42);
    eph.iodc      = static_cast<uint16_t>(field(l[5], 61));

    // Broadcast orbit 7
    eph.fit_interval_flag = static_cast<bool>(field(l[6], 23));

    // lpp_iod: use IODC lower 8 bits (matches LPP convention)
    eph.lpp_iod = eph.iodc & 0xFF;

    return true;
}

static bool parse_gal(std::ifstream& f, std::string const& line0, ephemeris::GalEphemeris& eph) {
    int    prn, year, month, day, hour, min;
    double sec;
    if (std::sscanf(line0.c_str(), "E%2d %4d %2d %2d %2d %2d %lf", &prn, &year, &month, &day, &hour,
                    &min, &sec) < 7)
        return false;

    double af0 = field(line0, 23);
    double af1 = field(line0, 42);
    double af2 = field(line0, 61);

    std::string l[7];
    for (int i = 0; i < 7; ++i) {
        if (!std::getline(f, l[i])) return false;
    }

    eph     = {};
    eph.prn = static_cast<uint8_t>(prn);
    eph.af0 = af0;
    eph.af1 = af1;
    eph.af2 = af2;

    auto toc_gst = ts::Gst{ts::Utc::from_date_time(year, month, day, hour, min, sec)};
    eph.toc      = toc_gst.time_of_week().as_double();

    // Broadcast orbit 1
    eph.iod_nav = static_cast<uint16_t>(field(l[0], 4));
    eph.crs     = field(l[0], 23);
    eph.delta_n = field(l[0], 42);
    eph.m0      = field(l[0], 61);

    // Broadcast orbit 2
    eph.cuc      = field(l[1], 4);
    eph.e        = field(l[1], 23);
    eph.cus      = field(l[1], 42);
    double sqrtA = field(l[1], 61);
    eph.a        = sqrtA * sqrtA;

    // Broadcast orbit 3
    eph.toe    = field(l[2], 4);
    eph.cic    = field(l[2], 23);
    eph.omega0 = field(l[2], 42);
    eph.cis    = field(l[2], 61);

    // Broadcast orbit 4
    eph.i0        = field(l[3], 4);
    eph.crc       = field(l[3], 23);
    eph.omega     = field(l[3], 42);
    eph.omega_dot = field(l[3], 61);

    // Broadcast orbit 5
    eph.idot        = field(l[4], 4);
    eph.week_number = static_cast<uint16_t>(field(l[4], 42));

    // lpp_iod: use iod_nav
    eph.lpp_iod = eph.iod_nav;

    return true;
}

static bool parse_bds(std::ifstream& f, std::string const& line0, ephemeris::BdsEphemeris& eph) {
    int    prn, year, month, day, hour, min;
    double sec;
    if (std::sscanf(line0.c_str(), "C%2d %4d %2d %2d %2d %2d %lf", &prn, &year, &month, &day, &hour,
                    &min, &sec) < 7)
        return false;

    double af0 = field(line0, 23);
    double af1 = field(line0, 42);
    double af2 = field(line0, 61);

    std::string l[7];
    for (int i = 0; i < 7; ++i) {
        if (!std::getline(f, l[i])) return false;
    }

    eph     = {};
    eph.prn = static_cast<uint8_t>(prn);
    eph.af0 = af0;
    eph.af1 = af1;
    eph.af2 = af2;

    auto toc_bdt = ts::Bdt{ts::Utc::from_date_time(year, month, day, hour, min, sec)};
    eph.toc      = toc_bdt.time_of_week().as_double();
    eph.toc_time = toc_bdt;

    // Broadcast orbit 1
    eph.aode    = static_cast<uint8_t>(field(l[0], 4));
    eph.crs     = field(l[0], 23);
    eph.delta_n = field(l[0], 42);
    eph.m0      = field(l[0], 61);

    // Broadcast orbit 2
    eph.cuc      = field(l[1], 4);
    eph.e        = field(l[1], 23);
    eph.cus      = field(l[1], 42);
    double sqrtA = field(l[1], 61);
    eph.a        = sqrtA * sqrtA;

    // Broadcast orbit 3
    eph.toe = field(l[2], 4);
    eph.toe_time =
        ts::Bdt{ts::Utc::from_date_time(year, month, day, hour, min, sec) + eph.toe - eph.toc};
    eph.cic    = field(l[2], 23);
    eph.omega0 = field(l[2], 42);
    eph.cis    = field(l[2], 61);

    // Broadcast orbit 4
    eph.i0        = field(l[3], 4);
    eph.crc       = field(l[3], 23);
    eph.omega     = field(l[3], 42);
    eph.omega_dot = field(l[3], 61);

    // Broadcast orbit 5
    eph.idot        = field(l[4], 4);
    eph.week_number = static_cast<uint16_t>(field(l[4], 42));

    // Broadcast orbit 6
    eph.sv_health = static_cast<uint8_t>(field(l[5], 23));
    eph.iode      = static_cast<uint8_t>(eph.aode);
    eph.iodc      = static_cast<uint8_t>(field(l[5], 61));

    eph.lpp_iod = eph.iode;

    return true;
}

bool parse_nav(std::string const& path, NavCallbacks const& callbacks) NOEXCEPT {
    std::ifstream f(path);
    if (!f.is_open()) {
        ERRORF("failed to open nav file: %s", path.c_str());
        return false;
    }

    // Skip header
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("END OF HEADER") != std::string::npos) break;
    }

    long count = 0;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        char sys = line[0];

        if (sys == 'G' && callbacks.gps) {
            ephemeris::GpsEphemeris eph{};
            if (parse_gps(f, line, eph)) {
                callbacks.gps(eph);
                count++;
            }
        } else if (sys == 'E' && callbacks.gal) {
            ephemeris::GalEphemeris eph{};
            if (parse_gal(f, line, eph)) {
                callbacks.gal(eph);
                count++;
            }
        } else if (sys == 'C' && callbacks.bds) {
            ephemeris::BdsEphemeris eph{};
            if (parse_bds(f, line, eph)) {
                callbacks.bds(eph);
                count++;
            }
        }
    }

    INFOF("parsed %ld nav records from %s", count, path.c_str());
    return true;
}

}  // namespace rinex
}  // namespace format
