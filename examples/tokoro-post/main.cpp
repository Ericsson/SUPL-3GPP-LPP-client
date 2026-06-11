/**
 * tokoro-post: Fast post-processing binary for SSR→VRS conversion.
 * Reads UBX tbin + SSR tbin + RINEX nav, produces RTCM output.
 * No streamline, no scheduler, no event queue — pure sequential processing.
 */
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <string>
#include <vector>

#include <ephemeris/bds.hpp>
#include <ephemeris/gal.hpp>
#include <ephemeris/gps.hpp>
#include <format/lpp/uper_parser.hpp>
#include <format/nav/gps/lnav.hpp>
#include <format/rinex/nav_reader.hpp>
#include <format/tbin/reader.hpp>
#include <format/ubx/messages/rxm_sfrbx.hpp>
#include <format/ubx/parser.hpp>
#include <generator/rtcm/generator.hpp>
#include <generator/tokoro/generator.hpp>
#include <loglet/loglet.hpp>
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

struct Config {
    std::string              ubx_path;
    std::string              ssr_path;
    std::string              output_path;
    std::string              diag_dir;
    std::string              antex_file;
    std::vector<std::string> nav_files;
    double                   ubx_shift = -72000.0;
    double                   stop_time = 0.0;
    double                   pos_x = 0, pos_y = 0, pos_z = 0;
    int                      eph_cache = 512;
    bool                     no_gps    = false;
    bool                     no_gal    = false;
    bool                     no_bds    = false;
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
        else if (arg == "--ubx-shift")
            cfg.ubx_shift = std::stod(next());
        else if (arg == "--stop-time")
            cfg.stop_time = std::stod(next());
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
        else {
            fprintf(stderr, "Unknown argument: %s\n", arg.c_str());
            exit(1);
        }
    }
    if (cfg.ssr_path.empty() || cfg.output_path.empty()) {
        fprintf(stderr, "Usage: tokoro-post --ssr <tbin> --output <rtcm> --pos-x X --pos-y Y "
                        "--pos-z Z [--ubx <tbin>] [--nav <file>...]\n");
        exit(1);
    }
    return cfg;
}

// Merge heap entry
struct MergeEntry {
    int64_t  timestamp_us;
    uint32_t source;  // 0=ubx, 1=ssr
    bool     operator>(MergeEntry const& o) const { return timestamp_us > o.timestamp_us; }
};

int main(int argc, char** argv) {
    auto cfg = parse_args(argc, argv);

    // Open tbin readers
    format::tbin::Reader ubx_reader, ssr_reader;
    bool                 has_ubx = !cfg.ubx_path.empty();
    if (has_ubx && !ubx_reader.open(cfg.ubx_path)) {
        fprintf(stderr, "Cannot open %s\n", cfg.ubx_path.c_str());
        return 1;
    }
    if (!ssr_reader.open(cfg.ssr_path)) {
        fprintf(stderr, "Cannot open %s\n", cfg.ssr_path.c_str());
        return 1;
    }

    // Setup generator
    auto generator = std::make_unique<generator::tokoro::Generator>();
    generator->set_iod_consistency_check(true);
    generator->set_ephemeris_max_cache(static_cast<size_t>(cfg.eph_cache));

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
    rs_cfg.generate_qzs         = false;
    auto ref_station = std::make_shared<generator::tokoro::ReferenceStation>(*generator, rs_cfg);
    ref_station->set_msm_type(5);
    ref_station->set_phase_alignment(true);
    ref_station->set_shapiro_correction(true);
    ref_station->set_earth_solid_tides_correction(true);
    ref_station->set_phase_windup_correction(true);
    if (!cfg.diag_dir.empty()) ref_station->set_diag_output(cfg.diag_dir);

    // Open output
    FILE* out_file = fopen(cfg.output_path.c_str(), "wb");
    if (!out_file) {
        fprintf(stderr, "Cannot open output %s\n", cfg.output_path.c_str());
        return 1;
    }

    // Setup parsers
    format::ubx::Parser     ubx_parser;
    format::lpp::UperParser lpp_parser;

    // Merge heap
    std::priority_queue<MergeEntry, std::vector<MergeEntry>, std::greater<MergeEntry>> heap;
    format::tbin::Message ubx_msg, ssr_msg;
    int64_t               shift_us = static_cast<int64_t>(cfg.ubx_shift * 1000000.0);
    int64_t               stop_us =
        cfg.stop_time > 0 ? static_cast<int64_t>(cfg.stop_time * 1000000.0) : INT64_MAX;

    if (has_ubx && ubx_reader.next(ubx_msg)) heap.push({ubx_msg.timestamp_us + shift_us, 0});
    if (ssr_reader.next(ssr_msg)) heap.push({ssr_msg.timestamp_us, 1});

    // Processing state
    ts::Tai last_gen_time;
    int64_t last_log_us   = 0;
    int64_t first_data_us = 0;
    size_t  msg_count     = 0;
    auto    wall_start    = std::chrono::steady_clock::now();

    // Main loop
    while (!heap.empty()) {
        auto top = heap.top();
        heap.pop();

        if (top.timestamp_us > stop_us) break;

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
                // We only care about SFRBX for ephemeris but the X20 doesn't output SF1
                // so this is a no-op for GPS. Kept for BDS/GAL if they work.
            }
            if (has_ubx && ubx_reader.next(ubx_msg))
                heap.push({ubx_msg.timestamp_us + shift_us, 0});
        } else {
            // SSR LPP message
            lpp_parser.append(ssr_msg.data.data(), ssr_msg.data.size());
            while (auto lpp_msg = lpp_parser.try_parse()) {
                auto new_data = generator->process_lpp(*lpp_msg);
                if (new_data) {
                    auto gen_time = generator->last_correction_data_time();
                    if (gen_time != last_gen_time) {
                        last_gen_time = gen_time;
                        ref_station->generate(gen_time);
                        auto messages = ref_station->produce();
                        for (auto& m : messages) {
                            fwrite(m.data().data(), 1, m.data().size(), out_file);
                        }
                        msg_count += messages.size();
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
