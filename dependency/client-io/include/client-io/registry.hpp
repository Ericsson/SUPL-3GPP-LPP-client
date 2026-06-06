#pragma once
#include <client-io/input_format.hpp>
#include <client-io/output_format.hpp>
#include <io/input.hpp>
#include <io/output.hpp>
#include <io/registry.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace io_registry {

using Options = std::unordered_map<std::string, std::string>;

struct InputTypeHandler {
    std::string                                                                    name;
    std::string                                                                    help;
    std::function<std::unique_ptr<io::Input>(Options const&, io::StreamRegistry&)> create;
};

struct OutputTypeHandler {
    std::string                                                                     name;
    std::string                                                                     help;
    std::function<std::unique_ptr<io::Output>(Options const&, io::StreamRegistry&)> create;
};

struct InputFormatInfo {
    std::string name;
    InputFormat flag;
    std::string extra_options_help;
};

struct OutputFormatInfo {
    std::string  name;
    OutputFormat flag;
};

void register_input_type(InputTypeHandler);
void register_output_type(OutputTypeHandler);
void register_input_format(InputFormatInfo);
void register_output_format(OutputFormatInfo);

std::vector<InputTypeHandler> const&  input_types();
std::vector<OutputTypeHandler> const& output_types();
std::vector<InputFormatInfo> const&   input_formats();
std::vector<OutputFormatInfo> const&  output_formats();

InputFormat  parse_input_format_list(std::string const& str);
OutputFormat parse_output_format_list(std::string const& str);

}  // namespace io_registry
