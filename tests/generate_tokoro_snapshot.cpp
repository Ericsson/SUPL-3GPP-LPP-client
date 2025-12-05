#include <args.hxx>
#include <generator/tokoro/generator.hpp>
#include <generator/tokoro/snapshot.hpp>
#include <loglet/loglet.hpp>
#include <msgpack/msgpack.hpp>

#include <fstream>
#include <string>

LOGLET_MODULE(generate_snapshot);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(generate_snapshot)

static bool load_msgpack(std::string const& filename, generator::tokoro::SnapshotInput& input) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        ERRORF("Failed to open input file: %s", filename.c_str());
        return false;
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(ifs)),
                                std::istreambuf_iterator<char>());
    msgpack::Unpacker    unpacker(buffer.data(), buffer.size());

    if (!input.msgpack_unpack(unpacker)) {
        ERRORF("Failed to parse msgpack: %s", filename.c_str());
        return false;
    }

    return true;
}

static bool save_msgpack(std::string const&                       filename,
                         generator::tokoro::SnapshotOutput const& output) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        ERRORF("Failed to open output file: %s", filename.c_str());
        return false;
    }

    std::vector<uint8_t> buffer;
    msgpack::Packer      packer(buffer);
    output.msgpack_pack(packer);

    ofs.write(reinterpret_cast<char const*>(buffer.data()), buffer.size());
    return true;
}

int main(int argc, char** argv) {
    loglet::initialize();

    args::ArgumentParser parser("Generate Tokoro snapshot observations");
    args::Group          logging_group(parser, "Logging:");
    args::Flag           trace(logging_group, "trace", "Set log level to trace", {"trace"});
    args::Flag           verbose(logging_group, "verbose", "Set log level to verbose", {"verbose"});
    args::Flag           debug(logging_group, "debug", "Set log level to debug", {"debug"});
    args::Flag           info(logging_group, "info", "Set log level to info", {"info"});
    args::Flag           warning(logging_group, "warning", "Set log level to warning", {"warning"});
    args::Flag           error(logging_group, "error", "Set log level to error", {"error"});
    args::Flag no_color(logging_group, "no-color", "Disable colored output", {"log-no-color"});
    args::Flag print_input(parser, "print-input", "Print snapshot input data before processing",
                           {"print-input"});
    args::PositionalList<std::string> input_files(parser, "input_files", "Input snapshot files");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help const&) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError const& e) {
        ERRORF("%s", e.what());
        std::cerr << parser;
        return 1;
    }

    loglet::Level log_level = loglet::Level::Info;
    if (trace) {
        log_level = loglet::Level::Trace;
    } else if (verbose) {
        log_level = loglet::Level::Verbose;
    } else if (debug) {
        log_level = loglet::Level::Debug;
    } else if (info) {
        log_level = loglet::Level::Info;
    } else if (warning) {
        log_level = loglet::Level::Warning;
    } else if (error) {
        log_level = loglet::Level::Error;
    }
    loglet::set_level(log_level);
    loglet::set_color_enable(!no_color);

    if (!input_files) {
        ERRORF("No input files specified");
        std::cerr << parser;
        return 1;
    }

    for (auto const& input_file : args::get(input_files)) {
        INFOF("Processing: %s", input_file.c_str());

        generator::tokoro::SnapshotInput input;
        if (!load_msgpack(input_file, input)) {
            continue;
        }

        if (print_input) {
            INFOF("Snapshot data:");
            INFOF("  Time: %s", input.time.rtklib_time_string().c_str());
            INFOF("  Position: [%.3f, %.3f, %.3f]", input.position.x, input.position.y,
                  input.position.z);
            INFOF("  Ephemeris: %zu", input.ephemeris.size());
            for (auto const& eph : input.ephemeris) {
                INFOF("    Data size: %zu bytes", eph.data.size());
            }
            INFOF("  Orbit corrections: %zu", input.orbit_corrections.size());
            for (auto const& orb : input.orbit_corrections) {
                INFOF("    GNSS: %d, PRN: %d, IOD: %d", orb.gnss, orb.prn, orb.iod);
            }
            INFOF("  Clock corrections: %zu", input.clock_corrections.size());
            for (auto const& clk : input.clock_corrections) {
                INFOF("    GNSS: %d, PRN: %d", clk.gnss, clk.prn);
            }
            INFOF("  Code biases: %zu", input.code_biases.size());
            for (auto const& cb : input.code_biases) {
                INFOF("    GNSS: %d, PRN: %d, Signal: %d", cb.gnss, cb.prn, cb.signal);
            }
            INFOF("  Phase biases: %zu", input.phase_biases.size());
            for (auto const& pb : input.phase_biases) {
                INFOF("    GNSS: %d, PRN: %d, Signal: %d", pb.gnss, pb.prn, pb.signal);
            }
            INFOF("  Grid data: %zu", input.grid_data.size());
            for (auto const& gd : input.grid_data) {
                INFOF("    GNSS: %d, Points: %zu", gd.gnss, gd.grid_points.size());
            }
            INFOF("  Correction Point Set:");
            INFOF("    Set ID: %u", input.correction_point_set.set_id);
            INFOF("    Grid point count: %ld", input.correction_point_set.grid_point_count);
            INFOF("    Reference point: (%.6f, %.6f)",
                  input.correction_point_set.reference_point_latitude,
                  input.correction_point_set.reference_point_longitude);
            INFOF("    Steps: (%.6f, %.6f)", input.correction_point_set.step_of_latitude,
                  input.correction_point_set.step_of_longitude);
            INFOF("    Number of steps: (%ld, %ld)",
                  input.correction_point_set.number_of_steps_latitude,
                  input.correction_point_set.number_of_steps_longitude);
            INFOF("    Bitmask: 0x%016lx", input.correction_point_set.bitmask);
        }

        generator::tokoro::Generator gen;
        gen.load_snapshot(input);

        DEBUGF("Loaded snapshot into generator");
        DEBUGF("  Config - GPS: %d, GAL: %d, BDS: %d, QZS: %d", input.config.gps, input.config.gal,
               input.config.bds, input.config.qzs);

        auto config = generator::tokoro::ReferenceStationConfig{input.config.itrf_position,
                                                                input.config.rtcm_position,
                                                                input.config.gps,
                                                                input.config.glo,
                                                                input.config.gal,
                                                                input.config.bds,
                                                                input.config.qzs};

        DEBUGF("Creating reference station...");
        auto station = gen.define_reference_station(config);

        station->set_shapiro_correction(input.config.shapiro_correction);
        station->set_earth_solid_tides_correction(input.config.earth_solid_tides_correction);
        station->set_phase_windup_correction(input.config.phase_windup_correction);
        station->set_antenna_phase_variation_correction(
            input.config.antenna_phase_variation_correction);
        station->set_tropospheric_height_correction(input.config.tropospheric_height_correction);
        station->set_elevation_mask(input.config.elevation_mask);
        station->set_phase_range_rate(input.config.phase_range_rate);
        station->set_negative_phase_windup(input.config.negative_phase_windup);
        station->set_require_code_bias(input.config.require_code_bias);
        station->set_require_phase_bias(input.config.require_phase_bias);
        station->set_require_tropo(input.config.require_tropo);
        station->set_require_iono(input.config.require_iono);
        station->set_use_tropospheric_model(input.config.use_tropospheric_model);
        station->set_use_ionospheric_height_correction(
            input.config.use_ionospheric_height_correction);

        if (!station->generate(input.time)) {
            ERRORF("Generation failed: %s", input_file.c_str());
            continue;
        }

        DEBUGF("Generation succeeded, extracting observations...");

        generator::tokoro::SnapshotOutput output;
        generator::tokoro::extract_observations(station, output);

        DEBUGF("Extracted %zu observations", output.observations.size());

        auto output_file = input_file;
        auto pos         = output_file.find("_snapshot");
        if (pos != std::string::npos) {
            output_file.replace(pos, 9, "_output");
        } else {
            output_file += ".output";
        }

        if (!save_msgpack(output_file, output)) {
            continue;
        }

        INFOF("Generated: %s (%zu observations)", output_file.c_str(), output.observations.size());
    }

    return 0;
}
