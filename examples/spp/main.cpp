#include <format/rinex/nav.hpp>
#include <format/rinex/obs.hpp>
#include <generator/idokeido/correction.hpp>
#include <generator/idokeido/eph.hpp>
#include <generator/idokeido/spp.hpp>
#include <loglet/loglet.hpp>

#include <ephemeris/bds.hpp>
#include <ephemeris/glo.hpp>
#include <time/gps.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

static void print_usage(char const* prog) {
    printf("Usage: %s <nav_file> <obs_file> [truth_x truth_y truth_z] [--output <file>]\n", prog);
}

static idokeido::SppConfiguration default_config() {
    idokeido::SppConfiguration cfg{};
    cfg.relativistic_model    = idokeido::RelativisticModel::Broadcast;
    cfg.ionospheric_mode      = idokeido::IonosphericMode::Broadcast;
    cfg.tropospheric_mode     = idokeido::TroposphericMode::Saastamoinen;
    cfg.weight_function       = idokeido::WeightFunction::Variance;
    cfg.sigma_a               = 0.3;
    cfg.sigma_b               = 0.3;
    cfg.epoch_selection       = idokeido::EpochSelection::LastObservation;
    cfg.gnss.gps              = true;
    cfg.gnss.glo              = false;
    cfg.gnss.gal              = true;
    cfg.gnss.bds              = false;
    cfg.observation_window    = 1.0;
    cfg.elevation_cutoff      = 10.0;
    cfg.snr_cutoff            = 0.0;
    cfg.outlier_cutoff        = 30.0;
    cfg.reject_cycle_slip     = false;
    cfg.reject_halfcycle_slip = false;
    cfg.reject_outliers       = true;
    return cfg;
}

int main(int argc, char** argv) {
    // Parse flags before positional args
    char const* output_path = nullptr;
    char const* stat_path   = nullptr;
    bool        flag_gps = false, flag_glo = false, flag_gal = false, flag_bds = false;
    double      flag_snr = -1.0, flag_el = -1.0;

    for (int i = 1; i < argc;) {
        if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_path = argv[i + 1];
            for (int j = i; j < argc - 2; ++j)
                argv[j] = argv[j + 2];
            argc -= 2;
        } else if (strcmp(argv[i], "--stat") == 0 && i + 1 < argc) {
            stat_path = argv[i + 1];
            for (int j = i; j < argc - 2; ++j)
                argv[j] = argv[j + 2];
            argc -= 2;
        } else if (strcmp(argv[i], "--snr") == 0 && i + 1 < argc) {
            flag_el = std::stod(argv[i + 1]);
            for (int j = i; j < argc - 2; ++j)
                argv[j] = argv[j + 2];
            argc -= 2;
        } else if (strcmp(argv[i], "--gps") == 0) {
            flag_gps = true;
            for (int j = i; j < argc - 1; ++j)
                argv[j] = argv[j + 1];
            argc -= 1;
        } else if (strcmp(argv[i], "--glo") == 0) {
            flag_glo = true;
            for (int j = i; j < argc - 1; ++j)
                argv[j] = argv[j + 1];
            argc -= 1;
        } else if (strcmp(argv[i], "--gal") == 0) {
            flag_gal = true;
            for (int j = i; j < argc - 1; ++j)
                argv[j] = argv[j + 1];
            argc -= 1;
        } else if (strcmp(argv[i], "--bds") == 0) {
            flag_bds = true;
            for (int j = i; j < argc - 1; ++j)
                argv[j] = argv[j + 1];
            argc -= 1;
        } else {
            ++i;
        }
    }

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    double truth_x = 0.0, truth_y = 0.0, truth_z = 0.0;
    bool   has_truth = (argc >= 6);
    if (has_truth) {
        truth_x = std::stod(argv[3]);
        truth_y = std::stod(argv[4]);
        truth_z = std::stod(argv[5]);
    }

    FILE* out_file  = nullptr;
    FILE* stat_file = nullptr;
    if (output_path) {
        out_file = fopen(output_path, "w");
        if (!out_file) {
            fprintf(stderr, "Failed to open output file: %s\n", output_path);
            return 1;
        }
        fprintf(out_file, "time,sats,x,y,z,lat,lon,alt,clock");
        if (has_truth) fprintf(out_file, ",dx,dy,dz,d3");
        fprintf(out_file, "\n");
    }
    if (stat_path) {
        stat_file = fopen(stat_path, "w");
        if (!stat_file) {
            fprintf(stderr, "Failed to open stat file: %s\n", stat_path);
            return 1;
        }
    }

    loglet::initialize();
    loglet::set_level(loglet::Level::Warning);
    loglet::set_use_stderr(true);

    idokeido::EphemerisEngine eph_engine;
    idokeido::CorrectionCache correction_cache;
    auto                      cfg = default_config();
    if (flag_gps || flag_glo || flag_gal || flag_bds) {
        cfg.gnss.gps = flag_gps;
        cfg.gnss.glo = flag_glo;
        cfg.gnss.gal = flag_gal;
        cfg.gnss.bds = flag_bds;
    }
    if (flag_snr >= 0.0) cfg.snr_cutoff = flag_snr;
    if (flag_el >= 0.0) cfg.elevation_cutoff = flag_el;
    auto engine = idokeido::SppEngine(cfg, eph_engine, correction_cache);

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

    if (!format::rinex::parse_nav(argv[1], nav_cb)) {
        fprintf(stderr, "Failed to parse nav file: %s\n", argv[1]);
        return 1;
    }

    printf("%-25s  %5s", "time", "sats");
    if (has_truth) printf("  %8s %8s %8s %8s", "dx", "dy", "dz", "3d");
    printf("  %14s %14s %14s\n", "lat_deg", "lon_deg", "alt_m");

    struct Sample {
        double dx, dy, dz, de, dn, du, d3;
    };
    std::vector<Sample> samples;

    auto llh_to_enu_matrix = [](double x, double y, double z) {
        double r   = std::sqrt(x * x + y * y + z * z);
        double lat = std::asin(z / r);
        double lon = std::atan2(y, x);
        double sl = std::sin(lat), cl = std::cos(lat);
        double sn = std::sin(lon), cn = std::cos(lon);
        return std::array<std::array<double, 3>, 3>{
            {{{-sn, cn, 0.0}}, {{-sl * cn, -sl * sn, cl}}, {{cl * cn, cl * sn, sl}}}};
    };

    format::rinex::parse_obs(argv[2], [&](format::rinex::ObsEpoch const& epoch) {
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

            auto   R  = llh_to_enu_matrix(truth_x, truth_y, truth_z);
            double de = R[0][0] * dx + R[0][1] * dy + R[0][2] * dz;
            double dn = R[1][0] * dx + R[1][1] * dy + R[1][2] * dz;
            double du = R[2][0] * dx + R[2][1] * dy + R[2][2] * dz;
            samples.push_back({dx, dy, dz, de, dn, du, d3});
        }

        printf("  %14.9f %14.9f %14.3f\n", sol.latitude, sol.longitude, sol.altitude);

        if (out_file) {
            fprintf(out_file, "%s,%zu,%.4f,%.4f,%.4f,%.9f,%.9f,%.4f,%.4f",
                    epoch.time.rtklib_time_string(1).c_str(), sol.satellite_count,
                    sol.position_ecef.x(), sol.position_ecef.y(), sol.position_ecef.z(),
                    sol.latitude, sol.longitude, sol.altitude, sol.receiver_clock);
            if (has_truth) fprintf(out_file, ",%.4f,%.4f,%.4f,%.4f", dx, dy, dz, d3);
            fprintf(out_file, "\n");
        }

        if (stat_file) {
            // Extended $SAT lines: week, tow, sat, valid, az, el, residual, sx, sy, sz, clk, P
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
            double var = 0;
            double mn = get(samples[0]), mx = get(samples[0]);
            for (auto& s : samples) {
                double v = get(s);
                var += (v - mean) * (v - mean);
                mn = std::min(mn, v);
                mx = std::max(mx, v);
            }
            double sd = std::sqrt(var / nd);
            return std::array<double, 4>{mean, sd, mn, mx};
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
        e3d.reserve(n);
        for (auto& s : samples)
            e3d.push_back(s.d3);
        std::sort(e3d.begin(), e3d.end());
        double p95_3d = e3d[static_cast<size_t>(0.95 * static_cast<double>(n - 1))];

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
               2 * sd3, 3 * sd3, mn3, mx3, p95_3d);
    }

    return 0;
}
