#include <generator/tokoro/diag.hpp>

#include <cstring>
#include <sys/stat.h>

#include <time/utc.hpp>

namespace generator {
namespace tokoro {

// Write a right-aligned header field name with 2-space prefix separator
static void hdr(FILE* f, char const* name, int width) {
    fprintf(f, "  %*s", width, name);
}

static void mkdirs(std::string const& path) {
    for (size_t i = 1; i < path.size(); ++i) {
        if (path[i] == '/') {
            auto dir = path.substr(0, i);
            mkdir(dir.c_str(), 0755);
        }
    }
    auto last_slash = path.rfind('/');
    if (last_slash != std::string::npos) {
        mkdir(path.substr(0, last_slash).c_str(), 0755);
    }
}

static char const* gnss_name(SatelliteId const& sv_id) {
    if (sv_id.is_gps()) return "GPS";
    if (sv_id.is_galileo()) return "Galileo";
    if (sv_id.is_beidou()) return "BeiDou";
    if (sv_id.is_glonass()) return "GLONASS";
    if (sv_id.is_qzss()) return "QZSS";
    return "Unknown";
}

// MSM absolute signal ID → RINEX code, matching RTCM standard / rtcm_decode.py
static char const* msm_sig(SatelliteId const& sv_id, int msm_id) {
    if (sv_id.is_gps()) {
        switch (msm_id) {
        case 2: return "1C";
        case 3: return "1P";
        case 4: return "1W";
        case 7: return "1X";
        case 8: return "2S";
        case 9: return "2L";
        case 10: return "2X";
        case 11: return "2P";
        case 12: return "2W";
        case 15: return "5I";
        case 16: return "5Q";
        case 17: return "5X";
        case 24: return "1X";
        case 30: return "1D";
        case 31: return "1P";
        case 32: return "1Z";
        }
    } else if (sv_id.is_galileo()) {
        switch (msm_id) {
        case 2: return "1C";
        case 3: return "1A";
        case 4: return "1B";
        case 5: return "1X";
        case 6: return "1Z";
        case 7: return "1X";
        case 8: return "6C";
        case 9: return "6A";
        case 10: return "6B";
        case 11: return "6X";
        case 12: return "6Z";
        case 14: return "7I";
        case 15: return "7Q";
        case 16: return "7X";
        case 18: return "8I";
        case 19: return "8Q";
        case 20: return "8X";
        case 22: return "8I";
        case 23: return "8Q";
        case 24: return "8X";
        case 25: return "5I";
        case 26: return "5Q";
        case 27: return "5X";
        }
    } else if (sv_id.is_beidou()) {
        switch (msm_id) {
        case 2: return "2I";
        case 3: return "2Q";
        case 4: return "2X";
        case 8: return "6I";
        case 9: return "6Q";
        case 10: return "6X";
        case 14: return "7I";
        case 15: return "7Q";
        case 16: return "7X";
        case 22: return "5D";
        case 23: return "5P";
        case 24: return "5X";
        case 25: return "1D";
        case 26: return "1P";
        case 27: return "1X";
        }
    }
    return "??";
}

std::string DiagFile::rtcm_signal_name(SatelliteId const& sv_id,
                                       SignalId const&    signal_id) noexcept {
    auto msm = signal_id.as_msm();
    if (msm.valid) return msm_sig(sv_id, static_cast<int>(msm.value));
    return signal_id.to_rinex();
}

DiagFile::~DiagFile() {
    if (mFile) fclose(mFile);
}

void DiagFile::open(std::string const& path, SatelliteId sv_id, SignalId signal_id) noexcept {
    mSvId       = sv_id;
    mSignalId   = signal_id;
    mMsmSigName = rtcm_signal_name(sv_id, signal_id);
    if (path.empty()) return;
    mkdirs(path);
    mFile = fopen(path.c_str(), "w");
}

void DiagFile::write_header() noexcept {
    if (!mFile) return;
    fprintf(mFile, "Tokoro Signal Diagnostic. Satellite: %s, Signal: %s\n", mSvId.name(),
            mMsmSigName.c_str());
    fprintf(mFile, "GNSS          : %s\n", gnss_name(mSvId));
    fprintf(mFile, "Start Field Description\n");
    fprintf(mFile, "TIMESTAMP             UTC date/time\n");
    fprintf(mFile, "PRN                   Satellite PRN number\n");
    fprintf(mFile, "SIG                   Signal name (RINEX code)\n");
    fprintf(mFile, "SIG_ID                MSM signal mask position (1-32)\n");
    fprintf(mFile, "CLOCK                 SSR clock correction (evaluated), meters\n");
    fprintf(mFile, "CODE_BIAS             Signal code bias, meters\n");
    fprintf(mFile, "PHASE_BIAS            Signal phase bias, meters\n");
    fprintf(mFile, "STEC_GRID             Grid ionospheric delay (STEC), meters\n");
    fprintf(mFile, "STEC_POLY             Polynomial ionospheric delay (STEC), meters\n");
    fprintf(mFile,
            "STEC_TOTAL            Total ionospheric delay (after height correction), meters\n");
    fprintf(mFile, "STEC_H_CORR           Ionospheric height correction factor\n");
    fprintf(mFile, "TROPO_DRY             Dry (hydrostatic) tropospheric delay, meters\n");
    fprintf(mFile, "TROPO_WET             Wet tropospheric delay, meters\n");
    fprintf(mFile, "TROPO_TOTAL           Total tropospheric delay, meters\n");
    fprintf(mFile, "TROPO_DRY_MAP         Hydrostatic mapping function value\n");
    fprintf(mFile, "TROPO_WET_MAP         Wet mapping function value\n");
    fprintf(mFile, "TROPO_DRY_ZEN         Zenith hydrostatic delay (before mapping), meters\n");
    fprintf(mFile, "TROPO_WET_ZEN         Zenith wet delay (before mapping), meters\n");
    fprintf(mFile, "PHASE_WINDUP          Phase windup correction, meters\n");
    fprintf(mFile, "ANT_PHASE             Satellite antenna phase variation, meters\n");
    fprintf(mFile, "CODE_CORRECTION       Total code correction, meters\n");
    fprintf(mFile, "PHASE_CORRECTION      Total phase correction, meters\n");
    fprintf(mFile, "CODE_RANGE            Final code pseudorange, meters\n");
    fprintf(mFile, "PHASE_RANGE           Final phase range, meters\n");
    fprintf(mFile, "PHASE_RATE            Phase range rate, meters/second\n");
    fprintf(mFile, "CNR                   Carrier-to-noise ratio, dB-Hz\n");
    fprintf(mFile, "LOCK_TIME             Phase lock time, seconds\n");
    fprintf(mFile, "FREQ_MHZ              Signal frequency, MHz\n");
    fprintf(mFile, "DISCARD_REASON        Reason for discard (- if valid)\n");
    fprintf(mFile, "End Field Description\n");
    // Column header — widths match data format exactly (2-space separator between each)
    fprintf(mFile, "*%-23s  %3s  %3s  %2s", "TIMESTAMP", "PRN", "SIG", "ID");
    hdr(mFile, "CLOCK", 10);
    hdr(mFile, "CODE_BIAS", 10);
    hdr(mFile, "PHASE_BIAS", 10);
    hdr(mFile, "STEC_GRID", 10);
    hdr(mFile, "STEC_POLY", 10);
    hdr(mFile, "STEC_TOTAL", 11);
    hdr(mFile, "STEC_H_CORR", 11);
    hdr(mFile, "TROPO_DRY", 10);
    hdr(mFile, "TROPO_WET", 10);
    hdr(mFile, "TROPO_TOTAL", 11);
    hdr(mFile, "TROPO_D_MAP", 11);
    hdr(mFile, "TROPO_W_MAP", 11);
    hdr(mFile, "TROPO_D_ZEN", 11);
    hdr(mFile, "TROPO_W_ZEN", 11);
    hdr(mFile, "PHASE_WINDUP", 12);
    hdr(mFile, "ANT_PHASE", 10);
    hdr(mFile, "CODE_CORRECTION", 16);
    hdr(mFile, "PHASE_CORRECTION", 16);
    hdr(mFile, "CODE_RANGE", 16);
    hdr(mFile, "PHASE_RANGE", 16);
    hdr(mFile, "PHASE_RATE", 12);
    hdr(mFile, "CNR", 6);
    hdr(mFile, "LOCK_TIME", 9);
    hdr(mFile, "FREQ_MHZ", 11);
    fprintf(mFile, "  DISCARD_REASON\n");
    mHeaderWritten = true;
}

void DiagFile::write(ts::Tai const& time, DiagRow const& row) noexcept {
    if (!mFile) return;
    if (!mHeaderWritten) write_header();

    auto utc = ts::Utc{time};
    auto ts  = utc.rfc3339();
    if (!ts.empty() && ts.back() == 'Z') ts.pop_back();

    auto prefix = row.valid ? ' ' : '#';

    // Helper buffers for optional fields
    char clock_s[16], code_bias_s[16], phase_bias_s[16];
    char stec_grid_s[16], stec_poly_s[16], stec_total_s[16], stec_hcorr_s[16];
    char tropo_dry_s[16], tropo_wet_s[16], tropo_total_s[16];
    char tropo_dmap_s[16], tropo_wmap_s[16], tropo_dzen_s[16], tropo_wzen_s[16];
    char phase_windup_s[16], ant_phase_s[16];
    char code_corr_s[20], phase_corr_s[20], code_range_s[20], phase_range_s[20];
    char phase_rate_s[16];

    if (row.has_clock)
        snprintf(clock_s, sizeof(clock_s), "%+10.5f", row.clock);
    else
        snprintf(clock_s, sizeof(clock_s), "%10s", "-");

    if (row.has_code_bias)
        snprintf(code_bias_s, sizeof(code_bias_s), "%+10.5f", row.code_bias);
    else
        snprintf(code_bias_s, sizeof(code_bias_s), "%10s", "-");

    if (row.has_phase_bias)
        snprintf(phase_bias_s, sizeof(phase_bias_s), "%+10.5f", row.phase_bias);
    else
        snprintf(phase_bias_s, sizeof(phase_bias_s), "%10s", "-");

    if (row.has_ionospheric) {
        snprintf(stec_grid_s, sizeof(stec_grid_s), "%+10.5f", row.stec_grid);
        snprintf(stec_poly_s, sizeof(stec_poly_s), "%+10.5f", row.stec_poly);
        snprintf(stec_hcorr_s, sizeof(stec_hcorr_s), "%+11.6f", row.stec_height_correction);
    } else {
        snprintf(stec_grid_s, sizeof(stec_grid_s), "%10s", "-");
        snprintf(stec_poly_s, sizeof(stec_poly_s), "%10s", "-");
        snprintf(stec_hcorr_s, sizeof(stec_hcorr_s), "%11s", "-");
    }

    if (row.has_tropospheric) {
        snprintf(tropo_dry_s, sizeof(tropo_dry_s), "%+10.5f", row.tropo_dry);
        snprintf(tropo_wet_s, sizeof(tropo_wet_s), "%+10.5f", row.tropo_wet);
        snprintf(tropo_total_s, sizeof(tropo_total_s), "%+11.5f", row.tropo_total);
        snprintf(tropo_dmap_s, sizeof(tropo_dmap_s), "%+11.6f", row.tropo_dry_mapping);
        snprintf(tropo_wmap_s, sizeof(tropo_wmap_s), "%+11.6f", row.tropo_wet_mapping);
        snprintf(tropo_dzen_s, sizeof(tropo_dzen_s), "%+11.5f", row.tropo_dry_zenith);
        snprintf(tropo_wzen_s, sizeof(tropo_wzen_s), "%+11.5f", row.tropo_wet_zenith);
    } else {
        snprintf(tropo_dry_s, sizeof(tropo_dry_s), "%10s", "-");
        snprintf(tropo_wet_s, sizeof(tropo_wet_s), "%10s", "-");
        snprintf(tropo_total_s, sizeof(tropo_total_s), "%11s", "-");
        snprintf(tropo_dmap_s, sizeof(tropo_dmap_s), "%11s", "-");
        snprintf(tropo_wmap_s, sizeof(tropo_wmap_s), "%11s", "-");
        snprintf(tropo_dzen_s, sizeof(tropo_dzen_s), "%11s", "-");
        snprintf(tropo_wzen_s, sizeof(tropo_wzen_s), "%11s", "-");
    }

    if (row.has_phase_windup)
        snprintf(phase_windup_s, sizeof(phase_windup_s), "%+12.5f", row.phase_windup);
    else
        snprintf(phase_windup_s, sizeof(phase_windup_s), "%12s", "-");

    if (row.has_ant_phase)
        snprintf(ant_phase_s, sizeof(ant_phase_s), "%+10.5f", row.ant_phase);
    else
        snprintf(ant_phase_s, sizeof(ant_phase_s), "%10s", "-");

    if (row.valid) {
        snprintf(stec_total_s, sizeof(stec_total_s), "%+11.5f", row.stec_total);
        snprintf(code_corr_s, sizeof(code_corr_s), "%+16.5f", row.code_correction);
        snprintf(phase_corr_s, sizeof(phase_corr_s), "%+16.5f", row.phase_correction);
        snprintf(code_range_s, sizeof(code_range_s), "%+16.5f", row.code_range);
        snprintf(phase_range_s, sizeof(phase_range_s), "%+16.5f", row.phase_range);
        snprintf(phase_rate_s, sizeof(phase_rate_s), "%+12.5f", row.phase_rate);
    } else {
        snprintf(stec_total_s, sizeof(stec_total_s), "%11s", "-");
        snprintf(code_corr_s, sizeof(code_corr_s), "%16s", "-");
        snprintf(phase_corr_s, sizeof(phase_corr_s), "%16s", "-");
        snprintf(code_range_s, sizeof(code_range_s), "%16s", "-");
        snprintf(phase_range_s, sizeof(phase_range_s), "%16s", "-");
        snprintf(phase_rate_s, sizeof(phase_rate_s), "%12s", "-");
    }

    fprintf(mFile,
            "%c%s  %3d  %3s  %2d"
            "  %s  %s  %s"
            "  %s  %s  %s  %s"
            "  %s  %s  %s  %s  %s  %s  %s"
            "  %s  %s"
            "  %s  %s  %s  %s"
            "  %s  %+6.2f  %+9.3f  %+11.4f  %s\n",
            prefix, ts.c_str(), row.prn, mMsmSigName.c_str(), row.sig_id, clock_s, code_bias_s,
            phase_bias_s, stec_grid_s, stec_poly_s, stec_total_s, stec_hcorr_s, tropo_dry_s,
            tropo_wet_s, tropo_total_s, tropo_dmap_s, tropo_wmap_s, tropo_dzen_s, tropo_wzen_s,
            phase_windup_s, ant_phase_s, code_corr_s, phase_corr_s, code_range_s, phase_range_s,
            phase_rate_s, row.cnr, row.lock_time, row.frequency_mhz,
            row.valid ? "-" : row.discard_reason.c_str());
}

SatDiagFile::~SatDiagFile() {
    if (mFile) fclose(mFile);
}

void SatDiagFile::open(std::string const& path, SatelliteId sv_id) noexcept {
    mSvId = sv_id;
    if (path.empty()) return;
    mkdirs(path);
    mFile = fopen(path.c_str(), "w");
}

void SatDiagFile::write_header() noexcept {
    if (!mFile) return;
    fprintf(mFile, "Tokoro Satellite Diagnostic. Satellite: %s\n", mSvId.name());
    fprintf(mFile, "GNSS          : %s\n", gnss_name(mSvId));
    fprintf(mFile, "Start Field Description\n");
    fprintf(mFile, "TIMESTAMP             UTC date/time\n");
    fprintf(mFile, "PRN                   Satellite PRN number\n");
    fprintf(mFile, "TRUE_RANGE            Geometric range to satellite, meters\n");
    fprintf(mFile, "EPH_RANGE             Ephemeris-only range (no SSR orbit), meters\n");
    fprintf(mFile, "ORBIT                 SSR orbit correction (TRUE_RANGE - EPH_RANGE), meters\n");
    fprintf(mFile, "ELEVATION             Satellite elevation angle, degrees\n");
    fprintf(mFile, "AZIMUTH               Satellite azimuth angle, degrees\n");
    fprintf(mFile, "NADIR                 Satellite nadir angle, degrees\n");
    fprintf(mFile, "EPH_CLOCK             Ephemeris clock correction (c * -clock_bias), meters\n");
    fprintf(mFile, "EPH_WEEK              Ephemeris week number used\n");
    fprintf(mFile, "EPH_TOE               Ephemeris TOE (seconds of week)\n");
    fprintf(mFile, "ORB_RADIAL            SSR orbit radial delta, meters\n");
    fprintf(mFile, "ORB_ALONG             SSR orbit along-track delta, meters\n");
    fprintf(mFile, "ORB_CROSS             SSR orbit cross-track delta, meters\n");
    fprintf(mFile, "ORB_RADIAL_DOT        SSR orbit radial rate, meters/second\n");
    fprintf(mFile, "ORB_ALONG_DOT         SSR orbit along-track rate, meters/second\n");
    fprintf(mFile, "ORB_CROSS_DOT         SSR orbit cross-track rate, meters/second\n");
    fprintf(mFile, "ORB_IOD               SSR orbit IOD\n");
    fprintf(mFile, "CLK_C0                SSR clock C0 coefficient, meters\n");
    fprintf(mFile, "CLK_C1                SSR clock C1 coefficient, meters/second\n");
    fprintf(mFile, "CLK_C2                SSR clock C2 coefficient, meters/second^2\n");
    fprintf(mFile, "CLK_CORR              Evaluated SSR clock correction, meters\n");
    fprintf(mFile, "IONO_C00              Ionospheric polynomial C00, TECU\n");
    fprintf(mFile, "IONO_C01              Ionospheric polynomial C01, TECU/deg\n");
    fprintf(mFile, "IONO_C10              Ionospheric polynomial C10, TECU/deg\n");
    fprintf(mFile, "IONO_C11              Ionospheric polynomial C11, TECU/deg^2\n");
    fprintf(mFile, "IONO_REF_LAT          Ionospheric polynomial reference latitude, degrees\n");
    fprintf(mFile, "IONO_REF_LON          Ionospheric polynomial reference longitude, degrees\n");
    fprintf(mFile, "SHAPIRO               Shapiro relativistic delay, meters\n");
    fprintf(mFile, "SOLID_TIDES           Earth solid tides displacement, meters\n");
    fprintf(mFile, "DISCARD_REASON        Reason for discard (- if valid)\n");
    fprintf(mFile, "End Field Description\n");
    // Column header — widths match data format exactly (2-space separator between each)
    fprintf(mFile, "*%-23s  %3s", "TIMESTAMP", "PRN");
    hdr(mFile, "TRUE_RANGE", 15);
    hdr(mFile, "EPH_RANGE", 15);
    hdr(mFile, "ORBIT", 13);
    hdr(mFile, "ELEV", 6);
    hdr(mFile, "AZIM", 7);
    hdr(mFile, "NADIR", 7);
    hdr(mFile, "EPH_CLOCK", 13);
    hdr(mFile, "EPH_WEEK", 7);
    hdr(mFile, "EPH_TOE", 9);
    hdr(mFile, "ORB_RADIAL", 10);
    hdr(mFile, "ORB_ALONG", 11);
    hdr(mFile, "ORB_CROSS", 11);
    hdr(mFile, "ORB_RAD_DOT", 11);
    hdr(mFile, "ORB_ALG_DOT", 11);
    hdr(mFile, "ORB_CRS_DOT", 11);
    hdr(mFile, "ORB_IOD", 7);
    hdr(mFile, "CLK_C0", 12);
    hdr(mFile, "CLK_C1", 12);
    hdr(mFile, "CLK_C2", 12);
    hdr(mFile, "CLK_CORR", 10);
    hdr(mFile, "IONO_C00", 9);
    hdr(mFile, "IONO_C01", 9);
    hdr(mFile, "IONO_C10", 9);
    hdr(mFile, "IONO_C11", 9);
    hdr(mFile, "IONO_REF_LAT", 12);
    hdr(mFile, "IONO_REF_LON", 12);
    hdr(mFile, "SHAPIRO", 10);
    hdr(mFile, "SOLID_TIDES", 11);
    fprintf(mFile, "  DISCARD_REASON\n");
    mHeaderWritten = true;
}

void SatDiagFile::write(ts::Tai const& time, SatDiagRow const& row) noexcept {
    if (!mFile) return;
    if (!mHeaderWritten) write_header();

    auto utc = ts::Utc{time};
    auto ts  = utc.rfc3339();
    if (!ts.empty() && ts.back() == 'Z') ts.pop_back();

    auto prefix = row.discard_reason.empty() ? ' ' : '#';

    // Geometry
    char true_range_s[20], eph_range_s[20], orbit_s[16], elev_s[10], azim_s[10], nadir_s[10],
        eph_clock_s[16];
    char eph_week_s[10], eph_toe_s[12];
    if (row.has_geometry) {
        snprintf(true_range_s, sizeof(true_range_s), "%+15.5f", row.true_range);
        snprintf(eph_range_s, sizeof(eph_range_s), "%+15.5f", row.eph_range);
        snprintf(orbit_s, sizeof(orbit_s), "%+13.5f", row.orbit);
        snprintf(elev_s, sizeof(elev_s), "%+6.2f", row.elevation_deg);
        snprintf(azim_s, sizeof(azim_s), "%+7.2f", row.azimuth_deg);
        snprintf(nadir_s, sizeof(nadir_s), "%+7.3f", row.nadir_deg);
        snprintf(eph_clock_s, sizeof(eph_clock_s), "%+13.5f", row.eph_clock);
        snprintf(eph_week_s, sizeof(eph_week_s), "%7u", row.eph_week);
        snprintf(eph_toe_s, sizeof(eph_toe_s), "%9.0f", row.eph_toe);
    } else {
        snprintf(true_range_s, sizeof(true_range_s), "%15s", "-");
        snprintf(eph_range_s, sizeof(eph_range_s), "%15s", "-");
        snprintf(orbit_s, sizeof(orbit_s), "%13s", "-");
        snprintf(elev_s, sizeof(elev_s), "%6s", "-");
        snprintf(azim_s, sizeof(azim_s), "%7s", "-");
        snprintf(nadir_s, sizeof(nadir_s), "%7s", "-");
        snprintf(eph_clock_s, sizeof(eph_clock_s), "%13s", "-");
        snprintf(eph_week_s, sizeof(eph_week_s), "%7s", "-");
        snprintf(eph_toe_s, sizeof(eph_toe_s), "%9s", "-");
    }

    // Orbit SSR
    char orb_r[16], orb_a[16], orb_c[16], orb_rd[16], orb_ad[16], orb_cd[16], orb_iod[8];
    if (row.has_orbit_ssr) {
        snprintf(orb_r, sizeof(orb_r), "%+10.5f", row.orbit_radial);
        snprintf(orb_a, sizeof(orb_a), "%+11.5f", row.orbit_along);
        snprintf(orb_c, sizeof(orb_c), "%+11.5f", row.orbit_cross);
        snprintf(orb_rd, sizeof(orb_rd), "%+11.7f", row.orbit_radial_dot);
        snprintf(orb_ad, sizeof(orb_ad), "%+11.7f", row.orbit_along_dot);
        snprintf(orb_cd, sizeof(orb_cd), "%+11.7f", row.orbit_cross_dot);
        snprintf(orb_iod, sizeof(orb_iod), "%7u", row.orbit_iod);
    } else {
        snprintf(orb_r, sizeof(orb_r), "%10s", "-");
        snprintf(orb_a, sizeof(orb_a), "%11s", "-");
        snprintf(orb_c, sizeof(orb_c), "%11s", "-");
        snprintf(orb_rd, sizeof(orb_rd), "%11s", "-");
        snprintf(orb_ad, sizeof(orb_ad), "%11s", "-");
        snprintf(orb_cd, sizeof(orb_cd), "%11s", "-");
        snprintf(orb_iod, sizeof(orb_iod), "%7s", "-");
    }

    // Clock SSR
    char clk_c0[14], clk_c1[14], clk_c2[14], clk_corr[12];
    if (row.has_clock_ssr) {
        snprintf(clk_c0, sizeof(clk_c0), "%+12.6f", row.clock_c0);
        snprintf(clk_c1, sizeof(clk_c1), "%+12.9f", row.clock_c1);
        snprintf(clk_c2, sizeof(clk_c2), "%+12.9f", row.clock_c2);
        snprintf(clk_corr, sizeof(clk_corr), "%+10.5f", row.clock_correction);
    } else {
        snprintf(clk_c0, sizeof(clk_c0), "%12s", "-");
        snprintf(clk_c1, sizeof(clk_c1), "%12s", "-");
        snprintf(clk_c2, sizeof(clk_c2), "%12s", "-");
        snprintf(clk_corr, sizeof(clk_corr), "%10s", "-");
    }

    // Iono polynomial
    char ic00[16], ic01[16], ic10[16], ic11[16], irlat[16], irlon[16];
    if (row.has_iono_poly) {
        snprintf(ic00, sizeof(ic00), "%+9.4f", row.iono_c00);
        snprintf(ic01, sizeof(ic01), "%+9.4f", row.iono_c01);
        snprintf(ic10, sizeof(ic10), "%+9.4f", row.iono_c10);
        snprintf(ic11, sizeof(ic11), "%+9.4f", row.iono_c11);
        snprintf(irlat, sizeof(irlat), "%+12.6f", row.iono_ref_lat);
        snprintf(irlon, sizeof(irlon), "%+12.6f", row.iono_ref_lon);
    } else {
        snprintf(ic00, sizeof(ic00), "%9s", "-");
        snprintf(ic01, sizeof(ic01), "%9s", "-");
        snprintf(ic10, sizeof(ic10), "%9s", "-");
        snprintf(ic11, sizeof(ic11), "%9s", "-");
        snprintf(irlat, sizeof(irlat), "%12s", "-");
        snprintf(irlon, sizeof(irlon), "%12s", "-");
    }

    // Shapiro & solid tides
    char shapiro_s[16], solid_tides_s[16];
    if (row.has_shapiro)
        snprintf(shapiro_s, sizeof(shapiro_s), "%+10.5f", row.shapiro);
    else
        snprintf(shapiro_s, sizeof(shapiro_s), "%10s", "-");
    if (row.has_solid_tides)
        snprintf(solid_tides_s, sizeof(solid_tides_s), "%+11.5f", row.solid_tides);
    else
        snprintf(solid_tides_s, sizeof(solid_tides_s), "%11s", "-");

    fprintf(mFile,
            "%c%s  %3d"
            "  %s  %s  %s  %s  %s  %s"
            "  %s  %s  %s"
            "  %s  %s  %s  %s  %s  %s  %s"
            "  %s  %s  %s  %s"
            "  %s  %s  %s  %s  %s  %s"
            "  %s  %s  %s\n",
            prefix, ts.c_str(), row.prn, true_range_s, eph_range_s, orbit_s, elev_s, azim_s,
            nadir_s, eph_clock_s, eph_week_s, eph_toe_s, orb_r, orb_a, orb_c, orb_rd, orb_ad,
            orb_cd, orb_iod, clk_c0, clk_c1, clk_c2, clk_corr, ic00, ic01, ic10, ic11, irlat, irlon,
            shapiro_s, solid_tides_s,
            row.discard_reason.empty() ? "-" : row.discard_reason.c_str());
}

}  // namespace tokoro
}  // namespace generator
