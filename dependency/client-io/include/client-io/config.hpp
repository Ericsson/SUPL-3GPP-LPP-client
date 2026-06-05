#pragma once
#include <client-io/program_io.hpp>
#include <client-io/types.hpp>

#include <external_warnings.hpp>
EXTERNAL_WARNINGS_PUSH
#include <args.hxx>
EXTERNAL_WARNINGS_POP

namespace stream {
void setup(args::ArgumentParser& parser);
void parse(StreamsConfig& config);
void dump(StreamsConfig const& config);
}  // namespace stream

namespace input {
void setup(args::ArgumentParser& parser);
void parse(InputsConfig& config);
void dump(InputsConfig const& config);
}  // namespace input

namespace output {
void setup(args::ArgumentParser& parser);
void parse(OutputsConfig& config);
void dump(OutputsConfig const& config);
}  // namespace output
