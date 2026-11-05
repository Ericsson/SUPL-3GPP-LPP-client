/**
 * tokoro-post: Fast post-processing binary for SSR→VRS conversion.
 * Reads UBX tbin + SSR tbin + RINEX nav, produces RTCM output.
 * No streamline, no scheduler, no event queue — pure sequential processing.
 */
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include <cxx11_compat.hpp>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/lpp/uper_parser.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/glo.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/rinex/nav_reader.hpp>
#include <format/rtcm/1019.hpp>
#include <format/rtcm/1020.hpp>
#include <format/rtcm/1042.hpp>
#include <format/rtcm/1044.hpp>
#include <format/rtcm/1046.hpp>
#include <format/rtcm/ephemeris.hpp>
#include <format/rtcm/message.hpp>
#include <format/rtcm/parser.hpp>
#include <format/tbin/reader.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>
#include <format/ubx/parser.hpp>
#include <generator/rtcm/generator.hpp>
#include <generator/tokoro/generator.hpp>
#include <loglet/loglet.hpp>
#include <time/bdt.hpp>
#include <time/tai.hpp>
#include <time/utc.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <LPP-Message.h>
#include <constr_TYPE.h>
EXTERNAL_WARNINGS_POP

LOGLET_MODULE(tokoro_post);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(tokoro_post)

enum class GenStrategy {
    Assistance,
    TimeStep,
    TimeStepAligned,
    TimeStepLast,
};

struct Config {
    std::string              ubx_path;
    std::string              rtcm_path;
    std::string              ssr_path;
    std::string              output_path;
    std::string              diag_dir;
    std::string              antex_file;
    std::string              antenna_descriptor;
    std::string              receiver_descriptor;
    std::vector<std::string> nav_files;
    GenStrategy              gen_strategy   = GenStrategy::Assistance;
    double                   time_step      = 1.0;
    double                   ubx_shift      = -72000.0;
    double                   rtcm_shift     = 0.0;
    double                   stop_time      = 0.0;
    double                   max_orbit_age  = 15.0;
    double                   max_clock_age  = 15.0;
    double                   max_bias_age   = 15.0;
    double                   max_iono_age   = 90.0;
    double                   max_tropo_age  = 90.0;
    double                   elevation_mask = 10.0;
    double                   pos_x = 0, pos_y = 0, pos_z = 0;
    bool                     physical_pos_set = false;
    double                   physical_pos_x = 0, physical_pos_y = 0, physical_pos_z = 0;
    int                      eph_cache                  = 512;
    uint32_t                 msm_type                   = 5;
    uint32_t                 reference_station_id       = 1902;
    bool                     no_gps                     = false;
    bool                     no_gal                     = false;
    bool                     no_bds                     = false;
    bool                     no_qzs                     = false;
    bool                     antenna                    = false;  // enable ANTEX PCO/PCV correction
    bool                     iono_height_correction     = false;
    bool                     no_iod_consistency_check   = false;
    bool                     code_bias_optional         = false;
    bool                     phase_bias_optional        = false;
    bool                     tropo_optional             = false;
    bool                     iono_optional              = false;
    bool                     use_tropospheric_model     = false;
    bool                     no_shapiro_correction      = false;
    bool                     no_phase_windup_correction = false;
    bool                     no_earth_solid_tides_correction   = false;
    bool                     no_tropospheric_height_correction = false;
    bool                     no_tropo_hydrostatic_delta        = false;
    bool                     negative_phase_windup             = false;
    bool                     no_phase_alignment                = false;
    bool                     generate_rinex                    = false;
    bool                     generate_residuals                = false;
    bool                     ocit                              = false;
    bool                     rtoc                              = false;
    bool                     ignore_bitmask                    = false;
    bool                     iono_poly_only                    = false;
    std::string              orbit_frame;
    std::vector<std::string> orbit_source_frames;
    bool                     fake_cps_set       = false;
    uint16_t                 fake_cps_id        = 0;
    double                   fake_cps_lat       = 0;
    double                   fake_cps_lon       = 0;
    long                     fake_cps_steps_lat = 0;
    long                     fake_cps_steps_lon = 0;
    double                   fake_cps_step_lat  = 0;
    double                   fake_cps_step_lon  = 0;

    generator::tokoro::LockStart              lock_start = generator::tokoro::LockStart::Reset;
    generator::tokoro::LockStart              lock_rise  = generator::tokoro::LockStart::Reset;
    generator::tokoro::AmbiguityRandomization ambiguity_randomization =
        generator::tokoro::AmbiguityRandomization::Off;
    uint64_t ambiguity_seed     = 0;
    bool     ambiguity_seed_set = false;

    generator::tokoro::TroposphereModel tropospheric_model =
        generator::tokoro::TroposphereModel::Mops;
    generator::tokoro::MappingFunction tropospheric_mapping =
        generator::tokoro::MappingFunction::Niell;
    generator::tokoro::GradientSource  gradient_source = generator::tokoro::GradientSource::None;
    generator::tokoro::GradientMapping gradient_mapping =
        generator::tokoro::GradientMapping::ChenHerring;
    bool                     gradient_constant_set = false;
    double                   gradient_north        = 0.0;  // m
    double                   gradient_east         = 0.0;  // m
    bool                     diag_tropospheric_all = false;
    std::string              gpt2_grid;
    std::vector<std::string> vmf3_grids;
    std::string              vmf3_orography;
};

static Config parse_args(int argc, char** argv) {
    Config cfg;
    for (int i = 1; i < argc; i++) {
        std::string arg  = argv[i];
        auto        next = [&]() -> std::string {
            return (i + 1 < argc) ? argv[++i] : "";
        };
        if (arg == "--ubx")
            cfg.ubx_path = next();
        else if (arg == "--rtcm")
            cfg.rtcm_path = next();
        else if (arg == "--ssr")
            cfg.ssr_path = next();
        else if (arg == "--output" || arg == "-o")
            cfg.output_path = next();
        else if (arg == "--nav")
            cfg.nav_files.push_back(next());
        else if (arg == "--diag-dir")
            cfg.diag_dir = next();
        else if (arg == "--antex")
            cfg.antex_file = next();
        else if (arg == "--gen") {
            auto v = next();
            if (v == "assistance")
                cfg.gen_strategy = GenStrategy::Assistance;
            else if (v == "time-step")
                cfg.gen_strategy = GenStrategy::TimeStep;
            else if (v == "time-step-aligned")
                cfg.gen_strategy = GenStrategy::TimeStepAligned;
            else if (v == "time-step-last")
                cfg.gen_strategy = GenStrategy::TimeStepLast;
            else {
                fprintf(stderr, "Invalid --gen value: %s\n", v.c_str());
                exit(1);
            }
        } else if (arg == "--time-step")
            cfg.time_step = std::stod(next());
        else if (arg == "--ubx-shift")
            cfg.ubx_shift = std::stod(next());
        else if (arg == "--rtcm-shift")
            cfg.rtcm_shift = std::stod(next());
        else if (arg == "--stop-time")
            cfg.stop_time = std::stod(next());
        else if (arg == "--max-age") {
            double v          = std::stod(next());
            cfg.max_orbit_age = v;
            cfg.max_clock_age = v;
            cfg.max_bias_age  = v;
            cfg.max_iono_age  = v;
            cfg.max_tropo_age = v;
        } else if (arg == "--max-orbit-age")
            cfg.max_orbit_age = std::stod(next());
        else if (arg == "--max-clock-age")
            cfg.max_clock_age = std::stod(next());
        else if (arg == "--max-bias-age")
            cfg.max_bias_age = std::stod(next());
        else if (arg == "--max-iono-age")
            cfg.max_iono_age = std::stod(next());
        else if (arg == "--max-tropo-age")
            cfg.max_tropo_age = std::stod(next());
        else if (arg == "--elevation-mask")
            cfg.elevation_mask = std::stod(next());
        else if (arg == "--pos-x")
            cfg.pos_x = std::stod(next());
        else if (arg == "--pos-y")
            cfg.pos_y = std::stod(next());
        else if (arg == "--pos-z")
            cfg.pos_z = std::stod(next());
        else if (arg == "--eph-cache")
            cfg.eph_cache = std::stoi(next());
        else if (arg == "--no-gps")
            cfg.no_gps = true;
        else if (arg == "--no-gal")
            cfg.no_gal = true;
        else if (arg == "--no-bds")
            cfg.no_bds = true;
        else if (arg == "--no-qzs")
            cfg.no_qzs = true;
        else if (arg == "--antenna" || arg == "--antenna-pcv")
            cfg.antenna = true;
        else if (arg == "--use-ionospheric-height-correction")
            cfg.iono_height_correction = true;
        else if (arg == "--no-iod-consistency-check")
            cfg.no_iod_consistency_check = true;
        else if (arg == "--code-bias-optional")
            cfg.code_bias_optional = true;
        else if (arg == "--phase-bias-optional")
            cfg.phase_bias_optional = true;
        else if (arg == "--tropo-optional")
            cfg.tropo_optional = true;
        else if (arg == "--iono-optional")
            cfg.iono_optional = true;
        else if (arg == "--use-tropospheric-model")
            cfg.use_tropospheric_model = true;
        else if (arg == "--no-shapiro-correction")
            cfg.no_shapiro_correction = true;
        else if (arg == "--no-phase-windup-correction")
            cfg.no_phase_windup_correction = true;
        else if (arg == "--no-earth-solid-tides-correction")
            cfg.no_earth_solid_tides_correction = true;
        else if (arg == "--no-tropospheric-height-correction")
            cfg.no_tropospheric_height_correction = true;
        else if (arg == "--no-tropo-hydrostatic-delta")
            cfg.no_tropo_hydrostatic_delta = true;
        else if (arg == "--msm-type")
            cfg.msm_type = static_cast<uint32_t>(std::stoul(next()));
        else if (arg == "--negative-phase-windup")
            cfg.negative_phase_windup = true;
        else if (arg == "--no-phase-alignment")
            cfg.no_phase_alignment = true;
        else if (arg == "--lock-start" || arg == "--lock-rise") {
            auto value = next();
            auto mode  = generator::tokoro::LockStart::Reset;
            if (value == "max") {
                mode = generator::tokoro::LockStart::Max;
            } else if (value != "reset") {
                fprintf(stderr, "%s must be 'reset' or 'max', got `%s`\n", arg.c_str(),
                        value.c_str());
                exit(1);
            }
            (arg == "--lock-start" ? cfg.lock_start : cfg.lock_rise) = mode;
        } else if (arg == "--ambiguity-randomize") {
            auto value = next();
            if (value == "off") {
                cfg.ambiguity_randomization = generator::tokoro::AmbiguityRandomization::Off;
            } else if (value == "startup") {
                cfg.ambiguity_randomization = generator::tokoro::AmbiguityRandomization::Startup;
            } else if (value == "rise") {
                cfg.ambiguity_randomization = generator::tokoro::AmbiguityRandomization::Rise;
            } else if (value == "both") {
                cfg.ambiguity_randomization = generator::tokoro::AmbiguityRandomization::Both;
            } else {
                fprintf(stderr,
                        "--ambiguity-randomize must be 'off', 'startup', 'rise', or 'both', got "
                        "`%s`\n",
                        value.c_str());
                exit(1);
            }
        } else if (arg == "--tropo-model") {
            auto value = next();
            if (!generator::tokoro::parse_troposphere_model(value, cfg.tropospheric_model)) {
                fprintf(stderr, "--tropo-model must be one of %s, got `%s`\n",
                        generator::tokoro::troposphere_model_names().c_str(), value.c_str());
                exit(1);
            }
        } else if (arg == "--tropo-mapping") {
            auto value = next();
            if (!generator::tokoro::parse_mapping_function(value, cfg.tropospheric_mapping)) {
                fprintf(stderr, "--tropo-mapping must be one of %s, got `%s`\n",
                        generator::tokoro::mapping_function_names().c_str(), value.c_str());
                exit(1);
            }
        } else if (arg == "--tropo-gradient") {
            auto value = next();
            if (!generator::tokoro::parse_gradient_source(value, cfg.gradient_source)) {
                fprintf(stderr, "--tropo-gradient must be one of %s, got `%s`\n",
                        generator::tokoro::gradient_source_names().c_str(), value.c_str());
                exit(1);
            }
        } else if (arg == "--tropo-gradient-mapping") {
            auto value = next();
            if (!generator::tokoro::parse_gradient_mapping(value, cfg.gradient_mapping)) {
                fprintf(stderr, "--tropo-gradient-mapping must be one of %s, got `%s`\n",
                        generator::tokoro::gradient_mapping_names().c_str(), value.c_str());
                exit(1);
            }
        } else if (arg == "--tropo-gradient-north") {
            cfg.gradient_north        = std::stod(next()) * 1e-3;
            cfg.gradient_constant_set = true;
        } else if (arg == "--tropo-gradient-east") {
            cfg.gradient_east         = std::stod(next()) * 1e-3;
            cfg.gradient_constant_set = true;
        } else if (arg == "--diag-tropo-all") {
            cfg.diag_tropospheric_all = true;
        } else if (arg == "--gpt2-grid") {
            cfg.gpt2_grid = next();
        } else if (arg == "--vmf3-grid") {
            cfg.vmf3_grids.push_back(next());
        } else if (arg == "--vmf3-orography") {
            cfg.vmf3_orography = next();
        } else if (arg == "--ambiguity-seed") {
            cfg.ambiguity_seed     = std::stoull(next());
            cfg.ambiguity_seed_set = true;
        } else if (arg == "--antenna-descriptor")
            cfg.antenna_descriptor = next();
        else if (arg == "--receiver-descriptor")
            cfg.receiver_descriptor = next();
        else if (arg == "--generate-rinex")
            cfg.generate_rinex = true;
        else if (arg == "--residuals")
            cfg.generate_residuals = true;
        else if (arg == "--reference-station-id")
            cfg.reference_station_id = static_cast<uint32_t>(std::stoul(next()));
        else if (arg == "--physical-x") {
            cfg.physical_pos_x   = std::stod(next());
            cfg.physical_pos_set = true;
        } else if (arg == "--physical-y") {
            cfg.physical_pos_y   = std::stod(next());
            cfg.physical_pos_set = true;
        } else if (arg == "--physical-z") {
            cfg.physical_pos_z   = std::stod(next());
            cfg.physical_pos_set = true;
        } else if (arg == "--ocit")
            cfg.ocit = true;
        else if (arg == "--rtoc")
            cfg.rtoc = true;
        else if (arg == "--ignore-bitmask")
            cfg.ignore_bitmask = true;
        else if (arg == "--iono-poly-only")
            cfg.iono_poly_only = true;
        else if (arg == "--orbit-frame")
            cfg.orbit_frame = next();
        else if (arg == "--orbit-source-frame")
            cfg.orbit_source_frames.push_back(next());
        else if (arg == "--cps-id") {
            cfg.fake_cps_id  = static_cast<uint16_t>(std::stoi(next()));
            cfg.fake_cps_set = true;
        } else if (arg == "--cps-lat") {
            cfg.fake_cps_lat = std::stod(next());
            cfg.fake_cps_set = true;
        } else if (arg == "--cps-lon") {
            cfg.fake_cps_lon = std::stod(next());
            cfg.fake_cps_set = true;
        } else if (arg == "--cps-steps-lat") {
            cfg.fake_cps_steps_lat = std::stol(next());
            cfg.fake_cps_set       = true;
        } else if (arg == "--cps-steps-lon") {
            cfg.fake_cps_steps_lon = std::stol(next());
            cfg.fake_cps_set       = true;
        } else if (arg == "--cps-step-lat") {
            cfg.fake_cps_step_lat = std::stod(next());
            cfg.fake_cps_set      = true;
        } else if (arg == "--cps-step-lon") {
            cfg.fake_cps_step_lon = std::stod(next());
            cfg.fake_cps_set      = true;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", arg.c_str());
            exit(1);
        }
    }
    if (cfg.time_step <= 0.0) {
        fprintf(stderr, "--time-step must be greater than zero\n");
        exit(1);
    }
    {
        auto needs_gpt2 = cfg.tropospheric_model == generator::tokoro::TroposphereModel::Gpt2 ||
                          cfg.tropospheric_mapping == generator::tokoro::MappingFunction::Gpt2;
        auto needs_vmf3 = cfg.tropospheric_model == generator::tokoro::TroposphereModel::Vmf3 ||
                          cfg.tropospheric_mapping == generator::tokoro::MappingFunction::Vmf3;
        if (needs_gpt2 && cfg.gpt2_grid.empty()) {
            fprintf(stderr, "the tropospheric model or mapping function `gpt2` requires "
                            "--gpt2-grid\n");
            exit(1);
        }
        if (needs_vmf3 && (cfg.vmf3_grids.empty() || cfg.vmf3_orography.empty())) {
            fprintf(stderr, "the tropospheric model or mapping function `vmf3` requires "
                            "--vmf3-grid and --vmf3-orography\n");
            exit(1);
        }
        if (cfg.gradient_source == generator::tokoro::GradientSource::Constant &&
            !cfg.gradient_constant_set) {
            fprintf(stderr, "--tropo-gradient constant requires --tropo-gradient-north and/or "
                            "--tropo-gradient-east\n");
            exit(1);
        }
        if (cfg.diag_tropospheric_all && cfg.diag_dir.empty()) {
            fprintf(stderr, "--diag-tropo-all requires --diag-dir\n");
            exit(1);
        }
    }
    if (cfg.ssr_path.empty() || cfg.output_path.empty()) {
        fprintf(stderr, "Usage: tokoro-post --ssr <tbin> --output <rtcm> --pos-x X --pos-y Y "
                        "--pos-z Z [--ubx <tbin>] [--rtcm <tbin>] [--nav <file>...] "
                        "[--elevation-mask <degrees>] [--msm-type 4|5|6|7] "
                        "[--reference-station-id <id>] [--physical-x/y/z <meter>] "
                        "[--no-gps|--no-gal|--no-bds|--no-qzs] "
                        "[--code-bias-optional] [--phase-bias-optional] [--tropo-optional] "
                        "[--iono-optional] [--use-tropospheric-model] "
                        "[--no-shapiro-correction] [--no-phase-windup-correction] "
                        "[--no-earth-solid-tides-correction] [--no-tropospheric-height-correction] "
                        "[--no-tropo-hydrostatic-delta] [--negative-phase-windup] "
                        "[--no-phase-alignment] [--lock-start reset|max] "
                        "[--lock-rise reset|max] "
                        "[--tropo-model mops|saastamoinen|saastamoinen-gpt|gpt2|vmf3] "
                        "[--tropo-mapping niell|simple|gmf|gpt2|vmf3] [--diag-tropo-all] "
                        "[--tropo-gradient none|ssr-grid|constant] "
                        "[--tropo-gradient-mapping chen-herring|macmillan] "
                        "[--tropo-gradient-north <mm>] [--tropo-gradient-east <mm>] "
                        "[--gpt2-grid <file>] [--vmf3-grid <file>...] [--vmf3-orography <file>] "
                        "[--ambiguity-randomize off|startup|rise|both] [--ambiguity-seed <n>] "
                        "[--antenna-descriptor <string>] "
                        "[--receiver-descriptor <string>] [--generate-rinex] [--residuals] "
                        "[--ocit] [--rtoc] [--ignore-bitmask] [--iono-poly-only] [--orbit-frame "
                        "<frame>] [--orbit-source-frame <gnss>=<frame>] "
                        "[--gen assistance|time-step|time-step-aligned|time-step-last] "
                        "[--time-step <seconds>]\n");
        exit(1);
    }
    return cfg;
}

// Merge heap entry
struct MergeEntry {
    int64_t  timestamp_us;
    uint32_t source;  // 0=ubx, 1=ssr, 2=rtcm
    bool     operator>(MergeEntry const& o) const { return timestamp_us > o.timestamp_us; }
};

static ts::Tai unix_us_to_tai(int64_t us) {
    auto seconds  = us / 1000000LL;
    auto fraction = static_cast<double>(us % 1000000LL) / 1e6;
    return ts::Tai{ts::Utc{ts::Timestamp{seconds, fraction}}};
}

static int64_t tai_to_us(ts::Tai const& tai) {
    auto tm = tai.timestamp();
    return tm.seconds() * 1000000LL + static_cast<int64_t>(std::llround(tm.fraction() * 1e6));
}

static void rtcm_to_ephemeris(generator::tokoro::Generator& generator,
                              format::rtcm::Rtcm1019 const* rtcm) {
    generator.process_ephemeris(format::rtcm::to_ephemeris(*rtcm));
}

static void rtcm_to_ephemeris(generator::tokoro::Generator& generator,
                              format::rtcm::Rtcm1042 const* rtcm) {
    generator.process_ephemeris(format::rtcm::to_ephemeris(*rtcm));
}

static void rtcm_to_ephemeris(generator::tokoro::Generator& generator,
                              format::rtcm::Rtcm1046 const* rtcm) {
    generator.process_ephemeris(format::rtcm::to_ephemeris(*rtcm));
}

static void process_rtcm_ephemeris(generator::tokoro::Generator& generator,
                                   format::rtcm::Message const* message, ts::Tai const& time) {
    if (!message) return;
    if (auto rtcm1019 = dynamic_cast<format::rtcm::Rtcm1019 const*>(message)) {
        rtcm_to_ephemeris(generator, rtcm1019);
    } else if (auto rtcm1042 = dynamic_cast<format::rtcm::Rtcm1042 const*>(message)) {
        rtcm_to_ephemeris(generator, rtcm1042);
    } else if (auto rtcm1046 = dynamic_cast<format::rtcm::Rtcm1046 const*>(message)) {
        rtcm_to_ephemeris(generator, rtcm1046);
    } else if (auto rtcm1020 = dynamic_cast<format::rtcm::Rtcm1020 const*>(message)) {
        generator.process_ephemeris(format::rtcm::to_ephemeris(*rtcm1020, time));
    } else if (auto rtcm1044 = dynamic_cast<format::rtcm::Rtcm1044 const*>(message)) {
        generator.process_ephemeris(format::rtcm::to_ephemeris(*rtcm1044));
    }
}

static void process_ubx_ephemeris(generator::tokoro::Generator&               generator,
                                  format::nav::gps::lnav::EphemerisCollector& gps_collector,
                                  format::nav::gal::InavEphemerisCollector&   gal_collector,
                                  format::nav::D1Collector&                   bds_collector,
                                  format::nav::gps::lnav::EphemerisCollector& qzs_collector,
                                  format::nav::GloEphemerisCollector&         glo_collector,
                                  format::ubx::RxmSfrbx* sfrbx, ts::Tai const& time) {
    if (!sfrbx) return;
    if (sfrbx->gnss_id() == 0) {
        // GPS
        if (sfrbx->sig_id() != 0) return;
        auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());
        format::nav::gps::lnav::Subframe subframe{};
        if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) return;
        ephemeris::GpsEphemeris ephemeris{};
        if (!gps_collector.process(sfrbx->sv_id(), subframe, ephemeris)) return;
        generator.process_ephemeris(ephemeris);
    } else if (sfrbx->gnss_id() == 2) {
        // Galileo — sig_id 1 = E1-B I/NAV (ZED-X20P), sig_id 5 = E1-B I/NAV (older receivers)
        if (sfrbx->sig_id() != 1 && sfrbx->sig_id() != 5) return;
        auto                       words = format::nav::Words::from_sfrbx_e5b(sfrbx->words());
        format::nav::gal::InavWord word{};
        if (!format::nav::gal::InavWord::decode(words, word)) return;
        ephemeris::GalEphemeris ephemeris{};
        if (!gal_collector.process(sfrbx->sv_id(), word, ephemeris)) return;
        generator.process_ephemeris(ephemeris);
    } else if (sfrbx->gnss_id() == 3) {
        // BeiDou D1
        if (sfrbx->sig_id() != 0) return;
        auto                    words = format::nav::Words::from_sfrbx_bds_d1(sfrbx->words());
        format::nav::D1Subframe subframe{};
        if (!format::nav::D1Subframe::decode(words, sfrbx->sv_id(), subframe)) return;
        ephemeris::BdsEphemeris ephemeris{};
        if (!bds_collector.process(sfrbx->sv_id(), subframe, ephemeris)) return;
        generator.process_ephemeris(ephemeris);
    } else if (sfrbx->gnss_id() == 5) {
        // QZSS L1 C/A LNAV, the GPS LNAV layout
        if (sfrbx->sig_id() != 0) return;
        auto words = format::nav::Words::from_sfrbx_l1ca(sfrbx->words());
        format::nav::gps::lnav::Subframe subframe{};
        if (!format::nav::gps::lnav::Subframe::decode(words, subframe)) return;
        auto                    prn = static_cast<uint8_t>(192 + sfrbx->sv_id());
        ephemeris::GpsEphemeris gps{};
        if (!qzs_collector.process(prn, subframe, gps)) return;
        gps.prn = prn;
        generator.process_ephemeris(ephemeris::QzsEphemeris::from_gps_layout(gps));
    } else if (sfrbx->gnss_id() == 6) {
        // GLONASS L1OF/L2OF strings
        if (sfrbx->words().size() < 4) return;
        if (sfrbx->sv_id() < 1 || sfrbx->sv_id() > 24) return;
        auto string  = format::nav::GloString::from_sfrbx(sfrbx->words().data());
        auto channel = static_cast<int8_t>(static_cast<int>(sfrbx->freq_id()) - 7);
        ephemeris::GloEphemeris ephemeris{};
        if (!glo_collector.process(sfrbx->sv_id(), channel, string, time, ephemeris)) return;
        generator.process_ephemeris(ephemeris);
    }
}

int main(int argc, char** argv) {
    auto cfg = parse_args(argc, argv);

    // Open tbin readers
    format::tbin::Reader ubx_reader, ssr_reader, rtcm_reader;
    bool                 has_ubx  = !cfg.ubx_path.empty();
    bool                 has_rtcm = !cfg.rtcm_path.empty();
    if (has_ubx && !ubx_reader.open(cfg.ubx_path)) {
        fprintf(stderr, "Cannot open %s\n", cfg.ubx_path.c_str());
        return 1;
    }
    if (has_rtcm && !rtcm_reader.open(cfg.rtcm_path)) {
        fprintf(stderr, "Cannot open %s\n", cfg.rtcm_path.c_str());
        return 1;
    }
    if (!ssr_reader.open(cfg.ssr_path)) {
        fprintf(stderr, "Cannot open %s\n", cfg.ssr_path.c_str());
        return 1;
    }

    // Setup generator
    auto generator = std::make_unique<generator::tokoro::Generator>();
    generator->set_iod_consistency_check(!cfg.no_iod_consistency_check);
    generator->set_ephemeris_max_cache(static_cast<size_t>(cfg.eph_cache));
    generator->set_max_orbit_age(cfg.max_orbit_age);
    generator->set_max_clock_age(cfg.max_clock_age);
    generator->set_max_bias_age(cfg.max_bias_age);
    generator->set_max_iono_age(cfg.max_iono_age);
    generator->set_max_tropo_age(cfg.max_tropo_age);
    generator->set_ocit(cfg.ocit);
    generator->set_rtoc(cfg.rtoc);
    generator->set_ignore_bitmask(cfg.ignore_bitmask);
    generator->set_iono_polynomial_only(cfg.iono_poly_only);
    if (!cfg.orbit_frame.empty()) {
        if (!generator->set_orbit_frame(cfg.orbit_frame.c_str())) {
            fprintf(stderr, "Invalid --orbit-frame %s\n", cfg.orbit_frame.c_str());
            return 1;
        }
        for (auto const& value : cfg.orbit_source_frames) {
            auto              separator = value.find('=');
            SatelliteId::Gnss gnss{};
            if (separator == std::string::npos ||
                !generator::tokoro::gnss_from_name(value.substr(0, separator).c_str(), gnss) ||
                !generator->set_orbit_source_frame(gnss, value.substr(separator + 1).c_str())) {
                fprintf(stderr, "Invalid --orbit-source-frame %s\n", value.c_str());
                return 1;
            }
        }
    } else if (!cfg.orbit_source_frames.empty()) {
        fprintf(stderr, "--orbit-source-frame requires --orbit-frame\n");
        return 1;
    }
    if (cfg.fake_cps_set) {
        generator->set_fake_correction_point_set(
            cfg.fake_cps_id, cfg.fake_cps_lat, cfg.fake_cps_lon, cfg.fake_cps_steps_lat,
            cfg.fake_cps_steps_lon, cfg.fake_cps_step_lat, cfg.fake_cps_step_lon);
    }

    // Load nav files
    for (auto& nav_path : cfg.nav_files) {
        auto nav = format::rinex::parse_nav_file(nav_path);
        for (auto& eph : nav.gps)
            generator->process_ephemeris(eph);
        for (auto& eph : nav.gal)
            generator->process_ephemeris(eph);
        for (auto& eph : nav.bds)
            generator->process_ephemeris(eph);
        for (auto& eph : nav.glo)
            generator->process_ephemeris(eph);
        for (auto& eph : nav.qzs)
            generator->process_ephemeris(eph);
    }

    // Load antex
#ifdef INCLUDE_FORMAT_ANTEX
    if (!cfg.gpt2_grid.empty() && !generator->troposphere_data().load_gpt2_grid(cfg.gpt2_grid)) {
        fprintf(stderr, "Cannot load GPT2 grid file %s\n", cfg.gpt2_grid.c_str());
        return 1;
    }
    for (auto const& file : cfg.vmf3_grids) {
        if (!generator->troposphere_data().load_vmf3_grid(file)) {
            fprintf(stderr, "Cannot load VMF3 grid file %s\n", file.c_str());
            return 1;
        }
    }
    if (!cfg.vmf3_orography.empty() &&
        !generator->troposphere_data().load_vmf3_orography(cfg.vmf3_orography)) {
        fprintf(stderr, "Cannot load VMF3 orography file %s\n", cfg.vmf3_orography.c_str());
        return 1;
    }

    if (!cfg.antex_file.empty()) {
        auto antex = format::antex::Antex::from_file(cfg.antex_file);
        if (antex) generator->set_antex(std::move(antex));
    }
#endif

    // Setup reference station
    generator::tokoro::ReferenceStationConfig rs_cfg{};
    rs_cfg.itrf_ground_position = {cfg.pos_x, cfg.pos_y, cfg.pos_z};
    rs_cfg.rtcm_ground_position = {cfg.pos_x, cfg.pos_y, cfg.pos_z};
    rs_cfg.generate_gps         = !cfg.no_gps;
    rs_cfg.generate_gal         = !cfg.no_gal;
    rs_cfg.generate_bds         = !cfg.no_bds;
    rs_cfg.generate_glo         = false;
    rs_cfg.generate_qzs         = !cfg.no_qzs;
    auto ref_station = std::make_shared<generator::tokoro::ReferenceStation>(*generator, rs_cfg);
    ref_station->set_msm_type(cfg.msm_type);
    ref_station->set_reference_station_id(cfg.reference_station_id);
    ref_station->set_phase_alignment(!cfg.no_phase_alignment);
    ref_station->set_shapiro_correction(!cfg.no_shapiro_correction);
    ref_station->set_earth_solid_tides_correction(!cfg.no_earth_solid_tides_correction);
    ref_station->set_phase_windup_correction(!cfg.no_phase_windup_correction);
    ref_station->set_tropospheric_height_correction(!cfg.no_tropospheric_height_correction);
    ref_station->set_tropospheric_hydrostatic_delta(!cfg.no_tropo_hydrostatic_delta);
    ref_station->set_antenna_phase_variation_correction(cfg.antenna);
    ref_station->set_use_ionospheric_height_correction(cfg.iono_height_correction);
    ref_station->set_elevation_mask(cfg.elevation_mask);
    ref_station->set_negative_phase_windup(cfg.negative_phase_windup);
    ref_station->set_lock_start(cfg.lock_start);
    ref_station->set_lock_rise(cfg.lock_rise);
    ref_station->set_ambiguity_randomization(cfg.ambiguity_randomization);
    {
        auto seed = cfg.ambiguity_seed;
        if (!cfg.ambiguity_seed_set &&
            cfg.ambiguity_randomization != generator::tokoro::AmbiguityRandomization::Off) {
            std::random_device device;
            seed = (static_cast<uint64_t>(device()) << 32) ^ device();
            INFOF("ambiguity seed: %llu", static_cast<unsigned long long>(seed));
        }
        ref_station->set_ambiguity_seed(seed);
    }
    ref_station->set_antenna_descriptor(cfg.antenna_descriptor);
    ref_station->set_receiver_descriptor(cfg.receiver_descriptor);
    ref_station->set_require_code_bias(!cfg.code_bias_optional);
    ref_station->set_require_phase_bias(!cfg.phase_bias_optional);
    ref_station->set_require_tropo(!cfg.tropo_optional);
    ref_station->set_require_iono(!cfg.iono_optional);
    ref_station->set_use_tropospheric_model(cfg.use_tropospheric_model);
    ref_station->set_generate_residuals(cfg.generate_residuals);
    if (cfg.physical_pos_set) {
        ref_station->set_physical_ground_position(
            {cfg.physical_pos_x, cfg.physical_pos_y, cfg.physical_pos_z});
    }
#ifdef INCLUDE_FORMAT_RINEX
    ref_station->set_generate_rinex(cfg.generate_rinex);
#endif
    ref_station->set_tropospheric_model(cfg.tropospheric_model);
    ref_station->set_tropospheric_mapping(cfg.tropospheric_mapping);
    ref_station->set_tropospheric_gradient(cfg.gradient_source, cfg.gradient_mapping);
    if (cfg.gradient_constant_set) {
        ref_station->set_tropospheric_gradient_constant(cfg.gradient_north, cfg.gradient_east);
    }
    if (!cfg.diag_dir.empty()) {
        ref_station->set_diag_output(cfg.diag_dir);
        ref_station->set_diag_tropospheric_all(cfg.diag_tropospheric_all);
    }

    // Open output
    FILE* out_file = fopen(cfg.output_path.c_str(), "wb");
    if (!out_file) {
        fprintf(stderr, "Cannot open output %s\n", cfg.output_path.c_str());
        return 1;
    }

    // Setup parsers
    format::ubx::Parser                        ubx_parser;
    format::rtcm::Parser                       rtcm_parser;
    format::lpp::UperParser                    lpp_parser;
    format::nav::gps::lnav::EphemerisCollector gps_collector;
    format::nav::gal::InavEphemerisCollector   gal_collector;
    format::nav::D1Collector                   bds_collector;
    format::nav::gps::lnav::EphemerisCollector qzs_collector;
    format::nav::GloEphemerisCollector         glo_collector;

    // Merge heap
    std::priority_queue<MergeEntry, std::vector<MergeEntry>, std::greater<MergeEntry>> heap;
    format::tbin::Message ubx_msg, ssr_msg, rtcm_msg;
    int64_t               shift_us      = static_cast<int64_t>(cfg.ubx_shift * 1000000.0);
    int64_t               rtcm_shift_us = static_cast<int64_t>(cfg.rtcm_shift * 1000000.0);
    int64_t               stop_us =
        cfg.stop_time > 0 ? static_cast<int64_t>(cfg.stop_time * 1000000.0) : INT64_MAX;

    if (has_ubx && ubx_reader.next(ubx_msg)) heap.push({ubx_msg.timestamp_us + shift_us, 0});
    if (ssr_reader.next(ssr_msg)) heap.push({ssr_msg.timestamp_us, 1});
    if (has_rtcm && rtcm_reader.next(rtcm_msg))
        heap.push({rtcm_msg.timestamp_us + rtcm_shift_us, 2});

    // Processing state
    ts::Tai last_gen_time;
    int64_t last_log_us   = 0;
    int64_t first_data_us = 0;
    size_t  msg_count     = 0;
    auto    wall_start    = std::chrono::steady_clock::now();

    auto generate_at = [&](ts::Tai const& gen_time) {
        ref_station->generate(gen_time);
        auto messages = ref_station->produce();
        for (auto& m : messages) {
            fwrite(m.data().data(), 1, m.data().size(), out_file);
        }
        msg_count += messages.size();
    };

    bool    time_step_mode = cfg.gen_strategy != GenStrategy::Assistance;
    int64_t step_us        = std::max<int64_t>(1, std::llround(cfg.time_step * 1e6));
    bool    tick_started   = false;
    int64_t next_tick_us   = 0;

    // Emit every time step tick that has elapsed up to the given data time.
    auto emit_ticks = [&](int64_t unix_us) {
        if (generator->last_correction_data_time().timestamp().seconds() == 0) return;

        auto now_us = tai_to_us(unix_us_to_tai(unix_us));
        if (!tick_started) {
            tick_started = true;
            next_tick_us = now_us;
            if (cfg.gen_strategy == GenStrategy::TimeStepAligned) {
                next_tick_us = ((now_us + step_us - 1) / step_us) * step_us;
            }
        }

        while (next_tick_us <= now_us) {
            if (cfg.gen_strategy == GenStrategy::TimeStepLast) {
                generate_at(generator->last_correction_data_time());
            } else {
                auto tick = ts::Tai{ts::Timestamp{
                    next_tick_us / 1000000LL, static_cast<double>(next_tick_us % 1000000LL) / 1e6}};
                generate_at(tick);
            }
            next_tick_us += step_us;
        }
    };

    // Main loop
    while (!heap.empty()) {
        auto top = heap.top();
        heap.pop();

        if (top.timestamp_us > stop_us) break;

        if (time_step_mode) emit_ticks(top.timestamp_us);

        if (first_data_us == 0) first_data_us = top.timestamp_us;

        // Progress log every 60s of data
        if (top.timestamp_us - last_log_us >= 60000000LL) {
            auto t = static_cast<time_t>(top.timestamp_us / 1000000LL);
            char buf[32];
            strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));

            auto   wall_now     = std::chrono::steady_clock::now();
            double wall_elapsed = std::chrono::duration<double>(wall_now - wall_start).count();
            double data_elapsed = (top.timestamp_us - first_data_us) / 1e6;
            double data_total   = (stop_us < INT64_MAX) ? (stop_us - first_data_us) / 1e6 : 0;
            double eta_s        = 0;
            if (data_elapsed > 0 && data_total > 0) {
                double rate = wall_elapsed / data_elapsed;
                eta_s       = (data_total - data_elapsed) * rate;
            }

            if (eta_s > 0)
                fprintf(stderr, "\r[tokoro-post] %s (%zu msgs, ETA %.0fs)    ", buf, msg_count,
                        eta_s);
            else
                fprintf(stderr, "\r[tokoro-post] %s (%zu msgs, %.1fx realtime)    ", buf, msg_count,
                        wall_elapsed > 0 ? data_elapsed / wall_elapsed : 0.0);
            last_log_us = top.timestamp_us;
        }

        if (top.source == 0) {
            // UBX message — feed to parser for ephemeris extraction
            ubx_parser.append(ubx_msg.data.data(), ubx_msg.data.size());
            while (auto msg = ubx_parser.try_parse()) {
                auto sfrbx = dynamic_cast<format::ubx::RxmSfrbx*>(msg.get());
                if (sfrbx) {
                    process_ubx_ephemeris(*generator, gps_collector, gal_collector, bds_collector,
                                          qzs_collector, glo_collector, sfrbx,
                                          unix_us_to_tai(top.timestamp_us));
                }
            }
            if (has_ubx && ubx_reader.next(ubx_msg))
                heap.push({ubx_msg.timestamp_us + shift_us, 0});
        } else if (top.source == 2) {
            // RTCM message — feed to parser for ephemeris extraction
            rtcm_parser.append(rtcm_msg.data.data(), rtcm_msg.data.size());
            while (auto msg = rtcm_parser.try_parse()) {
                process_rtcm_ephemeris(*generator, msg.get(), unix_us_to_tai(top.timestamp_us));
            }
            if (has_rtcm && rtcm_reader.next(rtcm_msg))
                heap.push({rtcm_msg.timestamp_us + rtcm_shift_us, 2});
        } else {
            // SSR LPP message
            lpp_parser.append(ssr_msg.data.data(), ssr_msg.data.size());
            while (auto lpp_msg = lpp_parser.try_parse()) {
                auto new_data = generator->process_lpp(*lpp_msg);
                if (new_data && !time_step_mode) {
                    auto gen_time = generator->last_correction_data_time();
                    if (gen_time != last_gen_time) {
                        last_gen_time = gen_time;
                        generate_at(gen_time);
                    }
                }
                ASN_STRUCT_FREE(asn_DEF_LPP_Message, lpp_msg);
            }
            if (ssr_reader.next(ssr_msg)) heap.push({ssr_msg.timestamp_us, 1});
        }
    }

    fprintf(stderr, "\n[tokoro-post] Done: %zu RTCM messages written\n", msg_count);
    fclose(out_file);
    return 0;
}
