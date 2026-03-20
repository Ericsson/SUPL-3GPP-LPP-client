#include <format/rinex/nav.hpp>
#include <format/rinex/obs.hpp>
#include <generator/idokeido/correction.hpp>
#include <generator/idokeido/eph.hpp>
#include <generator/idokeido/ppp.hpp>
#include <generator/idokeido/spp.hpp>
#include <loglet/loglet.hpp>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

static void print_usage(char const* prog) {
    printf("Usage: %s <nav_file> <obs_file> [truth_x truth_y truth_z]\n", prog);
    printf("  nav_file  : RINEX 3 navigation file\n");
    printf("  obs_file  : RINEX 3 observation file\n");
    printf("  truth_xyz : optional ECEF truth position in meters\n");
}

static idokeido::SppConfiguration default_spp_config() {
    idokeido::SppConfiguration cfg{};
    cfg.relativistic_model    = idokeido::RelativisticModel::Broadcast;
    cfg.ionospheric_mode      = idokeido::IonosphericMode::None;
    cfg.weight_function       = idokeido::WeightFunction::None;
    cfg.epoch_selection       = idokeido::EpochSelection::LastObservation;
    cfg.gnss.gps              = true;
    cfg.gnss.glo              = false;
    cfg.gnss.gal              = true;
    cfg.gnss.bds              = false;
    cfg.observation_window    = 1.0;
    cfg.elevation_cutoff      = 10.0;
    cfg.snr_cutoff            = 0.0;
    cfg.outlier_cutoff        = 100.0;
    cfg.reject_cycle_slip     = false;
    cfg.reject_halfcycle_slip = false;
    cfg.reject_outliers       = false;
    return cfg;
}

static idokeido::PppConfiguration default_ppp_config() {
    idokeido::PppConfiguration cfg{};
    cfg.mode                   = idokeido::PppMode::Static;
    cfg.ionosphere_mode        = idokeido::PppIonosphereMode::None;
    cfg.troposphere_wet_mode   = idokeido::PppTroposphereMode::Estimated;
    cfg.troposphere_dry_mode   = idokeido::PppTroposphereMode::Estimated;
    cfg.apply_phase_windup     = false;
    cfg.apply_shapiro          = false;
    cfg.relativistic_model     = idokeido::RelativisticModel::Broadcast;
    cfg.weight_function        = idokeido::WeightFunction::None;
    cfg.epoch_selection        = idokeido::EpochSelection::LastObservation;
    cfg.gnss.gps               = true;
    cfg.gnss.glo               = false;
    cfg.gnss.gal               = true;
    cfg.gnss.bds               = false;
    cfg.observation_window     = 1.0;
    cfg.elevation_cutoff       = 10.0;
    cfg.snr_cutoff             = 0.0;
    cfg.reject_cycle_slip      = true;
    cfg.reject_halfcycle_slip  = false;
    cfg.use_carrier_phase      = true;
    cfg.sigma_pseudorange      = 3.0;
    cfg.sigma_carrier_phase    = 0.003;
    cfg.process_noise_position = 0.0;
    cfg.process_noise_clock    = 1e6;
    cfg.process_noise_ztd_wet  = 1e-8;
    cfg.process_noise_ztd_dry  = 1e-10;
    cfg.process_noise_iono     = 1e-6;
    return cfg;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    std::string nav_path = argv[1];
    std::string obs_path = argv[2];

    double truth_x = 0.0, truth_y = 0.0, truth_z = 0.0;
    bool   has_truth = (argc >= 6);
    if (has_truth) {
        truth_x = std::stod(argv[3]);
        truth_y = std::stod(argv[4]);
        truth_z = std::stod(argv[5]);
    }

    // Enable WARN logging for SPP to diagnose why satellites are rejected
    loglet::initialize();
    loglet::set_level(loglet::Level::Info);
    loglet::set_use_stderr(true);

    // Load navigation data
    idokeido::EphemerisEngine   eph_engine;
    format::rinex::NavCallbacks nav_cb;
    nav_cb.gps = [&](ephemeris::GpsEphemeris const& e) {
        eph_engine.add(e);
    };
    nav_cb.gal = [&](ephemeris::GalEphemeris const& e) {
        eph_engine.add(e);
    };

    if (!format::rinex::parse_nav(nav_path, nav_cb)) {
        fprintf(stderr, "Failed to parse nav file: %s\n", nav_path.c_str());
        return 1;
    }
    // Create engines
    idokeido::CorrectionCache correction_cache;
    auto spp_engine = idokeido::SppEngine(default_spp_config(), eph_engine, correction_cache);
    auto ppp_engine = idokeido::PppEngine(default_ppp_config(), eph_engine, correction_cache);
    bool ppp_seeded = false;

    printf("%-25s  %8s %8s %8s %8s %8s  %8s %8s %8s %8s %8s\n", "time", "spp_sats", "spp_dx",
           "spp_dy", "spp_dz", "spp_3d", "ppp_sats", "ppp_dx", "ppp_dy", "ppp_dz", "ppp_3d");
    if (has_truth) {
        printf("  (truth: %.3f %.3f %.3f)\n", truth_x, truth_y, truth_z);
    }

    // Process observations
    format::rinex::parse_obs(obs_path, [&](format::rinex::ObsEpoch const& epoch) {
        for (auto const& m : epoch.measurements) {
            idokeido::RawMeasurement raw{};
            raw.time          = m.satellite_id.is_valid() ? epoch.time : epoch.time;
            raw.time          = epoch.time;
            raw.satellite_id  = m.satellite_id;
            raw.signal_id     = m.signal_id;
            raw.pseudo_range  = m.pseudorange;
            raw.carrier_phase = m.carrier_phase;
            raw.doppler       = m.doppler;
            raw.snr           = m.snr;
            raw.lock_time     = m.loss_of_lock ? 0.0 : 1.0;

            spp_engine.add_measurement(raw);
            ppp_engine.add_measurement(raw);
        }

        auto spp = spp_engine.evaluate();
        if (!ppp_seeded && spp.status == idokeido::Solution::Status::Standard) {
            ppp_engine.seed(spp);
            ppp_seeded = true;
        }
        auto ppp = ppp_seeded ? ppp_engine.evaluate() : idokeido::PppSolution{};

        char time_str[32];
        std::snprintf(time_str, sizeof(time_str), "%s", epoch.time.rtklib_time_string(1).c_str());

        printf("%-25s  %8zu", time_str, spp.satellite_count);
        if (has_truth && spp.status == idokeido::Solution::Status::Standard) {
            double sdx = spp.position_ecef.x() - truth_x;
            double sdy = spp.position_ecef.y() - truth_y;
            double sdz = spp.position_ecef.z() - truth_z;
            printf(" %8.3f %8.3f %8.3f %8.3f", sdx, sdy, sdz,
                   std::sqrt(sdx * sdx + sdy * sdy + sdz * sdz));
        } else {
            printf(" %8s %8s %8s %8s", "-", "-", "-", "-");
        }

        printf("  %8zu", ppp.satellite_count);
        if (has_truth && ppp.status == idokeido::PppSolution::Status::Standard) {
            double dx = ppp.position_ecef.x() - truth_x;
            double dy = ppp.position_ecef.y() - truth_y;
            double dz = ppp.position_ecef.z() - truth_z;
            printf(" %8.3f %8.3f %8.3f %8.3f", dx, dy, dz, std::sqrt(dx * dx + dy * dy + dz * dz));
        } else {
            printf(" %8s %8s %8s %8s", "-", "-", "-", "-");
        }
        printf("\n");
    });

    return 0;
}
