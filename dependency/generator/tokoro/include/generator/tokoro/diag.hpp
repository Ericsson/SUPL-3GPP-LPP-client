#pragma once
#include <cstdio>
#include <string>

#include <gnss/satellite_id.hpp>
#include <gnss/signal_id.hpp>
#include <time/tai.hpp>

namespace generator {
namespace tokoro {

struct DiagEpoch {
    ts::Tai time{};
    double  age{};
    bool    valid{false};

    void set(ts::Tai const& epoch, ts::Tai const& now) noexcept {
        time  = epoch;
        age   = now - epoch;
        valid = true;
    }
};

struct DiagRow {
    int prn;
    int sig_id;  // MSM mask position

    // Clock
    double clock{};  // SSR clock correction (evaluated)

    // Biases
    double code_bias{};
    double phase_bias{};

    // Assistance data time of the biases
    DiagEpoch code_bias_epoch;
    DiagEpoch phase_bias_epoch;

    // SSR phase discontinuity / integer indicators (0..3) of the phase bias in use, and whether
    // the discontinuity indicator changed since the previous epoch (cycle slip).
    int phase_discontinuity{};
    // TODO(ewasjon): Whole cycles added to the phase range (N * wavelength) per satellite/signal.
    // Integer support is not implemented yet, so this is always 0.
    long integer_cycles{0};
    int  phase_integer{};
    bool cycle_slip{false};
    bool has_phase_discontinuity{false};
    bool has_phase_integer{false};
    bool has_cycle_slip{false};

    // Ionosphere
    double stec_grid{};
    double stec_poly{};
    double stec_total{};  // after height correction
    double stec_height_correction{1.0};

    // Troposphere
    double tropo_dry{};
    double tropo_wet{};
    double tropo_total{};
    double tropo_dry_mapping{};
    double tropo_wet_mapping{};
    double tropo_dry_zenith{};
    double tropo_wet_zenith{};

    // Signal-level corrections
    double phase_windup{};
    double ant_phase{};

    // Combined corrections and final ranges
    double code_correction{};
    double phase_correction{};
    double code_range{};
    double phase_range{};
    double phase_rate{};

    double cnr{};
    double lock_time{};
    double frequency_mhz{};

    // Validity
    bool        valid{false};
    bool        has_clock{false};
    bool        has_code_bias{false};
    bool        has_phase_bias{false};
    bool        has_ionospheric{false};
    bool        has_tropospheric{false};
    bool        has_phase_windup{false};
    bool        has_ant_phase{false};
    std::string discard_reason;
};

class DiagFile {
public:
    DiagFile() = default;
    ~DiagFile();

    DiagFile(DiagFile const&)            = delete;
    DiagFile& operator=(DiagFile const&) = delete;
    DiagFile(DiagFile&&)                 = default;
    DiagFile& operator=(DiagFile&&)      = default;

    void open(std::string const& path, SatelliteId sv_id, SignalId signal_id) noexcept;
    void write(ts::Tai const& time, DiagRow const& row) noexcept;

    NODISCARD static std::string rtcm_signal_name(SatelliteId const& sv_id,
                                                  SignalId const&    signal_id) noexcept;

private:
    FILE*       mFile{nullptr};
    SatelliteId mSvId{};
    SignalId    mSignalId{};
    std::string mMsmSigName;
    bool        mHeaderWritten{false};

    void write_header() noexcept;
};

struct SatDiagRow {
    int prn;

    // Geometry (available after successful position computation)
    double   true_range{};
    double   eph_range{};
    double   orbit{};
    double   elevation_deg{};
    double   azimuth_deg{};
    double   nadir_deg{};
    double   eph_clock{};
    uint16_t eph_week{};
    double   eph_toe{};

    // Raw SSR orbit correction
    double   orbit_radial{};
    double   orbit_along{};
    double   orbit_cross{};
    double   orbit_radial_dot{};
    double   orbit_along_dot{};
    double   orbit_cross_dot{};
    uint16_t orbit_iod{};

    // Raw SSR clock correction
    double clock_c0{};
    double clock_c1{};
    double clock_c2{};

    // Evaluated SSR clock correction
    double clock_correction{};

    // Ionospheric polynomial (per-satellite)
    double iono_c00{};
    double iono_c01{};
    double iono_c10{};
    double iono_c11{};
    double iono_ref_lat{};
    double iono_ref_lon{};
    double iono_quality{};  // SSR STEC quality indicator, TECU (1-sigma)

    // Assistance data time (message epoch and age at the row time)
    DiagEpoch orbit_epoch;
    DiagEpoch clock_epoch;
    DiagEpoch iono_epoch;  // SSR STEC message epoch
    DiagEpoch grid_epoch;  // SSR gridded (troposphere/ionosphere residual) message epoch

    // Shapiro & solid tides
    double shapiro{};
    double solid_tides{};

    // Flags
    bool has_geometry{false};
    bool has_orbit_ssr{false};
    bool has_clock_ssr{false};
    bool has_iono_poly{false};
    bool has_iono_quality{false};
    bool has_shapiro{false};
    bool has_solid_tides{false};

    std::string discard_reason;
};

class SatDiagFile {
public:
    SatDiagFile() = default;
    ~SatDiagFile();

    SatDiagFile(SatDiagFile const&)            = delete;
    SatDiagFile& operator=(SatDiagFile const&) = delete;
    SatDiagFile(SatDiagFile&&)                 = default;
    SatDiagFile& operator=(SatDiagFile&&)      = default;

    void open(std::string const& path, SatelliteId sv_id) noexcept;
    void write(ts::Tai const& time, SatDiagRow const& row) noexcept;

private:
    FILE*       mFile{nullptr};
    SatelliteId mSvId{};
    bool        mHeaderWritten{false};

    void write_header() noexcept;
};

}  // namespace tokoro
}  // namespace generator
