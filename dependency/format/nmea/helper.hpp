#pragma once
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(std::string const& str, char delim);

bool parse_double(std::string const& token, double& value);
bool parse_double_opt(std::string const& token, double& value);
