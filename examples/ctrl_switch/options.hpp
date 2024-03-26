#pragma once
#include <string>
#include <vector>

struct Config {
    std::vector<std::string> commands;
};

extern Config parse_configuration(int argc, char** argv);
