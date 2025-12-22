#pragma once
#include <io/stream.hpp>
#include <io/stream/fd.hpp>
#include <io/stream/file.hpp>
#include <io/stream/pty.hpp>
#include <io/stream/serial.hpp>
#include <io/stream/stdio.hpp>
#include <io/stream/tcp_client.hpp>
#include <io/stream/udp_client.hpp>

#include "../input_format.hpp"
#include "../output_format.hpp"

#include <chrono>
#include <string>
#include <vector>

// Stream configurations (from --stream)
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

// Input common options
struct InputCommon {
    InputFormat              format = INPUT_FORMAT_NONE;
    std::vector<std::string> tags;
    std::vector<std::string> stages;
    bool                     print                 = false;
    bool                     nmea_lf_only          = false;
    bool                     discard_errors        = false;
    bool                     discard_unknowns      = false;
    bool                     exclude_from_shutdown = false;
};

// Input: reference to --stream
struct InputStreamConfig : InputCommon {
    std::string stream_id;
};

// Input: stdin
struct InputStdinConfig : InputCommon {};

// Input: file (legacy io::Input)
struct InputFileConfig : InputCommon {
    std::string               path;
    size_t                    bytes_per_tick = 128;
    std::chrono::milliseconds tick_interval  = std::chrono::milliseconds(100);
};

// Input: inline serial (creates stream)
struct InputSerialConfig : InputCommon {
    std::string      stream_id;
    io::SerialConfig config;
};

// Input: inline tcp-client (creates stream)
struct InputTcpClientConfig : InputCommon {
    std::string         stream_id;
    io::TcpClientConfig config;
};

// Input: inline tcp-server (creates stream)
struct InputTcpServerConfig : InputCommon {
    std::string stream_id;
    std::string listen;
    uint16_t    port = 0;
    std::string path;
};

// Input: inline udp-server (creates stream)
struct InputUdpServerConfig : InputCommon {
    std::string stream_id;
    std::string listen;
    uint16_t    port = 0;
    std::string path;
};

struct InputsConfig {
    std::vector<InputStreamConfig>    stream;
    std::vector<InputStdinConfig>     stdin_inputs;
    std::vector<InputFileConfig>      file;
    std::vector<InputSerialConfig>    serial;
    std::vector<InputTcpClientConfig> tcp_client;
    std::vector<InputTcpServerConfig> tcp_server;
    std::vector<InputUdpServerConfig> udp_server;

    bool                      disable_pipe_buffer_optimization = false;
    bool                      shutdown_on_complete             = false;
    std::chrono::milliseconds shutdown_delay                   = std::chrono::milliseconds(1000);
};

// Output common options
struct OutputCommon {
    OutputFormat             format = OUTPUT_FORMAT_NONE;
    std::vector<std::string> include_tags;
    std::vector<std::string> exclude_tags;
    std::vector<std::string> stages;
};

// Output: reference to --stream
struct OutputStreamConfig : OutputCommon {
    std::string stream_id;
};

// Output: stdout
struct OutputStdoutConfig : OutputCommon {};

// Output: file (legacy io::Output)
struct OutputFileConfig : OutputCommon {
    std::string path;
    bool        truncate = true;
    bool        append   = false;
};

// Output: chunked-log
struct OutputChunkedLogConfig : OutputCommon {
    std::string path;
};

// Output: tcp-server (legacy io::Output)
struct OutputTcpServerConfig : OutputCommon {
    std::string listen;
    uint16_t    port = 0;
    std::string path;
};

// Output: inline serial (creates stream)
struct OutputSerialConfig : OutputCommon {
    std::string      stream_id;
    io::SerialConfig config;
};

// Output: inline tcp-client (creates stream)
struct OutputTcpClientConfig : OutputCommon {
    std::string         stream_id;
    io::TcpClientConfig config;
};

// Output: inline udp-client (creates stream)
struct OutputUdpClientConfig : OutputCommon {
    std::string         stream_id;
    io::UdpClientConfig config;
};

struct OutputsConfig {
    std::vector<OutputStreamConfig>     stream;
    std::vector<OutputStdoutConfig>     stdout_outputs;
    std::vector<OutputFileConfig>       file;
    std::vector<OutputChunkedLogConfig> chunked_log;
    std::vector<OutputTcpServerConfig>  tcp_server;
    std::vector<OutputSerialConfig>     serial;
    std::vector<OutputTcpClientConfig>  tcp_client;
    std::vector<OutputUdpClientConfig>  udp_client;
};
