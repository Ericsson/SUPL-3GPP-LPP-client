#pragma once
#include <string>
#include <vector>

#include <memory>
#include <io/input.hpp>
#include <io/output.hpp>
#include <io/serial.hpp>

struct Config {
    int port;
    int update_interval;
    std::unique_ptr<io::Input> input;
    std::unique_ptr<io::Output> output;
};

extern Config parse_configuration(int argc, char** argv);
