#pragma once
#include <string>
#include <vector>

#include <memory>
#include <interface/interface.hpp>

struct Config {
    int port;
    int update_interval;
    std::unique_ptr<interface::Interface> interface;
};

extern Config parse_configuration(int argc, char** argv);
