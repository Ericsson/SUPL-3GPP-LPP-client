#pragma once
#include <memory>
#include <interface/interface.hpp>
#include <vector>

/// Output options.
struct OutputOptions {
    /// Interfaces to output data to.
    std::vector<std::unique_ptr<interface::Interface>> interfaces;
};

struct Options {
    OutputOptions output;
};

extern Options parse_configuration(int argc, char** argv);
