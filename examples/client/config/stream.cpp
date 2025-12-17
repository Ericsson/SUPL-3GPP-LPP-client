#include <core/string.hpp>
#include <cxx11_compat.hpp>
#include <io/adapters.hpp>
#include <io/registry.hpp>
#include <io/stream/fd.hpp>
#include <io/stream/file.hpp>
#include <io/stream/pty.hpp>
#include <io/stream/serial.hpp>
#include <io/stream/stdio.hpp>
#include <io/stream/tcp_client.hpp>
#include <io/stream/tcp_server.hpp>
#include <io/stream/udp_client.hpp>
#include <io/stream/udp_server.hpp>
#include <loglet/loglet.hpp>
#include "../config.hpp"

#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, config)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-dtor"
#pragma GCC diagnostic ignored "-Wnewline-eof"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Winconsistent-missing-destructor-override"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wshadow-field"
#pragma GCC diagnostic ignored "-Wsuggest-destructor-override"
#include <args.hxx>
#pragma GCC diagnostic pop

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

static std::unordered_map<std::string, std::string> parse_options(std::string const& str) {
    auto                                         parts = core::split(str, ',');
    std::unordered_map<std::string, std::string> options;
    for (auto const& part : parts) {
        auto kv = core::split(part, '=');
        if (kv.size() == 1) {
            options[kv[0]] = "";
        } else if (kv.size() == 2) {
            options[kv[0]] = kv[1];
        } else {
            throw args::ValidationError("--stream: invalid option, got `" + part + "`");
        }
    }
    return options;
}

static bool parse_bool(std::unordered_map<std::string, std::string> const& options,
                       std::string const& key, bool default_value) {
    if (options.find(key) == options.end()) return default_value;
    auto value = options.at(key);
    if (value.empty()) return true;
    if (value == "true") return true;
    if (value == "false") return false;
    throw args::ValidationError("--stream: `" + key + "` must be a boolean");
}

static io::ReadBufferConfig
parse_read_config(std::unordered_map<std::string, std::string> const& options) {
    io::ReadBufferConfig config;
    if (options.find("read_min_bytes") != options.end()) {
        config.min_bytes = std::stoull(options.at("read_min_bytes"));
    }
    if (options.find("read_timeout_ms") != options.end()) {
        config.timeout = std::chrono::milliseconds(std::stoll(options.at("read_timeout_ms")));
    }
    return config;
}

static io::BaudRate parse_baudrate(std::string const& str) {
    long baud_rate = std::stol(str);
    if (baud_rate == 9600) return io::BaudRate::BR9600;
    if (baud_rate == 19200) return io::BaudRate::BR19200;
    if (baud_rate == 38400) return io::BaudRate::BR38400;
    if (baud_rate == 57600) return io::BaudRate::BR57600;
    if (baud_rate == 115200) return io::BaudRate::BR115200;
    if (baud_rate == 230400) return io::BaudRate::BR230400;
    if (baud_rate == 460800) return io::BaudRate::BR460800;
    if (baud_rate == 921600) return io::BaudRate::BR921600;
    throw args::ValidationError("--stream serial: unsupported baudrate `" + str + "`");
}

static std::shared_ptr<io::Stream>
parse_serial(std::string const& id, std::unordered_map<std::string, std::string> const& options) {
    if (options.find("device") == options.end()) {
        throw args::ValidationError("--stream serial: missing `device` option");
    }

    io::SerialConfig config;
    config.device      = options.at("device");
    config.read_config = parse_read_config(options);
    if (options.find("baudrate") != options.end()) {
        config.baud_rate = parse_baudrate(options.at("baudrate"));
    }
    config.raw = parse_bool(options, "raw", false);

    return std::make_shared<io::SerialStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_tcp_client(std::string const&                                  id,
                 std::unordered_map<std::string, std::string> const& options) {
    io::TcpClientConfig config;
    config.reconnect   = parse_bool(options, "reconnect", true);
    config.read_config = parse_read_config(options);

    if (options.find("path") != options.end()) {
        config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--stream tcp-client: missing `host` or `port` option");
        }
        config.host = options.at("host");
        config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }

    return std::make_shared<io::TcpClientStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_tcp_server(std::string const&                                  id,
                 std::unordered_map<std::string, std::string> const& options) {
    io::TcpServerConfig config;
    config.read_config = parse_read_config(options);

    if (options.find("path") != options.end()) {
        config.path = options.at("path");
    } else {
        config.listen = options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
        if (options.find("port") == options.end()) {
            throw args::ValidationError("--stream tcp-server: missing `port` option");
        }
        config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }

    return std::make_shared<io::TcpServerStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_udp_client(std::string const&                                  id,
                 std::unordered_map<std::string, std::string> const& options) {
    io::UdpClientConfig config;
    config.read_config = parse_read_config(options);

    if (options.find("path") != options.end()) {
        config.path = options.at("path");
    } else {
        if (options.find("host") == options.end() || options.find("port") == options.end()) {
            throw args::ValidationError("--stream udp-client: missing `host` or `port` option");
        }
        config.host = options.at("host");
        config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }

    return std::make_shared<io::UdpClientStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_udp_server(std::string const&                                  id,
                 std::unordered_map<std::string, std::string> const& options) {
    io::UdpServerConfig config;
    config.read_config = parse_read_config(options);

    if (options.find("path") != options.end()) {
        config.path = options.at("path");
    } else {
        config.listen = options.find("listen") != options.end() ? options.at("listen") : "0.0.0.0";
        if (options.find("port") == options.end()) {
            throw args::ValidationError("--stream udp-server: missing `port` option");
        }
        config.port = static_cast<uint16_t>(std::stoul(options.at("port")));
    }

    return std::make_shared<io::UdpServerStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_pty(std::string const& id, std::unordered_map<std::string, std::string> const& options) {
    io::PtyConfig config;
    config.link_path   = options.find("link") != options.end() ? options.at("link") : "";
    config.raw         = parse_bool(options, "raw", false);
    config.read_config = parse_read_config(options);
    return std::make_shared<io::PtyStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_stdio(std::string const& id, std::unordered_map<std::string, std::string> const& options) {
    io::StdioConfig config;
    config.use_stderr  = parse_bool(options, "stderr", false);
    config.read_config = parse_read_config(options);
    return std::make_shared<io::StdioStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_fd(std::string const& id, std::unordered_map<std::string, std::string> const& options) {
    if (options.find("fd") == options.end()) {
        throw args::ValidationError("--stream fd: missing `fd` option");
    }
    io::FdConfig config;
    config.fd          = std::stoi(options.at("fd"));
    config.owns_fd     = false;
    config.read_config = parse_read_config(options);
    return std::make_shared<io::FdStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_file(std::string const& id, std::unordered_map<std::string, std::string> const& options) {
    if (options.find("path") == options.end()) {
        throw args::ValidationError("--stream file: missing `path` option");
    }

    io::FileConfig config;
    config.path        = options.at("path");
    config.read        = parse_bool(options, "read", true);
    config.write       = parse_bool(options, "write", false);
    config.append      = parse_bool(options, "append", false);
    config.truncate    = parse_bool(options, "truncate", false);
    config.create      = parse_bool(options, "create", true);
    config.read_config = parse_read_config(options);

    if (options.find("bytes_per_tick") != options.end()) {
        config.bytes_per_tick = std::stoull(options.at("bytes_per_tick"));
    }
    if (options.find("tick_interval_ms") != options.end()) {
        config.tick_interval =
            std::chrono::milliseconds(std::stoll(options.at("tick_interval_ms")));
    }

    return std::make_shared<io::FileStream>(id, config);
}

static std::shared_ptr<io::Stream>
parse_stream(std::string const& type, std::unordered_map<std::string, std::string> const& options) {
    if (options.find("id") == options.end()) {
        throw args::ValidationError("--stream " + type + ": missing `id` option");
    }
    auto id = options.at("id");

    if (type == "serial") return parse_serial(id, options);
    if (type == "tcp-client") return parse_tcp_client(id, options);
    if (type == "tcp-server") return parse_tcp_server(id, options);
    if (type == "udp-client") return parse_udp_client(id, options);
    if (type == "udp-server") return parse_udp_server(id, options);
    if (type == "pty") return parse_pty(id, options);
    if (type == "stdio") return parse_stdio(id, options);
    if (type == "fd") return parse_fd(id, options);
    if (type == "file") return parse_file(id, options);

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
        auto stream  = parse_stream(type, options);

        if (config->stream_registry.has(stream->id())) {
            throw args::ValidationError("--stream: duplicate stream id `" + stream->id() + "`");
        }

        config->stream_registry.add(stream->id(), stream);
    }
}

}  // namespace stream
