#pragma once
#include <interface/interface.hpp>
#include <memory>
#include <vector>

/// Format options.
enum class Format {
    /// SPARTN format, using the old SPARTN generator.
    SPARTN_OLD,
    /// SPARTN format, using the new SPARTN generator.
    SPARTN_NEW,
};

/// SPARTN options.
struct SpartnOptions {
    bool iode_shift;
};

/// Output options.
struct OutputOptions {
    /// Interfaces to output data to.
    std::vector<std::unique_ptr<interface::Interface>> interfaces;
};

struct Options {
    Format        format;
    OutputOptions output;
    SpartnOptions spartn;
};

extern Options parse_configuration(int argc, char** argv);
