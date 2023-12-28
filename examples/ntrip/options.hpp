#pragma once
#include <memory>
#include <interface/interface.hpp>
#include <vector>

/// Host options.
struct HostOptions {
    /// Hostname to connect to.
    std::string hostname;

    /// Port to connect to.
    uint16_t port;

    /// Mountpoint to connect to.
    std::unique_ptr<std::string> mountpoint;

    /// Username to connect with.
    std::string username;

    /// Password to connect with.
    std::string password;

    /// Nmea String
    std::string nmea;
};

/// Output options.
struct OutputOptions {
    /// Interfaces to output data to.
    std::vector<std::unique_ptr<interface::Interface>> interfaces;
};

struct Options {
    HostOptions   host;
    OutputOptions output;
};

extern Options parse_configuration(int argc, char** argv);
