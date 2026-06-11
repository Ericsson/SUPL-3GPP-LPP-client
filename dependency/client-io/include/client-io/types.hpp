#pragma once
#include <io/stream.hpp>
#include <io/stream/fd.hpp>
#include <io/stream/file.hpp>
#include <io/stream/pty.hpp>
#include <io/stream/serial.hpp>
#include <io/stream/stdio.hpp>
#include <io/stream/tcp_client.hpp>
#include <io/stream/udp_client.hpp>

#include <client-io/input_format.hpp>
#include <client-io/output_format.hpp>

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

// ── Stream configurations (from --stream) — unchanged ────────────────────────

struct StreamSerialConfig {
    std::string      id;
    io::SerialConfig config;
};

struct StreamTcpClientConfig {
    std::string         id;
    io::TcpClientConfig config;
};

struct StreamTcpServerConfig {
    std::string          id;
    io::ReadBufferConfig read_config;
    std::string          listen;
    uint16_t             port = 0;
    std::string          path;
};

struct StreamUdpClientConfig {
    std::string         id;
    io::UdpClientConfig config;
};

struct StreamUdpServerConfig {
    std::string          id;
    io::ReadBufferConfig read_config;
    std::string          listen;
    uint16_t             port = 0;
    std::string          path;
};

struct StreamPtyConfig {
    std::string   id;
    io::PtyConfig config;
};

struct StreamStdioConfig {
    std::string     id;
    io::StdioConfig config;
};

struct StreamFdConfig {
    std::string  id;
    io::FdConfig config;
};

struct StreamFileConfig {
    std::string    id;
    io::FileConfig config;
};

struct StreamsConfig {
    std::vector<StreamSerialConfig>    serial;
    std::vector<StreamTcpClientConfig> tcp_client;
    std::vector<StreamTcpServerConfig> tcp_server;
    std::vector<StreamUdpClientConfig> udp_client;
    std::vector<StreamUdpServerConfig> udp_server;
    std::vector<StreamPtyConfig>       pty;
    std::vector<StreamStdioConfig>     stdio;
    std::vector<StreamFdConfig>        fd;
    std::vector<StreamFileConfig>      file;
};

// ── Generic input/output entries ──────────────────────────────────────────────

struct InputEntry {
    std::string                                  type;
    InputFormat                                  format = INPUT_FORMAT_NONE;
    std::vector<std::string>                     tags;
    std::unordered_map<std::string, std::string> options;
    bool                                         exclude_from_shutdown = false;
    bool                                         print                 = false;
    bool                                         nmea_lf_only          = false;
    bool                                         discard_errors        = false;
    bool                                         discard_unknowns      = false;
};

struct InputsConfig {
    std::vector<InputEntry>   inputs;
    bool                      shutdown_on_complete             = false;
    bool                      sync_mode                        = false;
    bool                      disable_pipe_buffer_optimization = false;
    std::chrono::milliseconds shutdown_delay                   = std::chrono::milliseconds(1000);
};

struct OutputEntry {
    std::string                                  type;
    OutputFormat                                 format = OUTPUT_FORMAT_NONE;
    std::vector<std::string>                     include_tags;
    std::vector<std::string>                     exclude_tags;
    std::unordered_map<std::string, std::string> options;
};

struct OutputsConfig {
    std::vector<OutputEntry> outputs;
};
