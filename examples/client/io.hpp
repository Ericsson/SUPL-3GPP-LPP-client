#pragma once
#include "config/types.hpp"

#include <io/input.hpp>
#include <io/output.hpp>
#include <io/registry.hpp>

void create_streams(StreamsConfig const& config, io::StreamRegistry& registry);
void create_implicit_streams(InputsConfig const& inputs, OutputsConfig const& outputs,
                             io::StreamRegistry& registry);

std::unique_ptr<io::Input> create_input(InputStreamConfig const& cfg, io::StreamRegistry& registry);
std::unique_ptr<io::Input> create_input(InputStdinConfig const& cfg, io::StreamRegistry& registry);
std::unique_ptr<io::Input> create_input(InputFileConfig const& cfg, io::StreamRegistry& registry);
std::unique_ptr<io::Input> create_input(InputSerialConfig const& cfg, io::StreamRegistry& registry);
std::unique_ptr<io::Input> create_input(InputTcpClientConfig const& cfg,
                                        io::StreamRegistry&         registry);
std::unique_ptr<io::Input> create_input(InputTcpServerConfig const& cfg,
                                        io::StreamRegistry&         registry);
std::unique_ptr<io::Input> create_input(InputUdpServerConfig const& cfg,
                                        io::StreamRegistry&         registry);

std::unique_ptr<io::Output> create_output(OutputStreamConfig const& cfg,
                                          io::StreamRegistry&       registry);
std::unique_ptr<io::Output> create_output(OutputStdoutConfig const& cfg,
                                          io::StreamRegistry&       registry);
std::unique_ptr<io::Output> create_output(OutputFileConfig const& cfg,
                                          io::StreamRegistry&     registry);
std::unique_ptr<io::Output> create_output(OutputChunkedLogConfig const& cfg);
std::unique_ptr<io::Output> create_output(OutputTcpServerConfig const& cfg);
std::unique_ptr<io::Output> create_output(OutputSerialConfig const& cfg,
                                          io::StreamRegistry&       registry);
std::unique_ptr<io::Output> create_output(OutputTcpClientConfig const& cfg,
                                          io::StreamRegistry&          registry);
std::unique_ptr<io::Output> create_output(OutputUdpClientConfig const& cfg,
                                          io::StreamRegistry&          registry);
