#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#include <args.hxx>
#pragma GCC diagnostic pop

#include <coordinates/coordinates.hpp>
#include <coordinates/ecef_aer.hpp>
#include <coordinates/ecef_enu.hpp>
#include <coordinates/ecef_llh.hpp>
#include <coordinates/ecef_ned.hpp>
#include <coordinates/frames.hpp>
#include <coordinates/transform_graph.hpp>
#include <core/core.hpp>
#include <loglet/loglet.hpp>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace coordinates;

enum class CoordType {
    ECEF,
    LLH,
    ENU,
    NED,
    AER,
};

struct CoordInput {
    CoordType type;
    Vector3d  values;
    CoordType output_type;
    bool      has_output_override;
    double    input_epoch;
    double    output_epoch;
    bool      has_input_epoch;
    bool      has_output_epoch;
    FrameId   from_frame;
    FrameId   to_frame;
    bool      has_from_frame;
    bool      has_to_frame;
    Vector3d  velocity;
    bool      has_velocity;
};

struct Options {
    Vector3d origin;
    bool     has_origin;
    bool     show_path;
};

CoordType parse_coord_type(std::string const& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    if (lower == "ecef") return CoordType::ECEF;
    if (lower == "llh") return CoordType::LLH;
    if (lower == "enu") return CoordType::ENU;
    if (lower == "ned") return CoordType::NED;
    if (lower == "aer") return CoordType::AER;
    throw std::runtime_error("Invalid coordinate type: " + str);
}

char const* coord_type_name(CoordType type) {
    switch (type) {
    case CoordType::ECEF: return "ecef";
    case CoordType::LLH: return "llh";
    case CoordType::ENU: return "enu";
    case CoordType::NED: return "ned";
    case CoordType::AER: return "aer";
    }
    return "unknown";
}

Options parse_arguments(int argc, char** argv) {
    args::ArgumentParser         parser("Reference frame transformation tool");
    args::HelpFlag               help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> origin(parser, "lat,lon,h", "Origin for local coordinates",
                                        {'r', "origin"});
    args::Flag show_path(parser, "path", "Show transformation path", {'p', "path"});
    args::Flag list_frames(parser, "list", "List available frames", {'l', "list"});
    args::Flag show_matrix(parser, "matrix", "Show transformation matrix", {'m', "matrix"});

    args::Group logging_group(parser, "Logging:", args::Group::Validators::DontCare);
    args::Flag  log_trace(logging_group, "trace", "Set log level to trace", {"trace"});
    args::Flag  log_verbose(logging_group, "verbose", "Set log level to verbose", {"verbose"});
    args::Flag  log_debug(logging_group, "debug", "Set log level to debug", {"debug"});
    args::Flag  log_info(logging_group, "info", "Set log level to info", {"info"});
    args::Flag  log_warning(logging_group, "warning", "Set log level to warning", {"warning"});
    args::Flag  log_error(logging_group, "error", "Set log level to error", {"error"});
    args::Flag  log_no_color(logging_group, "no-color", "Disable colored output", {"log-no-color"});
    args::Flag  log_flush(logging_group, "flush", "Flush log after each line", {"log-flush"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help&) {
        std::cout << parser;
        std::exit(0);
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    } catch (args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    loglet::Level log_level = loglet::Level::Info;
    if (log_trace) {
        log_level = loglet::Level::Trace;
    } else if (log_verbose) {
        log_level = loglet::Level::Verbose;
    } else if (log_debug) {
        log_level = loglet::Level::Debug;
    } else if (log_info) {
        log_level = loglet::Level::Info;
    } else if (log_warning) {
        log_level = loglet::Level::Warning;
    } else if (log_error) {
        log_level = loglet::Level::Error;
    }

    loglet::set_level(log_level);
    loglet::set_color_enable(!log_no_color);
    loglet::set_always_flush(log_flush);

    if (list_frames) {
        std::cout << "Available reference frames:" << std::endl;
        auto frames = all_frames();
        for (auto frame : frames) {
            std::cout << "  " << frame_name(frame) << std::endl;
        }
        std::cout << "\nAliases:" << std::endl;
        std::cout << "  wgs84 -> wgs84(g1762)" << std::endl;
        std::cout << "  itrf -> itrf2020" << std::endl;
        std::cout << "  itrf-current -> itrf2020" << std::endl;
        std::exit(0);
    }

    if (show_matrix) {
        TransformGraph graph;
        auto           frames = all_frames();

        std::cout << "\033[1mTransformation Matrix\033[0m (direct transformations only)\n\n";

        // Print vertical header
        int max_name_len = 0;
        for (auto frame : frames) {
            int len = std::strlen(frame_name(frame));
            if (len > max_name_len) max_name_len = len;
        }

        for (int row = max_name_len - 1; row >= 0; row--) {
            std::cout << std::setw(15) << "";
            bool alt_col = false;
            for (auto frame : frames) {
                if (alt_col) std::cout << "\033[48;5;236m";
                char const* name = frame_name(frame);
                int         len  = std::strlen(name);
                if (row < len) {
                    std::cout << name[row];
                } else {
                    std::cout << " ";
                }
                std::cout << "\033[0m";
                alt_col = !alt_col;
            }
            std::cout << "\n";
        }
        std::cout << "\n";

        bool alt_row = false;
        for (auto from_frame : frames) {
            if (alt_row) std::cout << "\033[48;5;235m";
            std::cout << std::setw(15) << std::left << frame_name(from_frame) << std::right;

            bool alt_col = false;
            for (auto to_frame : frames) {
                if (alt_col) std::cout << "\033[48;5;236m";
                if (from_frame == to_frame) {
                    std::cout << "\033[90m█\033[0m";
                } else if (graph.has_direct_edge(from_frame, to_frame)) {
                    std::cout << "\033[42m \033[0m";
                } else {
                    std::cout << " ";
                }
                if (alt_row && !alt_col) std::cout << "\033[48;5;235m";
                if (!alt_row && alt_col) std::cout << "\033[48;5;236m";
                if (alt_row && alt_col) std::cout << "\033[48;5;237m";
                alt_col = !alt_col;
            }
            std::cout << "\033[0m\n";
            alt_row = !alt_row;
        }
        std::exit(0);
    }

    Options opts;
    opts.show_path  = args::get(show_path);
    opts.has_origin = false;

    if (origin) {
        auto origin_str = args::get(origin);
        std::replace(origin_str.begin(), origin_str.end(), ',', ' ');
        std::istringstream iss(origin_str);
        double             lat, lon, h;
        if (!(iss >> lat >> lon >> h)) {
            std::cerr << "Invalid origin format. Expected: lat,lon,height" << std::endl;
            std::exit(1);
        }
        opts.origin     = Vector3d(lat * M_PI / 180.0, lon * M_PI / 180.0, h);
        opts.has_origin = true;
    }

    return opts;
}

CoordInput parse_coord_line(std::string const& line, UNUSED Options const& opts) {
    std::istringstream iss(line);
    std::string        type_str;
    iss >> type_str;

    CoordInput input;
    input.type                = parse_coord_type(type_str);
    input.has_output_override = false;
    input.has_input_epoch     = false;
    input.has_output_epoch    = false;
    input.has_from_frame      = false;
    input.has_to_frame        = false;
    input.input_epoch         = 2010.0;
    input.output_epoch        = 2010.0;
    input.has_velocity        = false;

    double v1, v2, v3;
    if (!(iss >> v1 >> v2 >> v3)) {
        throw std::runtime_error("Invalid coordinate values");
    }

    if (input.type == CoordType::LLH) {
        input.values = Vector3d(v1 * M_PI / 180.0, v2 * M_PI / 180.0, v3);
    } else {
        input.values = Vector3d(v1, v2, v3);
    }

    std::string token;
    while (iss >> token) {
        if (token == "vel") {
            double vx, vy, vz;
            if (iss >> vx >> vy >> vz) {
                input.velocity     = Vector3d(vx, vy, vz);
                input.has_velocity = true;
            }
        } else if (token == "->") {
            std::string output_str;
            if (iss >> output_str) {
                input.output_type         = parse_coord_type(output_str);
                input.has_output_override = true;
            }
        } else if (token == "@") {
            if (iss >> input.input_epoch) {
                input.has_input_epoch = true;
            }
        } else if (token == "@out") {
            if (iss >> input.output_epoch) {
                input.has_output_epoch = true;
            }
        } else if (token == "from") {
            std::string frame_str;
            if (iss >> frame_str) {
                if (frame_from_name(frame_str.c_str(), input.from_frame)) {
                    input.has_from_frame = true;
                } else {
                    throw std::runtime_error("Unknown source frame: " + frame_str);
                }
            }
        } else if (token == "to") {
            std::string frame_str;
            if (iss >> frame_str) {
                if (frame_from_name(frame_str.c_str(), input.to_frame)) {
                    input.has_to_frame = true;
                } else {
                    throw std::runtime_error("Unknown destination frame: " + frame_str);
                }
            }
        }
    }

    if (!input.has_output_override) {
        input.output_type = input.type;
    }

    if (!input.has_from_frame || !input.has_to_frame) {
        throw std::runtime_error("Missing 'from' or 'to' frame specification");
    }

    return input;
}

Vector3d convert_to_ecef(CoordInput const& input, Options const& opts) {
    switch (input.type) {
    case CoordType::ECEF: return input.values;
    case CoordType::LLH: {
        Llh<WGS84> llh{input.values};
        return llh_to_ecef(llh).value;
    }
    case CoordType::ENU: {
        if (!opts.has_origin) {
            throw std::runtime_error("ENU requires --origin");
        }
        Enu<WGS84> enu{input.values};
        Llh<WGS84> origin{opts.origin};
        return enu_to_ecef(enu, origin).value;
    }
    case CoordType::NED: {
        if (!opts.has_origin) {
            throw std::runtime_error("NED requires --origin");
        }
        Ned<WGS84> ned{input.values};
        Llh<WGS84> origin{opts.origin};
        return ned_to_ecef(ned, origin).value;
    }
    case CoordType::AER: {
        if (!opts.has_origin) {
            throw std::runtime_error("AER requires --origin");
        }
        Aer        aer{input.values};
        Llh<WGS84> origin{opts.origin};
        return aer_to_ecef(aer, origin).value;
    }
    }
    return input.values;
}

Vector3d convert_from_ecef(Vector3d const& ecef, CoordType output_type, Options const& opts) {
    switch (output_type) {
    case CoordType::ECEF: return ecef;
    case CoordType::LLH: {
        Ecef<WGS84> ecef_coord{ecef};
        return ecef_to_llh(ecef_coord).value;
    }
    case CoordType::ENU: {
        if (!opts.has_origin) {
            throw std::runtime_error("ENU requires --origin");
        }
        Ecef<WGS84> ecef_coord{ecef};
        Llh<WGS84>  origin{opts.origin};
        return ecef_to_enu(ecef_coord, origin).value;
    }
    case CoordType::NED: {
        if (!opts.has_origin) {
            throw std::runtime_error("NED requires --origin");
        }
        Ecef<WGS84> ecef_coord{ecef};
        Llh<WGS84>  origin{opts.origin};
        return ecef_to_ned(ecef_coord, origin).value;
    }
    case CoordType::AER: {
        if (!opts.has_origin) {
            throw std::runtime_error("AER requires --origin");
        }
        Ecef<WGS84> ecef_coord{ecef};
        Llh<WGS84>  origin{opts.origin};
        return ecef_to_aer(ecef_coord, origin).value;
    }
    }
    return ecef;
}

void print_coord(CoordType type, Vector3d const& values) {
    std::cout << coord_type_name(type);
    if (type == CoordType::LLH) {
        std::cout << " " << std::fixed << std::setprecision(9) << values.x() * 180.0 / M_PI << " "
                  << values.y() * 180.0 / M_PI << " " << std::setprecision(4) << values.z();
    } else {
        std::cout << " " << std::fixed << std::setprecision(4) << values.x() << " " << values.y()
                  << " " << values.z();
    }
    std::cout << std::endl;
}

void process_coordinate(CoordInput const& input, Options const& opts, TransformGraph const& graph) {
    try {
        Vector3d ecef_in          = convert_to_ecef(input, opts);
        Vector3d ecef_velocity_in = input.velocity;

        auto result = graph.transform(input.from_frame, input.to_frame, input.input_epoch,
                                      input.output_epoch, ecef_in, ecef_velocity_in);

        if (opts.show_path && !result.steps.empty()) {
            std::cout << frame_name(input.from_frame) << " @" << std::fixed << std::setprecision(1)
                      << input.input_epoch << ": ";
            print_coord(CoordType::ECEF, ecef_in);
            std::cout << "  vel: " << std::fixed << std::setprecision(6) << ecef_velocity_in.x()
                      << " " << ecef_velocity_in.y() << " " << ecef_velocity_in.z() << " m/y\n";

            for (auto const& step : result.steps) {
                std::cout << frame_name(step.state_out.frame) << " @" << std::fixed
                          << std::setprecision(1) << step.state_out.epoch << ": ";
                print_coord(CoordType::ECEF, step.state_out.position);
                std::cout << "  vel: " << std::fixed << std::setprecision(6)
                          << step.state_out.velocity.x() << " " << step.state_out.velocity.y()
                          << " " << step.state_out.velocity.z() << " m/y\n";

                Vector3d diff     = step.state_out.position - step.state_in.position;
                double   distance = diff.norm();
                std::cout << "  Δ(" << std::fixed << std::setprecision(4) << diff.x() << ", "
                          << diff.y() << ", " << diff.z() << ") = " << std::setprecision(6)
                          << distance << "m";
                std::cout << std::endl;
            }

            if (input.output_type != CoordType::ECEF) {
                Vector3d output = convert_from_ecef(result.final_position, input.output_type, opts);
                std::cout << "=> ";
                print_coord(input.output_type, output);
            }
            return;
        }

        if (opts.show_path && !result.steps.empty()) {
            std::cout << frame_name(input.from_frame) << " @" << std::fixed << std::setprecision(1)
                      << input.input_epoch << ": ";
            print_coord(CoordType::ECEF, ecef_in);

            for (auto const& step : result.steps) {
                std::cout << frame_name(step.state_out.frame) << " @" << std::fixed
                          << std::setprecision(1) << step.state_out.epoch << ": ";
                print_coord(CoordType::ECEF, step.state_out.position);

                Vector3d diff     = step.state_out.position - step.state_in.position;
                double   distance = diff.norm();
                std::cout << "  Δ(" << std::fixed << std::setprecision(4) << diff.x() << ", "
                          << diff.y() << ", " << diff.z() << ") = " << std::setprecision(6)
                          << distance << "m";
                std::cout << std::endl;
            }

            if (input.output_type != CoordType::ECEF) {
                Vector3d output = convert_from_ecef(result.final_position, input.output_type, opts);
                std::cout << "=> ";
                print_coord(input.output_type, output);
            }
            return;
        }

        Vector3d output = convert_from_ecef(result.final_position, input.output_type, opts);
        print_coord(input.output_type, output);

        if (input.has_velocity) {
            std::cout << "  vel: " << std::fixed << std::setprecision(6)
                      << result.final_velocity.x() << " " << result.final_velocity.y() << " "
                      << result.final_velocity.z() << " m/y";
        }
        std::cout << std::endl;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv) {
    auto opts = parse_arguments(argc, argv);

    TransformGraph graph;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string        first_word;
        iss >> first_word;

        if (first_word == "origin") {
            std::cout << "# " << line << std::endl;
            try {
                std::string type_str;
                double      v1, v2, v3;
                if (!(iss >> type_str >> v1 >> v2 >> v3)) {
                    throw std::runtime_error("Invalid origin format");
                }
                auto type = parse_coord_type(type_str);
                if (type == CoordType::LLH) {
                    opts.origin = Vector3d(v1 * M_PI / 180.0, v2 * M_PI / 180.0, v3);
                } else {
                    opts.origin = Vector3d(v1, v2, v3);
                }
                opts.has_origin = true;
                std::cout << "Origin set" << std::endl;
            } catch (std::exception const& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            continue;
        }

        std::cout << "# " << line << std::endl;
        try {
            auto input = parse_coord_line(line, opts);
            process_coordinate(input, opts, graph);
        } catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}
