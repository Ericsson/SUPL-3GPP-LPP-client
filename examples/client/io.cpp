#include "io.hpp"

#include "config/parse.hpp"

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

#include "processor/chunked_log.hpp"

LOGLET_MODULE2(client, io);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(client, io)

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

void add_tcp_server(std::string const& id, StreamTcpServerConfig const& config,
                    io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] tcp-server stream: id=%s listen=%s port=%u", source, id.c_str(),
           config.listen.c_str(), config.port);
    std::unique_ptr<scheduler::SocketListenerTask> listener;
    if (!config.path.empty()) {
        listener = std::make_unique<scheduler::TcpUnixListenerTask>(config.path);
    } else {
        listener = std::make_unique<scheduler::TcpInetListenerTask>(config.listen, config.port);
    }
    registry.add(
        id, std::make_shared<io::TcpServerStream>(id, std::move(listener), config.read_config));
}

void add_udp_client(std::string const& id, io::UdpClientConfig const& config,
                    io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] udp-client stream: id=%s host=%s port=%u", source, id.c_str(), config.host.c_str(),
           config.port);
    registry.add(id, std::make_shared<io::UdpClientStream>(id, config));
}

void add_udp_server(std::string const& id, StreamUdpServerConfig const& config,
                    io::StreamRegistry& registry, char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] udp-server stream: id=%s listen=%s port=%u", source, id.c_str(),
           config.listen.c_str(), config.port);
    std::unique_ptr<scheduler::UdpSocketListenerTask> listener;
    if (!config.path.empty()) {
        listener = std::make_unique<scheduler::UdpUnixListenerTask>(config.path);
    } else {
        listener = std::make_unique<scheduler::UdpInetListenerTask>(config.listen, config.port);
    }
    registry.add(
        id, std::make_shared<io::UdpServerStream>(id, std::move(listener), config.read_config));
}

void add_stdio(std::string const& id, io::StdioConfig const& config, io::StreamRegistry& registry,
               char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] stdio stream: id=%s", source, id.c_str());
    registry.add(id, std::make_shared<io::StdioStream>(id, config));
}

void add_file(std::string const& id, io::FileConfig const& config, io::StreamRegistry& registry,
              char const* source) {
    if (registry.has(id)) return;
    DEBUGF("[%s] file stream: id=%s path=%s", source, id.c_str(), config.path.c_str());
    registry.add(id, std::make_shared<io::FileStream>(id, config));
}

}  // namespace

void create_streams(StreamsConfig const& config, io::StreamRegistry& registry) {
    for (auto const& cfg : config.serial) {
        add_serial(cfg.id, cfg.config, registry, "explicit");
    }
    for (auto const& cfg : config.tcp_client) {
        add_tcp_client(cfg.id, cfg.config, registry, "explicit");
    }
    for (auto const& cfg : config.tcp_server) {
        add_tcp_server(cfg.id, cfg, registry, "explicit");
    }
    for (auto const& cfg : config.udp_client) {
        add_udp_client(cfg.id, cfg.config, registry, "explicit");
    }
    for (auto const& cfg : config.udp_server) {
        add_udp_server(cfg.id, cfg, registry, "explicit");
    }
    for (auto const& cfg : config.pty) {
        if (registry.has(cfg.id)) continue;
        DEBUGF("[explicit] pty stream: id=%s link=%s", cfg.id.c_str(),
               cfg.config.link_path.c_str());
        registry.add(cfg.id, std::make_shared<io::PtyStream>(cfg.id, cfg.config));
    }
    for (auto const& cfg : config.stdio) {
        add_stdio(cfg.id, cfg.config, registry, "explicit");
    }
    for (auto const& cfg : config.fd) {
        if (registry.has(cfg.id)) continue;
        DEBUGF("[explicit] fd stream: id=%s fd=%d", cfg.id.c_str(), cfg.config.fd);
        registry.add(cfg.id, std::make_shared<io::FdStream>(cfg.id, cfg.config));
    }
    for (auto const& cfg : config.file) {
        add_file(cfg.id, cfg.config, registry, "explicit");
    }
}

void create_implicit_streams(InputsConfig const& inputs, OutputsConfig const& outputs,
                             io::StreamRegistry& registry) {
    // Input implicit streams (only shared stream types)
    for (auto const& cfg : inputs.serial) {
        add_serial(cfg.stream_id, cfg.config, registry, "implicit/input");
    }
    for (auto const& cfg : inputs.tcp_client) {
        add_tcp_client(cfg.stream_id, cfg.config, registry, "implicit/input");
    }
    for (auto const& cfg : inputs.tcp_server) {
        StreamTcpServerConfig stream_cfg;
        stream_cfg.id     = cfg.stream_id;
        stream_cfg.listen = cfg.listen;
        stream_cfg.port   = cfg.port;
        stream_cfg.path   = cfg.path;
        add_tcp_server(cfg.stream_id, stream_cfg, registry, "implicit/input");
    }
    for (auto const& cfg : inputs.udp_server) {
        StreamUdpServerConfig stream_cfg;
        stream_cfg.id     = cfg.stream_id;
        stream_cfg.listen = cfg.listen;
        stream_cfg.port   = cfg.port;
        stream_cfg.path   = cfg.path;
        add_udp_server(cfg.stream_id, stream_cfg, registry, "implicit/input");
    }

    // Output implicit streams (only shared stream types)
    for (auto const& cfg : outputs.serial) {
        add_serial(cfg.stream_id, cfg.config, registry, "implicit/output");
    }
    for (auto const& cfg : outputs.tcp_client) {
        add_tcp_client(cfg.stream_id, cfg.config, registry, "implicit/output");
    }
    for (auto const& cfg : outputs.udp_client) {
        add_udp_client(cfg.stream_id, cfg.config, registry, "implicit/output");
    }
}

// Input factories

std::unique_ptr<io::Input> create_input(InputStreamConfig const& cfg,
                                        io::StreamRegistry&      registry) {
    DEBUGF("input: stream=%s", cfg.stream_id.c_str());
    auto stream = registry.get(cfg.stream_id);
    if (!stream) {
        ERRORF("unknown stream id: %s", cfg.stream_id.c_str());
        return nullptr;
    }
    return std::make_unique<io::StreamInputAdapter>(stream);
}

static std::shared_ptr<io::Stream> get_or_create_stdio(io::StreamRegistry& registry) {
    static const std::string id = "stdio";
    auto                     existing = registry.get(id);
    if (existing) return existing;
    io::StdioConfig cfg;
    cfg.use_stderr = false;
    auto stream    = std::make_shared<io::StdioStream>(id, cfg);
    registry.add(id, stream);
    return stream;
}

std::unique_ptr<io::Input> create_input(InputStdinConfig const&, io::StreamRegistry& registry) {
    DEBUGF("input: stdin");
    return std::make_unique<io::StreamInputAdapter>(get_or_create_stdio(registry));
}

std::unique_ptr<io::Input> create_input(InputFileConfig const& cfg, io::StreamRegistry& registry) {
    DEBUGF("input: file=%s", cfg.path.c_str());
    auto           id = "file:" + cfg.path + ":" + generate_unique_id();
    io::FileConfig file_cfg;
    file_cfg.path           = cfg.path;
    file_cfg.read           = true;
    file_cfg.write          = false;
    file_cfg.bytes_per_tick = cfg.bytes_per_tick;
    file_cfg.tick_interval  = cfg.tick_interval;
    auto stream             = std::make_shared<io::FileStream>(id, file_cfg);
    registry.add(id, stream);
    return std::make_unique<io::StreamInputAdapter>(stream);
}

std::unique_ptr<io::Input> create_input(InputSerialConfig const& cfg,
                                        io::StreamRegistry&      registry) {
    DEBUGF("input: serial=%s", cfg.config.device.c_str());
    return std::make_unique<io::StreamInputAdapter>(registry.get(cfg.stream_id));
}

std::unique_ptr<io::Input> create_input(InputTcpClientConfig const& cfg,
                                        io::StreamRegistry&         registry) {
    DEBUGF("input: tcp-client=%s:%u", cfg.config.host.c_str(), cfg.config.port);
    return std::make_unique<io::StreamInputAdapter>(registry.get(cfg.stream_id));
}

std::unique_ptr<io::Input> create_input(InputTcpServerConfig const& cfg,
                                        io::StreamRegistry&         registry) {
    DEBUGF("input: tcp-server=%s:%u", cfg.listen.c_str(), cfg.port);
    return std::make_unique<io::StreamInputAdapter>(registry.get(cfg.stream_id));
}

std::unique_ptr<io::Input> create_input(InputUdpServerConfig const& cfg,
                                        io::StreamRegistry&         registry) {
    DEBUGF("input: udp-server=%s:%u", cfg.listen.c_str(), cfg.port);
    return std::make_unique<io::StreamInputAdapter>(registry.get(cfg.stream_id));
}

// Output factories

std::unique_ptr<io::Output> create_output(OutputStreamConfig const& cfg,
                                          io::StreamRegistry&       registry) {
    DEBUGF("output: stream=%s", cfg.stream_id.c_str());
    auto stream = registry.get(cfg.stream_id);
    if (!stream) {
        ERRORF("unknown stream id: %s", cfg.stream_id.c_str());
        return nullptr;
    }
    return std::make_unique<io::StreamOutputAdapter>(stream);
}

std::unique_ptr<io::Output> create_output(OutputStdoutConfig const&, io::StreamRegistry& registry) {
    DEBUGF("output: stdout");
    return std::make_unique<io::StreamOutputAdapter>(get_or_create_stdio(registry));
}

std::unique_ptr<io::Output> create_output(OutputFileConfig const& cfg,
                                          io::StreamRegistry&     registry) {
    DEBUGF("output: file=%s", cfg.path.c_str());
    auto           id = "file:" + cfg.path + ":" + generate_unique_id();
    io::FileConfig file_cfg;
    file_cfg.path     = cfg.path;
    file_cfg.read     = false;
    file_cfg.write    = true;
    file_cfg.append   = cfg.append;
    file_cfg.truncate = cfg.truncate;
    file_cfg.create   = true;
    auto stream       = std::make_shared<io::FileStream>(id, file_cfg);
    registry.add(id, stream);
    return std::make_unique<io::StreamOutputAdapter>(stream);
}

std::unique_ptr<io::Output> create_output(OutputChunkedLogConfig const& cfg) {
    DEBUGF("output: chunked-log=%s", cfg.path.c_str());
    return std::make_unique<ChunkedLogOutput>(cfg.path);
}

std::unique_ptr<io::Output> create_output(OutputTcpServerConfig const& cfg) {
    DEBUGF("output: tcp-server=%s:%u", cfg.listen.c_str(), cfg.port);
    if (!cfg.path.empty()) {
        return std::make_unique<io::TcpServerOutput>(cfg.path);
    }
    return std::make_unique<io::TcpServerOutput>(cfg.listen, cfg.port);
}

std::unique_ptr<io::Output> create_output(OutputSerialConfig const& cfg,
                                          io::StreamRegistry&       registry) {
    DEBUGF("output: serial=%s", cfg.config.device.c_str());
    return std::make_unique<io::StreamOutputAdapter>(registry.get(cfg.stream_id));
}

std::unique_ptr<io::Output> create_output(OutputTcpClientConfig const& cfg,
                                          io::StreamRegistry&          registry) {
    DEBUGF("output: tcp-client=%s:%u", cfg.config.host.c_str(), cfg.config.port);
    return std::make_unique<io::StreamOutputAdapter>(registry.get(cfg.stream_id));
}

std::unique_ptr<io::Output> create_output(OutputUdpClientConfig const& cfg,
                                          io::StreamRegistry&          registry) {
    DEBUGF("output: udp-client=%s:%u", cfg.config.host.c_str(), cfg.config.port);
    return std::make_unique<io::StreamOutputAdapter>(registry.get(cfg.stream_id));
}
