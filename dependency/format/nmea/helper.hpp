#pragma once
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(std::string const& str, char delim);

/// Replace non-printable characters so input can not inject control or escape sequences into the
/// log or the terminal.
std::string sanitize_text(std::string const& input);

bool parse_double(std::string const& token, double& value);
bool parse_double_opt(std::string const& token, double& value);
