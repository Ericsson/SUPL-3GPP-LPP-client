#pragma once
#include <memory>
#include <interface/interface.hpp>
#include <vector>

/// Format options.
enum class Format {
    /// SPARTN format.
    SPARTN,
    /// SPARTN2 format.
    SPARTN2,
};

/// Output options.
struct OutputOptions {
    /// Interfaces to output data to.
    std::vector<std::unique_ptr<interface::Interface>> interfaces;
};

struct Options {
    Format        format;
    OutputOptions output;
};

extern Options parse_configuration(int argc, char** argv);
