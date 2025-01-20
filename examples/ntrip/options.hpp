#pragma once
#include <memory>
#include <vector>

#include <io/output.hpp>
#include <io/file.hpp>
#include <io/serial.hpp>
#include <io/tcp.hpp>
#include <io/udp.hpp>
#include <io/stdout.hpp>

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

    bool hexdump;
};

/// Output options.
struct OutputOptions {
    std::vector<std::unique_ptr<io::Output>> outputs;
};

struct Options {
    HostOptions   host;
    OutputOptions output;
};

extern Options parse_configuration(int argc, char** argv);
