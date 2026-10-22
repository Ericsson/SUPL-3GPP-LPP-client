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
#include <string>
#include <vector>

#include <cxx11_compat.hpp>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/lpp/uper_parser.hpp>
#include <format/nav/d1.hpp>
#include <format/nav/gal/inav.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/rinex/nav_reader.hpp>
#include <format/rtcm/1019.hpp>
#include <format/rtcm/1042.hpp>
#include <format/rtcm/1046.hpp>
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
    bool                     fake_cps_set                      = false;
    uint16_t                 fake_cps_id                       = 0;
    double                   fake_cps_lat                      = 0;
    double                   fake_cps_lon                      = 0;
    long                     fake_cps_steps_lat                = 0;
    long                     fake_cps_steps_lon                = 0;
    double                   fake_cps_step_lat                 = 0;
    double                   fake_cps_step_lon                 = 0;
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
        else if (arg == "--antenna-descriptor")
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
                        "[--no-phase-alignment] [--antenna-descriptor <string>] "
                        "[--receiver-descriptor <string>] [--generate-rinex] [--residuals] "
                        "[--ocit] [--rtoc] [--ignore-bitmask] "
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
    ephemeris::GpsEphemeris ephemeris{};
    ephemeris.prn               = rtcm->prn;
    ephemeris.week_number       = rtcm->week;
    ephemeris.ca_or_p_on_l2     = rtcm->code_on_l2;
    ephemeris.ura_index         = rtcm->sv_accuracy;
    ephemeris.sv_health         = rtcm->sv_health;
    ephemeris.lpp_iod           = rtcm->iodc;
    ephemeris.iodc              = rtcm->iodc;
    ephemeris.iode              = rtcm->iode;
    ephemeris.aodo              = 0;
    ephemeris.toc               = rtcm->t_oc;
    ephemeris.toe               = rtcm->t_oe;
    ephemeris.tgd               = rtcm->t_gd;
    ephemeris.af2               = rtcm->a_f2;
    ephemeris.af1               = rtcm->a_f1;
    ephemeris.af0               = rtcm->a_f0;
    ephemeris.crc               = rtcm->c_rc;
    ephemeris.crs               = rtcm->c_rs;
    ephemeris.cuc               = rtcm->c_uc;
    ephemeris.cus               = rtcm->c_us;
    ephemeris.cic               = rtcm->c_ic;
    ephemeris.cis               = rtcm->c_is;
    ephemeris.e                 = rtcm->e;
    ephemeris.m0                = rtcm->m_0;
    ephemeris.delta_n           = rtcm->delta_n;
    ephemeris.a                 = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0                = rtcm->i_0;
    ephemeris.omega0            = rtcm->omega_0;
    ephemeris.omega             = rtcm->omega;
    ephemeris.omega_dot         = rtcm->omegadot;
    ephemeris.idot              = rtcm->idot;
    ephemeris.fit_interval_flag = rtcm->fit;
    ephemeris.l2_p_data_flag    = rtcm->l2_p_data_flag;
    generator.process_ephemeris(ephemeris);
}

static void rtcm_to_ephemeris(generator::tokoro::Generator& generator,
                              format::rtcm::Rtcm1042 const* rtcm) {
    ephemeris::BdsEphemeris ephemeris{};
    ephemeris.prn         = rtcm->prn;
    ephemeris.week_number = rtcm->week_number;
    ephemeris.sv_health   = rtcm->sv_health;
    ephemeris.lpp_iod     = static_cast<uint16_t>(static_cast<uint32_t>(rtcm->toe) >> 9);
    ephemeris.iodc        = (static_cast<uint32_t>(rtcm->toc) / 720) % 240;
    ephemeris.iode        = (static_cast<uint32_t>(rtcm->toe) / 720) % 240;
    ephemeris.aode        = rtcm->aode;
    ephemeris.aodc        = rtcm->aodc;
    ephemeris.toc_time =
        ts::Bdt::from_week_tow(rtcm->week_number, static_cast<int64_t>(rtcm->toc), 0.0);
    ephemeris.toe_time =
        ts::Bdt::from_week_tow(rtcm->week_number, static_cast<int64_t>(rtcm->toe), 0.0);
    ephemeris.toc       = rtcm->toc;
    ephemeris.toe       = rtcm->toe;
    ephemeris.af2       = rtcm->af2;
    ephemeris.af1       = rtcm->af1;
    ephemeris.af0       = rtcm->af0;
    ephemeris.crc       = rtcm->crc;
    ephemeris.crs       = rtcm->crs;
    ephemeris.cuc       = rtcm->cuc;
    ephemeris.cus       = rtcm->cus;
    ephemeris.cic       = rtcm->cic;
    ephemeris.cis       = rtcm->cis;
    ephemeris.e         = rtcm->e;
    ephemeris.m0        = rtcm->m0;
    ephemeris.delta_n   = rtcm->delta_n;
    ephemeris.a         = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0        = rtcm->i0;
    ephemeris.omega0    = rtcm->omega0;
    ephemeris.omega     = rtcm->omega;
    ephemeris.omega_dot = rtcm->omega_dot;
    ephemeris.idot      = rtcm->idot;
    generator.process_ephemeris(ephemeris);
}

static void rtcm_to_ephemeris(generator::tokoro::Generator& generator,
                              format::rtcm::Rtcm1046 const* rtcm) {
    ephemeris::GalEphemeris ephemeris{};
    ephemeris.prn         = rtcm->prn;
    ephemeris.week_number = rtcm->week_number;
    ephemeris.lpp_iod     = rtcm->iod_nav;
    ephemeris.iod_nav     = rtcm->iod_nav;
    ephemeris.toc         = rtcm->toc;
    ephemeris.toe         = rtcm->toe;
    ephemeris.af2         = rtcm->af2;
    ephemeris.af1         = rtcm->af1;
    ephemeris.af0         = rtcm->af0;
    ephemeris.crc         = rtcm->crc;
    ephemeris.crs         = rtcm->crs;
    ephemeris.cuc         = rtcm->cuc;
    ephemeris.cus         = rtcm->cus;
    ephemeris.cic         = rtcm->cic;
    ephemeris.cis         = rtcm->cis;
    ephemeris.e           = rtcm->e;
    ephemeris.m0          = rtcm->m0;
    ephemeris.delta_n     = rtcm->delta_n;
    ephemeris.a           = rtcm->sqrt_a * rtcm->sqrt_a;
    ephemeris.i0          = rtcm->i0;
    ephemeris.omega0      = rtcm->omega0;
    ephemeris.omega       = rtcm->omega;
    ephemeris.omega_dot   = rtcm->omega_dot;
    ephemeris.idot        = rtcm->idot;
    generator.process_ephemeris(ephemeris);
}

static void process_rtcm_ephemeris(generator::tokoro::Generator& generator,
                                   format::rtcm::Message const*  message) {
    if (!message) return;
    if (auto rtcm1019 = dynamic_cast<format::rtcm::Rtcm1019 const*>(message)) {
        rtcm_to_ephemeris(generator, rtcm1019);
    } else if (auto rtcm1042 = dynamic_cast<format::rtcm::Rtcm1042 const*>(message)) {
        rtcm_to_ephemeris(generator, rtcm1042);
    } else if (auto rtcm1046 = dynamic_cast<format::rtcm::Rtcm1046 const*>(message)) {
        rtcm_to_ephemeris(generator, rtcm1046);
    }
}

static void process_ubx_ephemeris(generator::tokoro::Generator&               generator,
                                  format::nav::gps::lnav::EphemerisCollector& gps_collector,
                                  format::nav::gal::InavEphemerisCollector&   gal_collector,
                                  format::nav::D1Collector&                   bds_collector,
                                  format::ubx::RxmSfrbx*                      sfrbx) {
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
    }

    // Load antex
#ifdef INCLUDE_FORMAT_ANTEX
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
    if (!cfg.diag_dir.empty()) ref_station->set_diag_output(cfg.diag_dir);

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
                                          sfrbx);
                }
            }
            if (has_ubx && ubx_reader.next(ubx_msg))
                heap.push({ubx_msg.timestamp_us + shift_us, 0});
        } else if (top.source == 2) {
            // RTCM message — feed to parser for ephemeris extraction
            rtcm_parser.append(rtcm_msg.data.data(), rtcm_msg.data.size());
            while (auto msg = rtcm_parser.try_parse()) {
                process_rtcm_ephemeris(*generator, msg.get());
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
