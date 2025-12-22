#include "parse.hpp"
#include "types.hpp"

#include <core/string.hpp>
#include <loglet/loglet.hpp>

#include "../config.hpp"
#include "../processor/chunked_log.hpp"
#include <cxx11_compat.hpp>
#include "../program_io.hpp"

#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace output {

static args::Group                      gGroup{"Output:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "output",
    "Add an output interface.\n"
    "Usage: --output <type>:<arguments>\n\n"
    "Arguments:\n"
    "  format=<fmt>[+<fmt>...]\n"
    "  itags=<tag>[+<tag>...]\n"
    "  otags=<tag>[+<tag>...]\n"
    "  chain=<stage>[+<stage>...]\n\n"
    "Types and their specific arguments:\n"
    "  stdout:\n"
    "  file:\n"
    "    path=<path>\n"
    "    append=<true|false>\n"
    "  serial:\n"
    "    device=<device>\n"
    "    baudrate=<baudrate>\n"
    "    databits=<5|6|7|8>\n"
    "    stopbits=<1|2>\n"
    "    parity=<none|odd|even>\n"
    "  tcp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  udp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  tcp-server:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  chunked-log:\n"
    "    path=<base_path>\n"
    "  stream:\n"
    "    id=<stream_id>  (reference to --stream)\n"
    "\n"
    "Options:\n"
    "  unique=<bool> (default=false, create separate stream)\n"
    "\n"
    "Formats:\n"
    "  all, ubx, nmea, rtcm, ctrl, spartn, lpp-xer, lpp-uper, lrf, possib, location, raw, test\n"
    "Stages:\n"
    "  tlf, hexdump\n"
    "Examples:\n"
    "  --output file:path=/tmp/output,format=ubx+nmea",
    {"output"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

static OutputFormat parse_format(std::string const& str) {
    if (str == "all") return OUTPUT_FORMAT_ALL;
    if (str == "ubx") return OUTPUT_FORMAT_UBX;
    if (str == "nmea") return OUTPUT_FORMAT_NMEA;
    if (str == "rtcm") return OUTPUT_FORMAT_RTCM;
    if (str == "ctrl") return OUTPUT_FORMAT_CTRL;
    if (str == "spartn") return OUTPUT_FORMAT_SPARTN;
    if (str == "lpp-xer") return OUTPUT_FORMAT_LPP_XER;
    if (str == "lpp-uper") return OUTPUT_FORMAT_LPP_UPER;
    if (str == "lfr") return OUTPUT_FORMAT_LFR;
    if (str == "possib") return OUTPUT_FORMAT_POSSIB;
    if (str == "location") return OUTPUT_FORMAT_LOCATION;
    if (str == "raw") return OUTPUT_FORMAT_RAW;
    if (str == "test") return OUTPUT_FORMAT_TEST;
    throw args::ValidationError("--output format: invalid format, got `" + str + "`");
}

static OutputFormat parse_format_list(std::string const& str) {
    auto parts  = parse_list(str, '+');
    auto format = OUTPUT_FORMAT_NONE;
    for (auto const& part : parts) {
        format |= parse_format(part);
    }
    return format;
}

static std::string parse_stage(std::string const& str) {
    if (str == "tlf") return "tlf";
    if (str == "hexdump") return "hexdump";
    throw args::ValidationError("--output stage: invalid stage, got `" + str + "`");
}

static std::vector<std::string> parse_stages(std::string const& str) {
    auto                     parts = parse_list(str, '+');
    std::vector<std::string> stages;
    for (auto const& part : parts) {
        stages.push_back(parse_stage(part));
    }
    return stages;
}

static void fill_common(OutputCommon& common, Options const& options, std::string const& type) {
    if (options.find("format") == options.end()) {
        throw args::ValidationError("--output " + type + ": missing `format` option");
    }
    common.format = parse_format_list(options.at("format"));

    if (options.find("itags") != options.end()) {
        common.include_tags = parse_list(options.at("itags"), '+');
    }
    if (options.find("otags") != options.end()) {
        common.exclude_tags = parse_list(options.at("otags"), '+');
    }
    if (options.find("stages") != options.end()) {
        common.stages = parse_stages(options.at("stages"));
    }
}

static std::string generate_stream_id(std::string const& type, Options const& options,
                                      bool unique) {
    std::string base_id;
    if (type == "serial") {
        base_id = "serial:" + options.at("device");
    } else if (type == "tcp-client") {
        if (options.find("path") != options.end()) {
            base_id = "tcp-client:" + options.at("path");
        } else {
            base_id = "tcp-client:" + options.at("host") + ":" + options.at("port");
        }
    } else if (type == "udp-client") {
        if (options.find("path") != options.end()) {
            base_id = "udp-client:" + options.at("path");
        } else {
            base_id = "udp-client:" + options.at("host") + ":" + options.at("port");
        }
    }

    if (unique) {
        return base_id + "#" + generate_unique_id();
    }
    return base_id;
}

static void parse_stdout(Options const& options, OutputsConfig& outputs) {
    OutputStdoutConfig cfg;
    fill_common(cfg, options, "stdout");
    outputs.stdout_outputs.push_back(std::move(cfg));
}

static void parse_file(Options const& options, OutputsConfig& outputs) {
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--output file: missing `path` option");
    }

    OutputFileConfig cfg;
    fill_common(cfg, options, "file");
    cfg.path = options.at("path");

    if (options.find("append") != options.end()) {
        if (options.at("append") == "true") {
            cfg.append   = true;
            cfg.truncate = false;
        }
    }

    outputs.file.push_back(std::move(cfg));
}

static void parse_chunked_log(Options const& options, OutputsConfig& outputs) {
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--output chunked-log: missing `path` option");
    }

    OutputChunkedLogConfig cfg;
    fill_common(cfg, options, "chunked-log");
    cfg.path = options.at("path");

    outputs.chunked_log.push_back(std::move(cfg));
}

static void parse_tcp_server(Options const& options, OutputsConfig& outputs) {
    OutputTcpServerConfig cfg;
    fill_common(cfg, options, "tcp-server");

    if (options.find("path") != options.end()) {
        cfg.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--output tcp-server: missing `host` or `port` option");
        }
        cfg.listen = options.at("host");
        cfg.port   = static_cast<uint16_t>(std::stoul(options.at("port")));
    }

    outputs.tcp_server.push_back(std::move(cfg));
}

static void parse_stream_ref(Options const& options, OutputsConfig& outputs) {
    if (options.find("id") == options.end()) {
        throw args::ValidationError("--output stream: missing `id` option");
    }

    OutputStreamConfig cfg;
    fill_common(cfg, options, "stream");
    cfg.stream_id = options.at("id");
    outputs.stream.push_back(std::move(cfg));
}

static void parse_serial(Options const& options, OutputsConfig& outputs) {
    if (options.find("device") == options.end()) {
        throw args::ValidationError("--output serial: missing `device` option");
    }

    auto unique = parse_bool(options, "unique", false);

    OutputSerialConfig cfg;
    fill_common(cfg, options, "serial");
    cfg.stream_id     = generate_stream_id("serial", options, unique);
    cfg.config.device = options.at("device");

    if (options.find("baudrate") != options.end()) {
        cfg.config.baud_rate = parse_baudrate(options.at("baudrate"));
    }
    if (options.find("databits") != options.end()) {
        cfg.config.data_bits = parse_databits(options.at("databits"));
    }
    if (options.find("stopbits") != options.end()) {
        cfg.config.stop_bits = parse_stopbits(options.at("stopbits"));
    }
    if (options.find("parity") != options.end()) {
        cfg.config.parity_bit = parse_paritybit(options.at("parity"));
    }

    outputs.serial.push_back(std::move(cfg));
}

static void parse_tcp_client(Options const& options, OutputsConfig& outputs) {
    auto unique = parse_bool(options, "unique", false);

    OutputTcpClientConfig cfg;
    fill_common(cfg, options, "tcp-client");

    if (options.find("path") != options.end()) {
        cfg.config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--output tcp-client: missing `host` or `port` option");
        }
        cfg.config.host = options.at("host");
        cfg.config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.stream_id = generate_stream_id("tcp-client", options, unique);

    outputs.tcp_client.push_back(std::move(cfg));
}

static void parse_udp_client(Options const& options, OutputsConfig& outputs) {
    auto unique = parse_bool(options, "unique", false);

    OutputUdpClientConfig cfg;
    fill_common(cfg, options, "udp-client");

    if (options.find("path") != options.end()) {
        cfg.config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--output udp-client: missing `host` or `port` option");
        }
        cfg.config.host = options.at("host");
        cfg.config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.stream_id = generate_stream_id("udp-client", options, unique);

    outputs.udp_client.push_back(std::move(cfg));
}

static void parse_interface(std::string const& source, Config* config) {
    auto parts = core::split(source, ':');
    if (parts.size() < 1 || parts.size() > 2) {
        throw args::ParseError("--output not in type:arguments format: \"" + source + "\"");
    }

    auto type    = parts[0];
    auto options = parts.size() == 2 ? parse_options(parts[1]) : Options{};

    if (type == "stdout") return parse_stdout(options, config->outputs_config);
    if (type == "file") return parse_file(options, config->outputs_config);
    if (type == "chunked-log") return parse_chunked_log(options, config->outputs_config);
    if (type == "tcp-server") return parse_tcp_server(options, config->outputs_config);
    if (type == "stream") return parse_stream_ref(options, config->outputs_config);
    if (type == "serial") return parse_serial(options, config->outputs_config);
    if (type == "tcp-client") return parse_tcp_client(options, config->outputs_config);
    if (type == "udp-client") return parse_udp_client(options, config->outputs_config);

    throw args::ParseError("--output type not recognized: \"" + type + "\"");
}

void parse(Config* config) {
    for (auto const& output : gArgs.Get()) {
        parse_interface(output, config);
    }
}

void dump(ProgramOutput const& config) {
    for (auto const& output : config.outputs) {
        DEBUGF("%p", output.initial_interface.get());
        DEBUG_INDENT_SCOPE();
        DEBUGF("format: %s%s%s%s%s%s%s%s%s%s%s", (output.format & OUTPUT_FORMAT_UBX) ? "UBX " : "",
               (output.format & OUTPUT_FORMAT_NMEA) ? "NMEA " : "",
               (output.format & OUTPUT_FORMAT_RTCM) ? "RTCM " : "",
               (output.format & OUTPUT_FORMAT_CTRL) ? "CTRL " : "",
               (output.format & OUTPUT_FORMAT_LPP_XER) ? "LPP-XER " : "",
               (output.format & OUTPUT_FORMAT_LPP_UPER) ? "LPP-UPER " : "",
               (output.format & OUTPUT_FORMAT_SPARTN) ? "SPARTN " : "",
               (output.format & OUTPUT_FORMAT_LFR) ? "LFR " : "",
               (output.format & OUTPUT_FORMAT_POSSIB) ? "POSSIB " : "",
               (output.format & OUTPUT_FORMAT_LOCATION) ? "LOCATION " : "",
               (output.format & OUTPUT_FORMAT_TEST) ? "TEST " : "");
    }
}

void dump(OutputsConfig const& config) {
    for (auto const& c : config.stream) {
        DEBUGF("stream: id=%s", c.stream_id.c_str());
    }
    for (auto const& c : config.stdout_outputs) {
        (void)c;
        DEBUGF("stdout");
    }
    for (auto const& c : config.file) {
        DEBUGF("file: path=%s", c.path.c_str());
    }
    for (auto const& c : config.chunked_log) {
        DEBUGF("chunked-log: path=%s", c.path.c_str());
    }
    for (auto const& c : config.tcp_server) {
        DEBUGF("tcp-server: listen=%s port=%u", c.listen.c_str(), c.port);
    }
    for (auto const& c : config.serial) {
        DEBUGF("serial: stream_id=%s device=%s", c.stream_id.c_str(), c.config.device.c_str());
    }
    for (auto const& c : config.tcp_client) {
        DEBUGF("tcp-client: stream_id=%s host=%s port=%u", c.stream_id.c_str(),
               c.config.host.c_str(), c.config.port);
    }
    for (auto const& c : config.udp_client) {
        DEBUGF("udp-client: stream_id=%s host=%s port=%u", c.stream_id.c_str(),
               c.config.host.c_str(), c.config.port);
    }
}

}  // namespace output
