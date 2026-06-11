#include "nav_reader.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

#include <loglet/loglet.hpp>
#include <time/bdt.hpp>
#include <time/gps.hpp>
#include <time/gst.hpp>

LOGLET_MODULE(rinex_nav);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(rinex_nav)

namespace format {
namespace rinex {

// RINEX uses Fortran-style 'D' exponent notation: replace 'D'/'d' with 'E'
static double parse_double(char const* str, int width) {
    char buf[32];
    int  n = width < 31 ? width : 31;
    memcpy(buf, str, static_cast<size_t>(n));
    buf[n] = '\0';
    for (int i = 0; i < n; i++) {
        if (buf[i] == 'D' || buf[i] == 'd') buf[i] = 'E';
    }
    return atof(buf);
}

static bool parse_gps_record(char lines[8][128], ephemeris::GpsEphemeris& eph) {
    int prn, year, month, day, hour, min, sec;
    if (sscanf(lines[0] + 1, "%d %d %d %d %d %d %d", &prn, &year, &month, &day, &hour, &min,
               &sec) != 7)
        return false;

    auto gps_time = ts::Gps::from_ymdhms(year, month, day, hour, min, static_cast<double>(sec));

    eph             = {};
    eph.prn         = static_cast<uint8_t>(prn);
    eph.week_number = static_cast<uint16_t>(gps_time.week());
    eph.toc         = gps_time.time_of_week().full_seconds();
    eph.af0         = parse_double(lines[0] + 23, 19);
    eph.af1         = parse_double(lines[0] + 42, 19);
    eph.af2         = parse_double(lines[0] + 61, 19);

    // Line 1: IODE, Crs, Delta_n, M0
    double iode_d = parse_double(lines[1] + 4, 19);
    eph.iode      = static_cast<uint8_t>(static_cast<int>(iode_d)) & 0xFF;
    eph.crs       = parse_double(lines[1] + 23, 19);
    eph.delta_n   = parse_double(lines[1] + 42, 19);
    eph.m0        = parse_double(lines[1] + 61, 19);

    // Line 2: Cuc, e, Cus, sqrt(A)
    eph.cuc       = parse_double(lines[2] + 4, 19);
    eph.e         = parse_double(lines[2] + 23, 19);
    eph.cus       = parse_double(lines[2] + 42, 19);
    double sqrt_a = parse_double(lines[2] + 61, 19);
    eph.a         = sqrt_a * sqrt_a;

    // Line 3: Toe, Cic, OMEGA0, Cis
    eph.toe    = parse_double(lines[3] + 4, 19);
    eph.cic    = parse_double(lines[3] + 23, 19);
    eph.omega0 = parse_double(lines[3] + 42, 19);
    eph.cis    = parse_double(lines[3] + 61, 19);

    // Line 4: i0, Crc, omega, OMEGA_DOT
    eph.i0        = parse_double(lines[4] + 4, 19);
    eph.crc       = parse_double(lines[4] + 23, 19);
    eph.omega     = parse_double(lines[4] + 42, 19);
    eph.omega_dot = parse_double(lines[4] + 61, 19);

    // Line 5: IDOT, codes_on_L2, GPS_week, L2_P_flag
    eph.idot           = parse_double(lines[5] + 4, 19);
    eph.ca_or_p_on_l2  = static_cast<uint8_t>(parse_double(lines[5] + 23, 19));
    double week_d      = parse_double(lines[5] + 42, 19);
    eph.week_number    = static_cast<uint16_t>(static_cast<int>(week_d));
    eph.l2_p_data_flag = static_cast<bool>(parse_double(lines[5] + 61, 19));

    // Line 6: SV_accuracy, SV_health, TGD, IODC
    eph.ura_index = static_cast<uint8_t>(parse_double(lines[6] + 4, 19));
    eph.sv_health = static_cast<uint8_t>(parse_double(lines[6] + 23, 19));
    eph.tgd       = parse_double(lines[6] + 42, 19);
    double iodc_d = parse_double(lines[6] + 61, 19);
    eph.iodc      = static_cast<uint16_t>(static_cast<int>(iodc_d));

    // Line 7: transmission_time, fit_interval
    // (transmission time ignored)
    double fit_d          = parse_double(lines[7] + 23, 19);
    eph.fit_interval_flag = (fit_d > 4.0);

    eph.lpp_iod = eph.iodc;
    eph.aodo    = 0;

    return true;
}

static bool parse_gal_record(char lines[8][128], ephemeris::GalEphemeris& eph) {
    int prn, year, month, day, hour, min, sec;
    if (sscanf(lines[0] + 1, "%d %d %d %d %d %d %d", &prn, &year, &month, &day, &hour, &min,
               &sec) != 7)
        return false;

    auto gps_time = ts::Gps::from_ymdhms(year, month, day, hour, min, static_cast<double>(sec));
    auto gst_time = ts::Gst(gps_time);

    eph             = {};
    eph.prn         = static_cast<uint8_t>(prn);
    eph.week_number = static_cast<uint16_t>(gst_time.week());
    eph.toc         = gst_time.time_of_week().full_seconds();
    eph.af0         = parse_double(lines[0] + 23, 19);
    eph.af1         = parse_double(lines[0] + 42, 19);
    eph.af2         = parse_double(lines[0] + 61, 19);

    // Line 1: IODnav, Crs, Delta_n, M0
    double iod_d = parse_double(lines[1] + 4, 19);
    eph.iod_nav  = static_cast<uint16_t>(static_cast<int>(iod_d));
    // LPP IOD for Galileo: the SSR orbit correction references iod_nav directly
    eph.lpp_iod = eph.iod_nav;
    eph.crs     = parse_double(lines[1] + 23, 19);
    eph.delta_n = parse_double(lines[1] + 42, 19);
    eph.m0      = parse_double(lines[1] + 61, 19);

    // Line 2: Cuc, e, Cus, sqrt(A)
    eph.cuc       = parse_double(lines[2] + 4, 19);
    eph.e         = parse_double(lines[2] + 23, 19);
    eph.cus       = parse_double(lines[2] + 42, 19);
    double sqrt_a = parse_double(lines[2] + 61, 19);
    eph.a         = sqrt_a * sqrt_a;

    // Line 3: Toe, Cic, OMEGA0, Cis
    eph.toe    = parse_double(lines[3] + 4, 19);
    eph.cic    = parse_double(lines[3] + 23, 19);
    eph.omega0 = parse_double(lines[3] + 42, 19);
    eph.cis    = parse_double(lines[3] + 61, 19);

    // Line 4: i0, Crc, omega, OMEGA_DOT
    eph.i0        = parse_double(lines[4] + 4, 19);
    eph.crc       = parse_double(lines[4] + 23, 19);
    eph.omega     = parse_double(lines[4] + 42, 19);
    eph.omega_dot = parse_double(lines[4] + 61, 19);

    // Line 5: IDOT, data_sources, GAL_week
    eph.idot = parse_double(lines[5] + 4, 19);
    // data_sources: bit0=I/NAV_E1B, bit1=F/NAV_E5a-I, bit2=I/NAV_E5b-I
    // SSR corrections reference I/NAV — reject F/NAV-only entries
    int  data_sources = static_cast<int>(parse_double(lines[5] + 23, 19));
    bool is_inav      = (data_sources & 0x01) || (data_sources & 0x04);  // bit0 or bit2
    if (!is_inav) return false;
    // Note: RINEX gives GPS week here, but week_number is already set correctly
    // from gst_time.week() above — do not overwrite with GPS week.

    return true;
}

static bool parse_bds_record(char lines[8][128], ephemeris::BdsEphemeris& eph) {
    int prn, year, month, day, hour, min, sec;
    if (sscanf(lines[0] + 1, "%d %d %d %d %d %d %d", &prn, &year, &month, &day, &hour, &min,
               &sec) != 7)
        return false;

    auto gps_time = ts::Gps::from_ymdhms(year, month, day, hour, min, static_cast<double>(sec));
    // BDT = GPS time - 14 seconds (BDS epoch is 2006-01-01)
    // But for week/tow we just convert via timestamp
    auto bdt_time = ts::Bdt(gps_time);

    eph             = {};
    eph.prn         = static_cast<uint8_t>(prn);
    eph.week_number = static_cast<uint16_t>(bdt_time.week());
    eph.toc         = bdt_time.time_of_week().full_seconds();
    eph.toc_time    = bdt_time;
    eph.af0         = parse_double(lines[0] + 23, 19);
    eph.af1         = parse_double(lines[0] + 42, 19);
    eph.af2         = parse_double(lines[0] + 61, 19);

    // Line 1: AODE, Crs, Delta_n, M0
    double aode_d = parse_double(lines[1] + 4, 19);
    eph.aode      = static_cast<uint8_t>(static_cast<int>(aode_d));
    eph.crs       = parse_double(lines[1] + 23, 19);
    eph.delta_n   = parse_double(lines[1] + 42, 19);
    eph.m0        = parse_double(lines[1] + 61, 19);

    // Line 2: Cuc, e, Cus, sqrt(A)
    eph.cuc       = parse_double(lines[2] + 4, 19);
    eph.e         = parse_double(lines[2] + 23, 19);
    eph.cus       = parse_double(lines[2] + 42, 19);
    double sqrt_a = parse_double(lines[2] + 61, 19);
    eph.a         = sqrt_a * sqrt_a;

    // Line 3: Toe, Cic, OMEGA0, Cis
    eph.toe    = parse_double(lines[3] + 4, 19);
    eph.cic    = parse_double(lines[3] + 23, 19);
    eph.omega0 = parse_double(lines[3] + 42, 19);
    eph.cis    = parse_double(lines[3] + 61, 19);

    // Line 4: i0, Crc, omega, OMEGA_DOT
    eph.i0        = parse_double(lines[4] + 4, 19);
    eph.crc       = parse_double(lines[4] + 23, 19);
    eph.omega     = parse_double(lines[4] + 42, 19);
    eph.omega_dot = parse_double(lines[4] + 61, 19);

    // Line 5: IDOT, spare, BDT_week
    eph.idot        = parse_double(lines[5] + 4, 19);
    double week_d   = parse_double(lines[5] + 42, 19);
    eph.week_number = static_cast<uint16_t>(static_cast<int>(week_d));

    // Line 6: SV_accuracy, SV_health, TGD1, TGD2
    eph.sv_health = static_cast<uint8_t>(parse_double(lines[6] + 23, 19));

    // Compute IOD from toe (same as rtcm2eph)
    eph.lpp_iod = static_cast<uint16_t>(static_cast<uint32_t>(eph.toe) >> 9);
    eph.iodc    = static_cast<uint8_t>((static_cast<uint32_t>(eph.toc) / 720) % 240);
    eph.iode    = static_cast<uint8_t>((static_cast<uint32_t>(eph.toe) / 720) % 240);
    eph.aodc    = eph.aode;

    eph.toe_time = ts::Bdt::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toe), 0.0);
    eph.toc_time = ts::Bdt::from_week_tow(eph.week_number, static_cast<int64_t>(eph.toc), 0.0);

    return true;
}

NavData parse_nav_file(std::string const& path) {
    NavData result;

    FILE* f = fopen(path.c_str(), "r");
    if (!f) {
        ERRORF("cannot open RINEX nav file: %s", path.c_str());
        return result;
    }

    // Skip header
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "END OF HEADER")) break;
    }

    // Parse records
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;
        if (line[0] == ' ') continue;  // continuation line without a record start

        char gnss    = line[0];
        int  n_lines = 8;                             // GPS/GAL/BDS: 1 header + 7 data lines
        if (gnss == 'R' || gnss == 'S') n_lines = 4;  // GLONASS/SBAS: 1 + 3

        char record[8][128];
        memset(record, 0, sizeof(record));
        size_t len = strlen(line);
        if (len > 127) len = 127;
        memcpy(record[0], line, len);

        bool ok = true;
        for (int i = 1; i < n_lines; i++) {
            if (!fgets(record[i], sizeof(record[i]), f)) {
                ok = false;
                break;
            }
        }
        if (!ok) break;

        if (gnss == 'G') {
            ephemeris::GpsEphemeris eph{};
            if (parse_gps_record(record, eph)) {
                result.gps.push_back(eph);
            }
        } else if (gnss == 'E') {
            ephemeris::GalEphemeris eph{};
            if (parse_gal_record(record, eph)) {
                result.gal.push_back(eph);
            }
        } else if (gnss == 'C') {
            ephemeris::BdsEphemeris eph{};
            if (parse_bds_record(record, eph)) {
                result.bds.push_back(eph);
            }
        }
        // R/S/J: skip (not supported)
    }

    fclose(f);
    INFOF("RINEX nav: loaded %zu GPS, %zu GAL, %zu BDS ephemerides from %s", result.gps.size(),
          result.gal.size(), result.bds.size(), path.c_str());
    return result;
}

}  // namespace rinex
}  // namespace format
