#include <format/rinex/nav.hpp>
#include <format/rinex/obs.hpp>
#include <generator/idokeido/correction.hpp>
#include <generator/idokeido/eph.hpp>
#include <generator/idokeido/spp.hpp>
#include <loglet/loglet.hpp>

#include <ephemeris/bds.hpp>
#include <ephemeris/glo.hpp>
#include <ephemeris/qzs.hpp>
#include <time/gps.hpp>

#include <args.hxx>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

static idokeido::SppConfiguration default_config() {
    idokeido::SppConfiguration cfg{};
    cfg.relativistic_model    = idokeido::RelativisticModel::Broadcast;
    cfg.ionospheric_mode      = idokeido::IonosphericMode::Broadcast;
    cfg.tropospheric_mode     = idokeido::TroposphericMode::Saastamoinen;
    cfg.weight_function       = idokeido::WeightFunction::Variance;
    cfg.sigma_a               = 0.3;
    cfg.sigma_b               = 0.3;
    cfg.epoch_selection       = idokeido::EpochSelection::LastObservation;
    cfg.filter_mode           = idokeido::FilterMode::None;
    cfg.gnss.gps              = true;
    cfg.gnss.glo              = false;
    cfg.gnss.gal              = true;
    cfg.gnss.bds              = false;
    cfg.gnss.qzs              = false;
    cfg.observation_window    = 1.0;
    cfg.elevation_cutoff      = 10.0;
    cfg.snr_cutoff            = 0.0;
    cfg.outlier_cutoff        = 30.0;
    cfg.reject_cycle_slip     = false;
    cfg.reject_halfcycle_slip = false;
    cfg.reject_outliers       = true;
    cfg.process_noise         = {0.0, 0.0, 0.0, 1e6};
    return cfg;
}

int main(int argc, char** argv) {
    args::ArgumentParser parser("SPP engine — RINEX-based single point positioning");
    args::HelpFlag       help{parser, "help", "Show help", {'h', "help"}};

    args::Positional<std::string> nav_arg{parser, "nav", "RINEX nav file"};
    args::Positional<std::string> obs_arg{parser, "obs", "RINEX obs file"};

    // Truth
    args::ValueFlag<double> tx{parser, "X", "Truth ECEF X (m)", {"tx"}};
    args::ValueFlag<double> ty{parser, "Y", "Truth ECEF Y (m)", {"ty"}};
    args::ValueFlag<double> tz{parser, "Z", "Truth ECEF Z (m)", {"tz"}};

    // GNSS systems
    args::Flag gps_flag{parser, "gps", "Enable GPS", {"gps"}};
    args::Flag glo_flag{parser, "glo", "Enable GLONASS", {"glo"}};
    args::Flag gal_flag{parser, "gal", "Enable Galileo", {"gal"}};
    args::Flag bds_flag{parser, "bds", "Enable BDS", {"bds"}};
    args::Flag qzs_flag{parser, "qzs", "Enable QZSS", {"qzs"}};

    // Filter
    args::ValueFlag<std::string> filter_flag{
        parser, "mode", "Filter mode: none|static|kinematic|kinematic-velocity", {"filter"}};
    args::ValueFlag<double> noise_h{parser, "m2/s", "Process noise horizontal (m²/s)", {"noise-h"}};
    args::ValueFlag<double> noise_v{parser, "m2/s", "Process noise vertical (m²/s)", {"noise-v"}};
    args::ValueFlag<double> noise_clk{parser, "m2/s", "Process noise clock (m²/s)", {"noise-clk"}};
    args::ValueFlag<double> noise_vel{
        parser, "m2/s", "Process noise velocity (m²/s)", {"noise-vel"}};

    // Cutoffs
    args::ValueFlag<double> snr_flag{parser, "dBHz", "SNR cutoff (dBHz)", {"snr"}};
    args::ValueFlag<double> el_flag{parser, "deg", "Elevation cutoff (deg)", {"elevation"}};

    // Output
    args::ValueFlag<std::string> out_flag{parser, "file", "CSV output file", {"output"}};
    args::ValueFlag<std::string> stat_flag{parser, "file", "SAT stat file", {"stat"}};

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help const&) {
        printf("%s", parser.Help().c_str());
        return 0;
    } catch (args::ParseError const& e) {
        fprintf(stderr, "%s\n", e.what());
        return 1;
    }

    if (!nav_arg || !obs_arg) {
        printf("%s", parser.Help().c_str());
        return 1;
    }

    auto cfg = default_config();

    if (gps_flag || glo_flag || gal_flag || bds_flag || qzs_flag) {
        cfg.gnss.gps = gps_flag ? true : false;
        cfg.gnss.glo = glo_flag ? true : false;
        cfg.gnss.gal = gal_flag ? true : false;
        cfg.gnss.bds = bds_flag ? true : false;
        cfg.gnss.qzs = qzs_flag ? true : false;
    }
    if (snr_flag) cfg.snr_cutoff = args::get(snr_flag);
    if (el_flag) cfg.elevation_cutoff = args::get(el_flag);

    if (filter_flag) {
        auto m = args::get(filter_flag);
        if (m == "static")
            cfg.filter_mode = idokeido::FilterMode::Static;
        else if (m == "kinematic")
            cfg.filter_mode = idokeido::FilterMode::Kinematic;
        else if (m == "kinematic-velocity")
            cfg.filter_mode = idokeido::FilterMode::KinematicVelocity;
        else if (m == "none")
            cfg.filter_mode = idokeido::FilterMode::None;
        else {
            fprintf(stderr, "Unknown filter mode: %s\n", m.c_str());
            return 1;
        }

        // Set sensible defaults for the chosen mode
        if (cfg.filter_mode == idokeido::FilterMode::Static) {
            cfg.process_noise = {1e-8, 1e-8, 0.0, 1e6};
        } else if (cfg.filter_mode == idokeido::FilterMode::Kinematic) {
            cfg.process_noise = {1.0, 0.1, 0.0, 1e6};
        } else if (cfg.filter_mode == idokeido::FilterMode::KinematicVelocity) {
            cfg.process_noise = {1.0, 0.1, 0.1, 1e6};
        }
    }
    if (noise_h) cfg.process_noise.position_horizontal = args::get(noise_h);
    if (noise_v) cfg.process_noise.position_vertical = args::get(noise_v);
    if (noise_clk) cfg.process_noise.clock = args::get(noise_clk);
    if (noise_vel) cfg.process_noise.velocity = args::get(noise_vel);

    bool   has_truth = tx && ty && tz;
    double truth_x   = has_truth ? args::get(tx) : 0.0;
    double truth_y   = has_truth ? args::get(ty) : 0.0;
    double truth_z   = has_truth ? args::get(tz) : 0.0;

    FILE* out_file  = nullptr;
    FILE* stat_file = nullptr;
    if (out_flag) {
        out_file = fopen(args::get(out_flag).c_str(), "w");
        if (!out_file) {
            fprintf(stderr, "Cannot open: %s\n", args::get(out_flag).c_str());
            return 1;
        }
        fprintf(out_file, "time,sats,x,y,z,lat,lon,alt,clock,pdop,hdop,vdop,tdop");
        if (has_truth) fprintf(out_file, ",dx,dy,dz,d3");
        fprintf(out_file, "\n");
    }
    if (stat_flag) {
        stat_file = fopen(args::get(stat_flag).c_str(), "w");
        if (!stat_file) {
            fprintf(stderr, "Cannot open: %s\n", args::get(stat_flag).c_str());
            return 1;
        }
    }

    loglet::initialize();
    loglet::set_level(loglet::Level::Warning);
    loglet::set_use_stderr(true);

    idokeido::EphemerisEngine eph_engine;
    idokeido::CorrectionCache correction_cache;
    auto                      engine = idokeido::SppEngine(cfg, eph_engine, correction_cache);

    format::rinex::NavCallbacks nav_cb;
    nav_cb.gps = [&](ephemeris::GpsEphemeris const& e) {
        eph_engine.add(e);
    };
    nav_cb.gal = [&](ephemeris::GalEphemeris const& e) {
        eph_engine.add(e);
    };
    nav_cb.bds = [&](ephemeris::BdsEphemeris const& e) {
        eph_engine.add(e);
    };
    nav_cb.glo = [&](ephemeris::GloEphemeris const& e) {
        eph_engine.add(e);
    };
    nav_cb.qzs = [&](ephemeris::QzsEphemeris const& e) {
        eph_engine.add(e);
    };
    nav_cb.klobuchar = [&](double alpha[4], double beta[4]) {
        idokeido::KlobucharModelParameters p{};
        for (int i = 0; i < 4; ++i) {
            p.a[i] = alpha[i];
            p.b[i] = beta[i];
        }
        engine.klobuchar_model(p);
    };
    nav_cb.bds_klobuchar = [&](double alpha[4], double beta[4]) {
        idokeido::KlobucharModelParameters p{};
        for (int i = 0; i < 4; ++i) {
            p.a[i] = alpha[i];
            p.b[i] = beta[i];
        }
        engine.bds_klobuchar_model(p);
    };

    if (!format::rinex::parse_nav(args::get(nav_arg), nav_cb)) {
        fprintf(stderr, "Failed to parse nav file\n");
        return 1;
    }

    printf("%-25s  %5s", "time", "sats");
    if (has_truth) printf("  %8s %8s %8s %8s", "dx", "dy", "dz", "3d");
    printf("  %14s %14s %14s\n", "lat_deg", "lon_deg", "alt_m");

    struct Sample {
        double dx, dy, dz, de, dn, du, d3;
    };
    std::vector<Sample> samples;

    auto llh_to_enu = [](double x, double y, double z) {
        double r   = std::sqrt(x * x + y * y + z * z);
        double lat = std::asin(z / r), lon = std::atan2(y, x);
        double sl = std::sin(lat), cl = std::cos(lat), sn = std::sin(lon), cn = std::cos(lon);
        return std::array<std::array<double, 3>, 3>{
            {{{-sn, cn, 0.0}}, {{-sl * cn, -sl * sn, cl}}, {{cl * cn, cl * sn, sl}}}};
    };

    format::rinex::parse_obs(args::get(obs_arg), [&](format::rinex::ObsEpoch const& epoch) {
        for (auto const& m : epoch.measurements) {
            idokeido::RawMeasurement raw{};
            raw.time          = epoch.time;
            raw.satellite_id  = m.satellite_id;
            raw.signal_id     = m.signal_id;
            raw.pseudo_range  = m.pseudorange;
            raw.carrier_phase = m.carrier_phase;
            raw.doppler       = m.doppler;
            raw.snr           = m.snr;
            raw.lock_time     = m.loss_of_lock ? 0.0 : 1.0;
            engine.add_measurement(raw);
        }

        auto sol = engine.evaluate();
        if (sol.status != idokeido::Solution::Status::Standard) return;

        printf("%-25s  %5zu", epoch.time.rtklib_time_string(1).c_str(), sol.satellite_count);

        double dx = 0, dy = 0, dz = 0, d3 = 0;
        if (has_truth) {
            dx = sol.position_ecef.x() - truth_x;
            dy = sol.position_ecef.y() - truth_y;
            dz = sol.position_ecef.z() - truth_z;
            d3 = std::sqrt(dx * dx + dy * dy + dz * dz);
            printf("  %8.3f %8.3f %8.3f %8.3f", dx, dy, dz, d3);
            auto R = llh_to_enu(truth_x, truth_y, truth_z);
            samples.push_back({dx, dy, dz, R[0][0] * dx + R[0][1] * dy + R[0][2] * dz,
                               R[1][0] * dx + R[1][1] * dy + R[1][2] * dz,
                               R[2][0] * dx + R[2][1] * dy + R[2][2] * dz, d3});
        }
        printf("  %14.9f %14.9f %14.3f\n", sol.latitude, sol.longitude, sol.altitude);

        if (out_file) {
            fprintf(out_file, "%s,%zu,%.4f,%.4f,%.4f,%.9f,%.9f,%.4f,%.4f,%.3f,%.3f,%.3f,%.3f",
                    epoch.time.rtklib_time_string(1).c_str(), sol.satellite_count,
                    sol.position_ecef.x(), sol.position_ecef.y(), sol.position_ecef.z(),
                    sol.latitude, sol.longitude, sol.altitude, sol.receiver_clock, sol.pdop,
                    sol.hdop, sol.vdop, sol.tdop);
            if (has_truth) fprintf(out_file, ",%.4f,%.4f,%.4f,%.4f", dx, dy, dz, d3);
            fprintf(out_file, "\n");
        }
        if (stat_file) {
            auto   gps_time = ts::Gps(sol.time);
            int    week     = static_cast<int>(gps_time.week());
            double tow      = gps_time.time_of_week().full_seconds();
            for (auto const& sat : sol.satellites) {
                fprintf(stat_file, "$SAT,%d,%.3f,%s,1,%.1f,%.1f,%.4f,%.3f,%.3f,%.3f,%.10f,%.3f\n",
                        week, tow, sat.id.name(), sat.azimuth * (180.0 / 3.14159265358979),
                        sat.elevation * (180.0 / 3.14159265358979), sat.residual, sat.sat_pos.x(),
                        sat.sat_pos.y(), sat.sat_pos.z(), sat.sat_clock, sat.pseudorange);
            }
        }
    });

    if (out_file) fclose(out_file);
    if (stat_file) fclose(stat_file);

    if (has_truth && !samples.empty()) {
        size_t n  = samples.size();
        double nd = static_cast<double>(n);

        auto stats = [&](auto get) {
            double mean = 0;
            for (auto& s : samples)
                mean += get(s);
            mean /= nd;
            double var = 0, mn = get(samples[0]), mx = get(samples[0]);
            for (auto& s : samples) {
                double v = get(s);
                var += (v - mean) * (v - mean);
                mn = std::min(mn, v);
                mx = std::max(mx, v);
            }
            return std::array<double, 4>{mean, std::sqrt(var / nd), mn, mx};
        };

        auto [mdx, sdx, mndx, mxdx] = stats([](Sample& s) {
            return s.dx;
        });
        auto [mdy, sdy, mndy, mxdy] = stats([](Sample& s) {
            return s.dy;
        });
        auto [mdz, sdz, mndz, mxdz] = stats([](Sample& s) {
            return s.dz;
        });
        auto [mde, sde, mnde, mxde] = stats([](Sample& s) {
            return s.de;
        });
        auto [mdn, sdn, mndn, mxdn] = stats([](Sample& s) {
            return s.dn;
        });
        auto [mdu, sdu, mndu, mxdu] = stats([](Sample& s) {
            return s.du;
        });
        auto [md3, sd3, mn3, mx3]   = stats([](Sample& s) {
            return s.d3;
        });

        std::vector<double> e3d;
        for (auto& s : samples)
            e3d.push_back(s.d3);
        std::sort(e3d.begin(), e3d.end());
        double p95 = e3d[static_cast<size_t>(0.95 * static_cast<double>(n - 1))];

        printf("\n--- Summary (%zu epochs) ---\n", n);
        printf("  %-6s  %8s  %8s  %8s  %8s  %8s  %8s\n", "", "mean", "1-sigma", "2-sigma",
               "3-sigma", "min", "max");
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n", "dx", mdx, sdx, 2 * sdx,
               3 * sdx, mndx, mxdx);
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n", "dy", mdy, sdy, 2 * sdy,
               3 * sdy, mndy, mxdy);
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n", "dz", mdz, sdz, 2 * sdz,
               3 * sdz, mndz, mxdz);
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n", "east", mde, sde, 2 * sde,
               3 * sde, mnde, mxde);
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n", "north", mdn, sdn, 2 * sdn,
               3 * sdn, mndn, mxdn);
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f\n", "up", mdu, sdu, 2 * sdu,
               3 * sdu, mndu, mxdu);
        printf("  %-6s  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  %8.3f  (95th: %.3f)\n", "3d", md3, sd3,
               2 * sd3, 3 * sd3, mn3, mx3, p95);
    }

    return 0;
}
