#pragma once
#include <sstream>
#include <string>
#include <vector>

/// Replace non-printable characters so input can not inject control or escape sequences into the
/// log or the terminal.
static std::string sanitize_text(std::string const& input) {
    std::string output;
    output.reserve(input.size());
    for (auto c : input) {
        auto uc = static_cast<unsigned char>(c);
        output += (uc >= 0x20 && uc < 0x7F) ? c : '.';
    }
    return output;
}

static std::vector<std::string> split(std::string const& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}
