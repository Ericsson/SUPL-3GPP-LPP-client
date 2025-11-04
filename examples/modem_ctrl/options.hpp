#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <io/input.hpp>
#include <io/output.hpp>
#include <io/serial.hpp>
#include <loglet/loglet.hpp>
#include <memory>

struct LoggingConfig {
    loglet::Level                                  log_level;
    std::unordered_map<std::string, loglet::Level> module_levels;
    bool                                           color;
    bool                                           flush;
};

struct Config {
    uint16_t                    port;
    int                         update_interval;
    LoggingConfig               logging;
    std::unique_ptr<io::Input>  input;
    std::unique_ptr<io::Output> output;
};

extern Config parse_configuration(int argc, char** argv);
