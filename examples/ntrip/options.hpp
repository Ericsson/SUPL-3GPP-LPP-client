#pragma once
#include <memory>
#include <vector>

#include <client-io/config.hpp>
#include <client-io/io.hpp>
#include <client-io/types.hpp>
#include <io/output.hpp>

/// Host options.
struct HostOptions {
    std::string                  hostname;
    uint16_t                     port;
    std::unique_ptr<std::string> mountpoint;
    std::string                  username;
    std::string                  password;
    std::string                  nmea;
    bool                         hexdump;
};

struct Options {
    HostOptions   host;
    OutputsConfig outputs;
};

extern Options parse_configuration(int argc, char** argv);
