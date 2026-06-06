#include <client-io/registry.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace io_registry {

static std::vector<InputTypeHandler>  gInputTypes;
static std::vector<OutputTypeHandler> gOutputTypes;
static std::vector<InputFormatInfo>   gInputFormats;
static std::vector<OutputFormatInfo>  gOutputFormats;

void register_input_type(InputTypeHandler h) {
    gInputTypes.push_back(std::move(h));
}
void register_output_type(OutputTypeHandler h) {
    gOutputTypes.push_back(std::move(h));
}
void register_input_format(InputFormatInfo f) {
    gInputFormats.push_back(std::move(f));
}
void register_output_format(OutputFormatInfo f) {
    gOutputFormats.push_back(std::move(f));
}

std::vector<InputTypeHandler> const& input_types() {
    return gInputTypes;
}
std::vector<OutputTypeHandler> const& output_types() {
    return gOutputTypes;
}
std::vector<InputFormatInfo> const& input_formats() {
    return gInputFormats;
}
std::vector<OutputFormatInfo> const& output_formats() {
    return gOutputFormats;
}

InputFormat parse_input_format_list(std::string const& str) {
    InputFormat result = INPUT_FORMAT_NONE;
    // split on '+'
    std::string token;
    for (char c : str + "+") {
        if (c == '+') {
            if (token.empty()) continue;
            bool found = false;
            for (auto const& f : gInputFormats) {
                if (f.name == token) {
                    result |= f.flag;
                    found = true;
                    break;
                }
            }
            if (!found) throw args::ValidationError("unknown input format: " + token);
            token.clear();
        } else {
            token += c;
        }
    }
    return result;
}

OutputFormat parse_output_format_list(std::string const& str) {
    OutputFormat result = OUTPUT_FORMAT_NONE;
    std::string  token;
    for (char c : str + "+") {
        if (c == '+') {
            if (token.empty()) continue;
            bool found = false;
            for (auto const& f : gOutputFormats) {
                if (f.name == token) {
                    result |= f.flag;
                    found = true;
                    break;
                }
            }
            if (!found) throw args::ValidationError("unknown output format: " + token);
            token.clear();
        } else {
            token += c;
        }
    }
    return result;
}

}  // namespace io_registry
