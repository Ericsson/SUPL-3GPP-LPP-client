#pragma once
#include <interface/interface.hpp>
#include <memory>
#include <vector>

struct Options {};

extern Options parse_configuration(int argc, char** argv);
