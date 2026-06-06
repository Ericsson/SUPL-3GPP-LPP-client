#include <client-io/io.hpp>
#include <client-io/parse.hpp>
#include <client-io/registry.hpp>
#include <client-io/tbin_output.hpp>

#include <cxx11_compat.hpp>
#include <io/adapters.hpp>
#include <io/file.hpp>
#include <io/stdin.hpp>
#include <io/stdout.hpp>
#include <io/stream/fd.hpp>
#include <io/stream/file.hpp>
#include <io/stream/pty.hpp>
#include <io/stream/serial.hpp>
#include <io/stream/stdio.hpp>
#include <io/stream/tcp_client.hpp>
#include <io/stream/tcp_server.hpp>
#include <io/stream/udp_client.hpp>
#include <io/stream/udp_server.hpp>
#include <io/tcp.hpp>
#include <loglet/loglet.hpp>
#include <scheduler/socket.hpp>

LOGLET_MODULE(client_io_wiring);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(client_io_wiring)

namespace {

void add_serial(std::string const& id, io::SerialConfig const& config, io::StreamRegistry& registry,
                char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] serial stream: id=%s device=%s", source, id.c_str(), config.device.c_str());
    registry.add(id, std::make_shared<io::SerialStream>(id, config));
}

void add_tcp_client(std::string const& id, io::TcpClientConfig const& config,
                    io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] tcp-client stream: id=%s host=%s port=%u", source, id.c_str(), config.host.c_str(),
           config.port);
    registry.add(id, std::make_shared<io::TcpClientStream>(id, config));
}

void add_tcp_server(std::string const& id, std::string const& listen, uint16_t port,
                    std::string const& path, io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] tcp-server stream: id=%s listen=%s port=%u", source, id.c_str(), listen.c_str(),
           port);
    std::unique_ptr<scheduler::SocketListenerTask> listener;
    if (!path.empty()) {
        listener = std::make_unique<scheduler::TcpUnixListenerTask>(path);
    } else {
        listener = std::make_unique<scheduler::TcpInetListenerTask>(listen, port);
    }
    registry.add(id, std::make_shared<io::TcpServerStream>(id, std::move(listener)));
}

void add_udp_client(std::string const& id, io::UdpClientConfig const& config,
                    io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] udp-client stream: id=%s host=%s port=%u", source, id.c_str(), config.host.c_str(),
           config.port);
    registry.add(id, std::make_shared<io::UdpClientStream>(id, config));
}

void add_udp_server(std::string const& id, std::string const& listen, uint16_t port,
                    std::string const& path, io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] udp-server stream: id=%s listen=%s port=%u", source, id.c_str(), listen.c_str(),
           port);
    std::unique_ptr<scheduler::UdpSocketListenerTask> listener;
    if (!path.empty()) {
        listener = std::make_unique<scheduler::UdpUnixListenerTask>(path);
    } else {
        listener = std::make_unique<scheduler::UdpInetListenerTask>(listen, port);
    }
    registry.add(id, std::make_shared<io::UdpServerStream>(id, std::move(listener)));
}

static std::shared_ptr<io::Stream> get_or_create_stdio(io::StreamRegistry& registry) {
    static std::string const id = "stdio";
    auto                     s  = registry.get(id);
    if (s) return s;
    io::StdioConfig cfg;
    cfg.use_stderr = false;
    auto stream    = std::make_shared<io::StdioStream>(id, cfg);
    registry.add(id, stream);
    return stream;
}

}  // namespace

void create_streams(StreamsConfig const& config, io::StreamRegistry& registry) {
    for (auto const& cfg : config.serial)
        add_serial(cfg.id, cfg.config, registry, "explicit");
    for (auto const& cfg : config.tcp_client)
        add_tcp_client(cfg.id, cfg.config, registry, "explicit");
    for (auto const& cfg : config.tcp_server)
        add_tcp_server(cfg.id, cfg.listen, cfg.port, cfg.path, registry, "explicit");
    for (auto const& cfg : config.udp_client)
        add_udp_client(cfg.id, cfg.config, registry, "explicit");
    for (auto const& cfg : config.udp_server)
        add_udp_server(cfg.id, cfg.listen, cfg.port, cfg.path, registry, "explicit");
    for (auto const& cfg : config.pty) {
        if (registry.has(cfg.id)) continue;
        registry.add(cfg.id, std::make_shared<io::PtyStream>(cfg.id, cfg.config));
    }
    for (auto const& cfg : config.stdio) {
        if (registry.has(cfg.id)) continue;
        registry.add(cfg.id, std::make_shared<io::StdioStream>(cfg.id, cfg.config));
    }
    for (auto const& cfg : config.fd) {
        if (registry.has(cfg.id)) continue;
        registry.add(cfg.id, std::make_shared<io::FdStream>(cfg.id, cfg.config));
    }
    for (auto const& cfg : config.file) {
        if (registry.has(cfg.id)) continue;
        registry.add(cfg.id, std::make_shared<io::FileStream>(cfg.id, cfg.config));
    }
}

std::unique_ptr<io::Input> create_input(InputEntry const& entry, io::StreamRegistry& registry) {
    for (auto const& handler : io_registry::input_types()) {
        if (handler.name == entry.type) {
            return handler.create(entry.options, registry);
        }
    }
    ERRORF("unknown input type: %s", entry.type.c_str());
    return nullptr;
}

std::unique_ptr<io::Output> create_output(OutputEntry const& entry, io::StreamRegistry& registry) {
    for (auto const& handler : io_registry::output_types()) {
        if (handler.name == entry.type) {
            return handler.create(entry.options, registry);
        }
    }
    ERRORF("unknown output type: %s", entry.type.c_str());
    return nullptr;
}

// ── Built-in type handler factories ───────────────────────────────────────────
// These are called by each binary to register the types it wants to support.

using Opts = io_registry::Options;

static bool opt(Opts const& o, std::string const& k, bool def) {
    auto it = o.find(k);
    if (it == o.end()) return def;
    if (it->second.empty() || it->second == "true") return true;
    return false;
}

static std::string get(Opts const& o, std::string const& k, std::string const& def = "") {
    auto it = o.find(k);
    return it == o.end() ? def : it->second;
}

static uint16_t get_port(Opts const& o, std::string const& k) {
    return static_cast<uint16_t>(std::stoul(o.at(k)));
}

static std::string stream_id(std::string const& prefix, Opts const& o) {
    bool        unique = opt(o, "unique", false);
    std::string id;
    if (o.count("path"))
        id = prefix + ":" + o.at("path");
    else if (o.count("host"))
        id = prefix + ":" + o.at("host") + ":" + get(o, "port");
    else if (o.count("listen"))
        id = prefix + ":" + o.at("listen") + ":" + get(o, "port");
    else if (o.count("device"))
        id = prefix + ":" + o.at("device");
    else
        id = prefix;
    return unique ? id + "#" + generate_unique_id() : id;
}

io_registry::InputTypeHandler make_stdin_input_type() {
    return {"stdin", "", [](Opts const&, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                return std::make_unique<io::StreamInputAdapter>(get_or_create_stdio(r));
            }};
}

io_registry::InputTypeHandler make_file_input_type() {
    return {"file", "    path=<path>\n    bps=<bytes_per_sec>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                if (!o.count("path")) throw std::runtime_error("--input file: missing path");
                auto           id = "file:" + o.at("path") + ":" + generate_unique_id();
                io::FileConfig cfg;
                cfg.path  = o.at("path");
                cfg.read  = true;
                cfg.write = false;
                if (o.count("bps")) {
                    auto bps           = std::stoull(o.at("bps"));
                    cfg.bytes_per_tick = static_cast<size_t>((bps + 9) / 10);
                }
                auto stream = std::make_shared<io::FileStream>(id, cfg);
                r.add(id, stream);
                return std::make_unique<io::StreamInputAdapter>(stream);
            }};
}

io_registry::InputTypeHandler make_serial_input_type() {
    return {"serial", "    device=<device>\n    baudrate=<baudrate>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                if (!o.count("device")) throw std::runtime_error("--input serial: missing device");
                auto sid = stream_id("serial", o);
                if (!r.has(sid)) {
                    io::SerialConfig cfg;
                    cfg.device = o.at("device");
                    if (o.count("baudrate")) cfg.baud_rate = parse_baudrate(o.at("baudrate"));
                    if (o.count("databits")) cfg.data_bits = parse_databits(o.at("databits"));
                    if (o.count("stopbits")) cfg.stop_bits = parse_stopbits(o.at("stopbits"));
                    if (o.count("parity")) cfg.parity_bit = parse_paritybit(o.at("parity"));
                    r.add(sid, std::make_shared<io::SerialStream>(sid, cfg));
                }
                return std::make_unique<io::StreamInputAdapter>(r.get(sid));
            }};
}

io_registry::InputTypeHandler make_tcp_client_input_type() {
    return {"tcp-client", "    host=<host>\n    port=<port>\n    reconnect=<bool>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                auto sid = stream_id("tcp-client", o);
                if (!r.has(sid)) {
                    io::TcpClientConfig cfg;
                    if (o.count("path"))
                        cfg.path = o.at("path");
                    else {
                        cfg.host = o.at("host");
                        cfg.port = get_port(o, "port");
                    }
                    cfg.reconnect = opt(o, "reconnect", true);
                    r.add(sid, std::make_shared<io::TcpClientStream>(sid, cfg));
                }
                return std::make_unique<io::StreamInputAdapter>(r.get(sid));
            }};
}

io_registry::InputTypeHandler make_tcp_server_input_type() {
    return {"tcp-server", "    listen=<addr>\n    port=<port>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                auto sid = stream_id("tcp-server", o);
                if (!r.has(sid)) {
                    auto listen = get(o, "listen", "0.0.0.0");
                    auto port   = get_port(o, "port");
                    auto path   = get(o, "path");
                    add_tcp_server(sid, listen, port, path, r, "input");
                }
                return std::make_unique<io::StreamInputAdapter>(r.get(sid));
            }};
}

io_registry::InputTypeHandler make_udp_server_input_type() {
    return {"udp-server", "    listen=<addr>\n    port=<port>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                auto sid = stream_id("udp-server", o);
                if (!r.has(sid)) {
                    auto listen = get(o, "listen", "0.0.0.0");
                    auto port   = get_port(o, "port");
                    auto path   = get(o, "path");
                    add_udp_server(sid, listen, port, path, r, "input");
                }
                return std::make_unique<io::StreamInputAdapter>(r.get(sid));
            }};
}

io_registry::InputTypeHandler make_stream_ref_input_type() {
    return {"stream", "    id=<stream_id>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Input> {
                if (!o.count("id")) throw std::runtime_error("--input stream: missing id");
                auto s = r.get(o.at("id"));
                if (!s) {
                    ERRORF("unknown stream id: %s", o.at("id").c_str());
                    return nullptr;
                }
                return std::make_unique<io::StreamInputAdapter>(s);
            }};
}

io_registry::OutputTypeHandler make_stdout_output_type() {
    return {"stdout", "", [](Opts const&, io::StreamRegistry& r) -> std::unique_ptr<io::Output> {
                return std::make_unique<io::StreamOutputAdapter>(get_or_create_stdio(r));
            }};
}

io_registry::OutputTypeHandler make_file_output_type() {
    return {"file", "    path=<path>\n    append=<bool>\n    tbin=<bool>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Output> {
                if (!o.count("path")) throw std::runtime_error("--output file: missing path");
                auto           id = "file:" + o.at("path") + ":" + generate_unique_id();
                io::FileConfig cfg;
                cfg.path     = o.at("path");
                cfg.read     = false;
                cfg.write    = true;
                cfg.append   = opt(o, "append", false);
                cfg.truncate = !cfg.append;
                cfg.create   = true;
                auto stream  = std::make_shared<io::FileStream>(id, cfg);
                r.add(id, stream);
                if (opt(o, "tbin", false)) {
                    return std::make_unique<TbinOutput>(
                        std::make_unique<io::StreamOutputAdapter>(stream), cfg.path);
                }
                return std::make_unique<io::StreamOutputAdapter>(stream);
            }};
}

io_registry::OutputTypeHandler make_tcp_server_output_type() {
    return {"tcp-server", "    host=<addr>\n    port=<port>\n    tbin=<bool>\n",
            [](Opts const& o, io::StreamRegistry&) -> std::unique_ptr<io::Output> {
                std::unique_ptr<io::Output> inner;
                if (o.count("path")) {
                    inner = std::make_unique<io::TcpServerOutput>(o.at("path"));
                } else {
                    auto listen = get(o, "host", "0.0.0.0");
                    auto port   = get_port(o, "port");
                    inner       = std::make_unique<io::TcpServerOutput>(listen, port);
                }
                if (opt(o, "tbin", false)) {
                    auto name = get(o, "port");
                    return std::make_unique<TbinOutput>(std::move(inner), name);
                }
                return inner;
            }};
}

io_registry::OutputTypeHandler make_serial_output_type() {
    return {"serial", "    device=<device>\n    baudrate=<baudrate>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Output> {
                if (!o.count("device")) throw std::runtime_error("--output serial: missing device");
                auto sid = stream_id("serial", o);
                if (!r.has(sid)) {
                    io::SerialConfig cfg;
                    cfg.device = o.at("device");
                    if (o.count("baudrate")) cfg.baud_rate = parse_baudrate(o.at("baudrate"));
                    r.add(sid, std::make_shared<io::SerialStream>(sid, cfg));
                }
                return std::make_unique<io::StreamOutputAdapter>(r.get(sid));
            }};
}

io_registry::OutputTypeHandler make_tcp_client_output_type() {
    return {"tcp-client", "    host=<host>\n    port=<port>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Output> {
                auto sid = stream_id("tcp-client", o);
                if (!r.has(sid)) {
                    io::TcpClientConfig cfg;
                    if (o.count("path"))
                        cfg.path = o.at("path");
                    else {
                        cfg.host = o.at("host");
                        cfg.port = get_port(o, "port");
                    }
                    cfg.reconnect = opt(o, "reconnect", true);
                    r.add(sid, std::make_shared<io::TcpClientStream>(sid, cfg));
                }
                return std::make_unique<io::StreamOutputAdapter>(r.get(sid));
            }};
}

io_registry::OutputTypeHandler make_udp_client_output_type() {
    return {"udp-client", "    host=<host>\n    port=<port>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Output> {
                auto sid = stream_id("udp-client", o);
                if (!r.has(sid)) {
                    io::UdpClientConfig cfg;
                    if (o.count("path"))
                        cfg.path = o.at("path");
                    else {
                        cfg.host = o.at("host");
                        cfg.port = get_port(o, "port");
                    }
                    r.add(sid, std::make_shared<io::UdpClientStream>(sid, cfg));
                }
                return std::make_unique<io::StreamOutputAdapter>(r.get(sid));
            }};
}

io_registry::OutputTypeHandler make_stream_ref_output_type() {
    return {"stream", "    id=<stream_id>\n",
            [](Opts const& o, io::StreamRegistry& r) -> std::unique_ptr<io::Output> {
                if (!o.count("id")) throw std::runtime_error("--output stream: missing id");
                auto s = r.get(o.at("id"));
                if (!s) {
                    ERRORF("unknown stream id: %s", o.at("id").c_str());
                    return nullptr;
                }
                return std::make_unique<io::StreamOutputAdapter>(s);
            }};
}
