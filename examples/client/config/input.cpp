#include "parse.hpp"
#include "types.hpp"

#include <core/string.hpp>
#include <loglet/loglet.hpp>

#include "../config.hpp"
#include "../program_io.hpp"

#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace input {

static args::Group                      gGroup{"Input:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "input",
    "Add an input interface.\n"
    "Usage: --input <type>:<arguments>\n\n"
    "Arguments:\n"
    "  format=<fmt>[+<fmt>...]\n"
    "  tags=<tag>[+<tag>...]\n"
    "  chain=<stage>[+<stage>...]\n\n"
    "Types and their specific arguments:\n"
    "  stdin:\n"
    "  file:\n"
    "    path=<path>\n"
    "    bps=<bps>\n"
    "  serial:\n"
    "    device=<device>\n"
    "    baudrate=<baudrate>\n"
    "    databits=<5|6|7|8>\n"
    "    stopbits=<1|2>\n"
    "    parity=<none|odd|even>\n"
    "  tcp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    reconnect=<bool> (default=true)\n"
    "    path=<path>\n"
    "  tcp-server:\n"
    "    listen=<addr> (default=0.0.0.0)\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  udp-server:\n"
    "    listen=<addr> (default=0.0.0.0)\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  stream:\n"
    "    id=<stream_id>  (reference to --stream)\n"
    "\n"
    "Options:\n"
    "  unique=<bool> (default=false, create separate stream)\n"
    "  nmea_lf_only=<bool> (default=false)\n"
    "  discard_errors=<bool> (default=false)\n"
    "  discard_unknowns=<bool> (default=false)\n"
    "  exclude_from_shutdown=<bool> (default=false)\n"
    "\n"
    "Stages:\n"
    "  tlf\n"
    "Formats:\n"
    "  all, ubx, nmea, rtcm, ctrl, lpp-uper, lpp-uper-pad, raw\n",
    {"input"},
};
static args::Flag gDisablePipeBufferOptimization{
    gGroup,
    "disable-pipe-buffer-optimization",
    "Disable pipe buffer size optimization for input streams",
    {"input-disable-pipe-buffer-optimization"},
};
static args::Flag gShutdownOnComplete{
    gGroup,
    "shutdown-on-complete",
    "Shutdown when all inputs complete (e.g., file EOF, disconnected serial/tcp)",
    {"input-shutdown-on-complete"},
};
static args::ValueFlag<int> gShutdownDelay{
    gGroup,
    "milliseconds",
    "Delay before shutdown to allow queue draining (default: 1000ms)",
    {"input-shutdown-delay"},
    1000,
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

static InputFormat parse_format(std::string const& str) {
    if (str == "all") return INPUT_FORMAT_ALL;
    if (str == "ubx") return INPUT_FORMAT_UBX;
    if (str == "nmea") return INPUT_FORMAT_NMEA;
    if (str == "rtcm") return INPUT_FORMAT_RTCM;
    if (str == "ctrl") return INPUT_FORMAT_CTRL;
    if (str == "lpp-uper") return INPUT_FORMAT_LPP_UPER;
    if (str == "lpp-uper-pad") return INPUT_FORMAT_LPP_UPER_PAD;
    if (str == "raw") return INPUT_FORMAT_RAW;
    throw args::ValidationError("--input format: invalid format, got `" + str + "`");
}

static InputFormat parse_format_list(std::string const& str) {
    auto parts  = parse_list(str, '+');
    auto format = INPUT_FORMAT_NONE;
    for (auto const& part : parts) {
        format |= parse_format(part);
    }
    return format;
}

static std::string parse_stage(std::string const& str) {
    if (str == "tlf") return "tlf";
    throw args::ValidationError("--input stage: invalid stage, got `" + str + "`");
}

static std::vector<std::string> parse_stages(std::string const& str) {
    auto                     parts = parse_list(str, '+');
    std::vector<std::string> stages;
    for (auto const& part : parts) {
        stages.push_back(parse_stage(part));
    }
    return stages;
}

static void fill_common(InputCommon& common, Options const& options, std::string const& type) {
    if (options.find("format") == options.end()) {
        throw args::ValidationError("--input " + type + ": missing `format` option");
    }
    common.format = parse_format_list(options.at("format"));

    if (options.find("tags") != options.end()) {
        common.tags = parse_list(options.at("tags"), '+');
    }
    if (options.find("chain") != options.end()) {
        common.stages = parse_stages(options.at("chain"));
    }

    common.print                 = parse_bool(options, "print", false);
    common.nmea_lf_only          = parse_bool(options, "nmea_lf_only", false);
    common.discard_errors        = parse_bool(options, "discard_errors", false);
    common.discard_unknowns      = parse_bool(options, "discard_unknowns", false);
    common.exclude_from_shutdown = parse_bool(options, "exclude_from_shutdown", false);
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
    } else if (type == "tcp-server") {
        if (options.find("path") != options.end()) {
            base_id = "tcp-server:" + options.at("path");
        } else {
            auto listen =
                options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
            base_id = "tcp-server:" + listen + ":" + options.at("port");
        }
    } else if (type == "udp-server") {
        if (options.find("path") != options.end()) {
            base_id = "udp-server:" + options.at("path");
        } else {
            auto listen =
                options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
            base_id = "udp-server:" + listen + ":" + options.at("port");
        }
    }

    if (unique) {
        return base_id + "#" + generate_unique_id();
    }
    return base_id;
}

static void parse_stdin(Options const& options, InputsConfig& inputs) {
    InputStdinConfig cfg;
    fill_common(cfg, options, "stdin");
    inputs.stdin_inputs.push_back(std::move(cfg));
}

static void parse_file(Options const& options, InputsConfig& inputs) {
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--input file: missing `path` option");
    }

    InputFileConfig cfg;
    fill_common(cfg, options, "file");
    cfg.path = options.at("path");

    if (options.find("bps") != options.end()) {
        auto bps           = std::stoull(options.at("bps"));
        cfg.bytes_per_tick = static_cast<size_t>((bps + 9) / 10);
    }

    inputs.file.push_back(std::move(cfg));
}

static void parse_stream_ref(Options const& options, InputsConfig& inputs) {
    if (options.find("id") == options.end()) {
        throw args::ValidationError("--input stream: missing `id` option");
    }

    InputStreamConfig cfg;
    fill_common(cfg, options, "stream");
    cfg.stream_id = options.at("id");
    inputs.stream.push_back(std::move(cfg));
}

static void parse_serial(Options const& options, InputsConfig& inputs) {
    if (options.find("device") == options.end()) {
        throw args::ValidationError("--input serial: missing `device` option");
    }

    auto unique = parse_bool(options, "unique", false);

    InputSerialConfig cfg;
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

    inputs.serial.push_back(std::move(cfg));
}

static void parse_tcp_client(Options const& options, InputsConfig& inputs) {
    auto unique = parse_bool(options, "unique", false);

    InputTcpClientConfig cfg;
    fill_common(cfg, options, "tcp-client");

    if (options.find("path") != options.end()) {
        cfg.config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--input tcp-client: missing `host` or `port` option");
        }
        cfg.config.host = options.at("host");
        cfg.config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.config.reconnect = parse_bool(options, "reconnect", true);
    cfg.stream_id        = generate_stream_id("tcp-client", options, unique);

    inputs.tcp_client.push_back(std::move(cfg));
}

static void parse_tcp_server(Options const& options, InputsConfig& inputs) {
    auto unique = parse_bool(options, "unique", false);

    InputTcpServerConfig cfg;
    fill_common(cfg, options, "tcp-server");

    if (options.find("path") != options.end()) {
        cfg.path = options.at("path");
    } else {
        if (options.find("port") == options.end()) {
            throw args::ValidationError("--input tcp-server: missing `port` option");
        }
        cfg.listen = options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
        cfg.port   = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.stream_id = generate_stream_id("tcp-server", options, unique);

    inputs.tcp_server.push_back(std::move(cfg));
}

static void parse_udp_server(Options const& options, InputsConfig& inputs) {
    auto unique = parse_bool(options, "unique", false);

    InputUdpServerConfig cfg;
    fill_common(cfg, options, "udp-server");

    if (options.find("path") != options.end()) {
        cfg.path = options.at("path");
    } else {
        if (options.find("port") == options.end()) {
            throw args::ValidationError("--input udp-server: missing `port` option");
        }
        cfg.listen = options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
        cfg.port   = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.stream_id = generate_stream_id("udp-server", options, unique);

    inputs.udp_server.push_back(std::move(cfg));
}

static void parse_interface(std::string const& source, Config* config) {
    auto parts = core::split(source, ':');
    if (parts.size() < 1 || parts.size() > 2) {
        throw args::ValidationError("--input: invalid input, got `" + source + "`");
    }

    auto type    = parts[0];
    auto options = parts.size() == 2 ? parse_options(parts[1]) : Options{};

    if (type == "stdin") return parse_stdin(options, config->inputs_config);
    if (type == "file") return parse_file(options, config->inputs_config);
    if (type == "stream") return parse_stream_ref(options, config->inputs_config);
    if (type == "serial") return parse_serial(options, config->inputs_config);
    if (type == "tcp-client") return parse_tcp_client(options, config->inputs_config);
    if (type == "tcp-server") return parse_tcp_server(options, config->inputs_config);
    if (type == "udp-server") return parse_udp_server(options, config->inputs_config);

    throw args::ValidationError("--input: invalid input type, got `" + type + "`");
}

void parse(Config* config) {
    config->inputs_config.disable_pipe_buffer_optimization = gDisablePipeBufferOptimization.Get();
    config->inputs_config.shutdown_on_complete             = gShutdownOnComplete.Get();
    config->inputs_config.shutdown_delay = std::chrono::milliseconds(gShutdownDelay.Get());

    for (auto const& input : gArgs.Get()) {
        parse_interface(input, config);
    }
}

void dump(ProgramInput const& config) {
    for (auto const& input : config.inputs) {
        DEBUGF("%p", input.interface.get());
        DEBUG_INDENT_SCOPE();
        DEBUGF("format: %s%s%s%s%s%s%s", (input.format & INPUT_FORMAT_UBX) ? "UBX " : "",
               (input.format & INPUT_FORMAT_NMEA) ? "NMEA " : "",
               (input.format & INPUT_FORMAT_RTCM) ? "RTCM " : "",
               (input.format & INPUT_FORMAT_CTRL) ? "CTRL " : "",
               (input.format & INPUT_FORMAT_LPP_UPER) ? "LPP-UPER " : "",
               (input.format & INPUT_FORMAT_LPP_UPER_PAD) ? "LPP-UPER-PAD " : "",
               (input.format & INPUT_FORMAT_RAW) ? "RAW " : "");
    }
}

void dump(InputsConfig const& config) {
    for (auto const& c : config.stream) {
        DEBUGF("stream: id=%s", c.stream_id.c_str());
    }
    for (auto const& c : config.stdin_inputs) {
        (void)c;
        DEBUGF("stdin");
    }
    for (auto const& c : config.file) {
        DEBUGF("file: path=%s", c.path.c_str());
    }
    for (auto const& c : config.serial) {
        DEBUGF("serial: stream_id=%s device=%s", c.stream_id.c_str(), c.config.device.c_str());
    }
    for (auto const& c : config.tcp_client) {
        DEBUGF("tcp-client: stream_id=%s host=%s port=%u", c.stream_id.c_str(),
               c.config.host.c_str(), c.config.port);
    }
    for (auto const& c : config.tcp_server) {
        DEBUGF("tcp-server: stream_id=%s listen=%s port=%u", c.stream_id.c_str(), c.listen.c_str(),
               c.port);
    }
    for (auto const& c : config.udp_server) {
        DEBUGF("udp-server: stream_id=%s listen=%s port=%u", c.stream_id.c_str(), c.listen.c_str(),
               c.port);
    }
}

}  // namespace input
