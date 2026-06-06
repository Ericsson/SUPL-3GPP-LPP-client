#pragma once
#include <client-io/registry.hpp>
#include <client-io/types.hpp>

#include <io/input.hpp>
#include <io/output.hpp>
#include <io/registry.hpp>

void create_streams(StreamsConfig const& config, io::StreamRegistry& registry);

std::unique_ptr<io::Input>  create_input(InputEntry const& entry, io::StreamRegistry& registry);
std::unique_ptr<io::Output> create_output(OutputEntry const& entry, io::StreamRegistry& registry);

// Input type handler factories — call before input::setup()
io_registry::InputTypeHandler make_stdin_input_type();
io_registry::InputTypeHandler make_file_input_type();
io_registry::InputTypeHandler make_serial_input_type();
io_registry::InputTypeHandler make_tcp_client_input_type();
io_registry::InputTypeHandler make_tcp_server_input_type();
io_registry::InputTypeHandler make_udp_server_input_type();
io_registry::InputTypeHandler make_stream_ref_input_type();

// Output type handler factories — call before output::setup()
io_registry::OutputTypeHandler make_stdout_output_type();
io_registry::OutputTypeHandler make_file_output_type();
io_registry::OutputTypeHandler make_tcp_server_output_type();
io_registry::OutputTypeHandler make_serial_output_type();
io_registry::OutputTypeHandler make_tcp_client_output_type();
io_registry::OutputTypeHandler make_udp_client_output_type();
io_registry::OutputTypeHandler make_stream_ref_output_type();
