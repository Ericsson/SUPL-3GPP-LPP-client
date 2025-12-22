#include "parse.hpp"
#include "types.hpp"

#include <core/string.hpp>
#include <loglet/loglet.hpp>

#include "../config.hpp"

#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace stream {

static args::Group                      gGroup{"Stream:"};
static args::ValueFlagList<std::string> gArgs{
    gGroup,
    "stream",
    "Declare a bidirectional stream.\n"
    "Usage: --stream <type>:<arguments>\n\n"
    "Common arguments:\n"
    "  id=<name>                  Required: unique identifier\n"
    "  read_min_bytes=<N>         Read buffering: min bytes\n"
    "  read_timeout_ms=<N>        Read buffering: flush interval\n\n"
    "Types:\n"
    "  serial:\n"
    "    device=<device>\n"
    "    baudrate=<baudrate>\n"
    "    databits=<5|6|7|8>\n"
    "    stopbits=<1|2>\n"
    "    parity=<none|odd|even>\n"
    "    raw=<bool>\n"
    "  tcp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "    reconnect=<bool>\n"
    "  tcp-server:\n"
    "    listen=<addr>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  udp-client:\n"
    "    host=<host>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  udp-server:\n"
    "    listen=<addr>\n"
    "    port=<port>\n"
    "    path=<path>\n"
    "  pty:\n"
    "    link=<path>\n"
    "  stdio:\n"
    "    stderr=<bool>\n"
    "  fd:\n"
    "    fd=<number>\n"
    "  file:\n"
    "    path=<path>\n"
    "    read=<bool>\n"
    "    write=<bool>\n"
    "    append=<bool>\n"
    "    truncate=<bool>\n"
    "    bytes_per_tick=<N>\n"
    "    tick_interval_ms=<N>\n",
    {"stream"},
};

void setup(args::ArgumentParser& parser) {
    static args::GlobalOptions sGlobals{parser, gGroup};
}

static void parse_serial(std::string const& id, Options const& options, StreamsConfig& streams) {
    if (options.find("device") == options.end()) {
        throw args::ValidationError("--stream serial: missing `device` option");
    }

    StreamSerialConfig cfg;
    cfg.id            = id;
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
    cfg.config.raw         = parse_bool(options, "raw", false);
    cfg.config.read_config = parse_read_config(options);

    streams.serial.push_back(std::move(cfg));
}

static void parse_tcp_client(std::string const& id, Options const& options,
                             StreamsConfig& streams) {
    StreamTcpClientConfig cfg;
    cfg.id               = id;
    cfg.config.reconnect = parse_bool(options, "reconnect", true);

    if (options.find("path") != options.end()) {
        cfg.config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--stream tcp-client: missing `host` or `port` option");
        }
        cfg.config.host = options.at("host");
        cfg.config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.config.read_config = parse_read_config(options);

    streams.tcp_client.push_back(std::move(cfg));
}

static void parse_tcp_server(std::string const& id, Options const& options,
                             StreamsConfig& streams) {
    StreamTcpServerConfig cfg;
    cfg.id = id;

    if (options.find("path") != options.end()) {
        cfg.path = options.at("path");
    } else {
        cfg.listen = options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
        if (options.find("port") == options.end()) {
            throw args::ValidationError("--stream tcp-server: missing `port` option");
        }
        cfg.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.read_config = parse_read_config(options);

    streams.tcp_server.push_back(std::move(cfg));
}

static void parse_udp_client(std::string const& id, Options const& options,
                             StreamsConfig& streams) {
    StreamUdpClientConfig cfg;
    cfg.id = id;

    if (options.find("path") != options.end()) {
        cfg.config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--stream udp-client: missing `host` or `port` option");
        }
        cfg.config.host = options.at("host");
        cfg.config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.config.read_config = parse_read_config(options);

    streams.udp_client.push_back(std::move(cfg));
}

static void parse_udp_server(std::string const& id, Options const& options,
                             StreamsConfig& streams) {
    StreamUdpServerConfig cfg;
    cfg.id = id;

    if (options.find("path") != options.end()) {
        cfg.path = options.at("path");
    } else {
        cfg.listen = options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
        if (options.find("port") == options.end()) {
            throw args::ValidationError("--stream udp-server: missing `port` option");
        }
        cfg.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }
    cfg.read_config = parse_read_config(options);

    streams.udp_server.push_back(std::move(cfg));
}

static void parse_pty(std::string const& id, Options const& options, StreamsConfig& streams) {
    StreamPtyConfig cfg;
    cfg.id = id;
    if (options.find("link") != options.end()) {
        cfg.config.link_path = options.at("link");
    }
    cfg.config.raw         = parse_bool(options, "raw", false);
    cfg.config.read_config = parse_read_config(options);

    streams.pty.push_back(std::move(cfg));
}

static void parse_stdio(std::string const& id, Options const& options, StreamsConfig& streams) {
    StreamStdioConfig cfg;
    cfg.id                 = id;
    cfg.config.use_stderr  = parse_bool(options, "stderr", false);
    cfg.config.read_config = parse_read_config(options);

    streams.stdio.push_back(std::move(cfg));
}

static void parse_fd(std::string const& id, Options const& options, StreamsConfig& streams) {
    if (options.find("fd") == options.end()) {
        throw args::ValidationError("--stream fd: missing `fd` option");
    }

    StreamFdConfig cfg;
    cfg.id                 = id;
    cfg.config.fd          = std::stoi(options.at("fd"));
    cfg.config.owns_fd     = false;
    cfg.config.read_config = parse_read_config(options);

    streams.fd.push_back(std::move(cfg));
}

static void parse_file(std::string const& id, Options const& options, StreamsConfig& streams) {
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--stream file: missing `path` option");
    }

    StreamFileConfig cfg;
    cfg.id                 = id;
    cfg.config.path        = options.at("path");
    cfg.config.read        = parse_bool(options, "read", true);
    cfg.config.write       = parse_bool(options, "write", false);
    cfg.config.append      = parse_bool(options, "append", false);
    cfg.config.truncate    = parse_bool(options, "truncate", false);
    cfg.config.create      = parse_bool(options, "create", true);
    cfg.config.read_config = parse_read_config(options);

    if (options.find("bytes_per_tick") != options.end()) {
        cfg.config.bytes_per_tick = std::stoull(options.at("bytes_per_tick"));
    }
    if (options.find("tick_interval_ms") != options.end()) {
        cfg.config.tick_interval =
            std::chrono::milliseconds(std::stoll(options.at("tick_interval_ms")));
    }

    streams.file.push_back(std::move(cfg));
}

static void parse_stream(std::string const& type, Options const& options, StreamsConfig& streams) {
    if (options.find("id") == options.end()) {
        throw args::ValidationError("--stream " + type + ": missing `id` option");
    }
    auto id = options.at("id");

    if (type == "serial") return parse_serial(id, options, streams);
    if (type == "tcp-client") return parse_tcp_client(id, options, streams);
    if (type == "tcp-server") return parse_tcp_server(id, options, streams);
    if (type == "udp-client") return parse_udp_client(id, options, streams);
    if (type == "udp-server") return parse_udp_server(id, options, streams);
    if (type == "pty") return parse_pty(id, options, streams);
    if (type == "stdio") return parse_stdio(id, options, streams);
    if (type == "fd") return parse_fd(id, options, streams);
    if (type == "file") return parse_file(id, options, streams);

    throw args::ValidationError("--stream: unknown type `" + type + "`");
}

void parse(Config* config) {
    for (auto const& arg : gArgs) {
        auto colon = arg.find(':');
        if (colon == std::string::npos) {
            throw args::ValidationError("--stream: invalid format, expected `<type>:<options>`");
        }

        auto type    = arg.substr(0, colon);
        auto options = parse_options(arg.substr(colon + 1));
        parse_stream(type, options, config->streams_config);
    }
}

void dump(StreamsConfig const& config) {
    for (auto const& c : config.serial) {
        DEBUGF("serial: id=%s device=%s", c.id.c_str(), c.config.device.c_str());
    }
    for (auto const& c : config.tcp_client) {
        DEBUGF("tcp-client: id=%s host=%s port=%u", c.id.c_str(), c.config.host.c_str(),
               c.config.port);
    }
    for (auto const& c : config.tcp_server) {
        DEBUGF("tcp-server: id=%s listen=%s port=%u", c.id.c_str(), c.listen.c_str(), c.port);
    }
    for (auto const& c : config.udp_client) {
        DEBUGF("udp-client: id=%s host=%s port=%u", c.id.c_str(), c.config.host.c_str(),
               c.config.port);
    }
    for (auto const& c : config.udp_server) {
        DEBUGF("udp-server: id=%s listen=%s port=%u", c.id.c_str(), c.listen.c_str(), c.port);
    }
    for (auto const& c : config.pty) {
        DEBUGF("pty: id=%s link=%s", c.id.c_str(), c.config.link_path.c_str());
    }
    for (auto const& c : config.stdio) {
        DEBUGF("stdio: id=%s stderr=%s", c.id.c_str(), c.config.use_stderr ? "true" : "false");
    }
    for (auto const& c : config.fd) {
        DEBUGF("fd: id=%s fd=%d", c.id.c_str(), c.config.fd);
    }
    for (auto const& c : config.file) {
        DEBUGF("file: id=%s path=%s", c.id.c_str(), c.config.path.c_str());
    }
}

}  // namespace stream
