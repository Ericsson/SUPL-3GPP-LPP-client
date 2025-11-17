#include "helper.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE3(format, nmea, helper);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(format, nmea, helper)

std::vector<std::string> split(std::string const& str, char delim) {
    std::vector<std::string> tokens;
    std::string              token;
    std::istringstream       token_stream(str);
    while (std::getline(token_stream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool parse_double(std::string const& token, double& value) {
    FUNCTION_SCOPEF("'%s'", token.c_str());
    try {
        value = std::stod(token);
        VERBOSEF("parsed: %.6f", value);
        return true;
    } catch (...) {
        VERBOSEF("exception parsing double: '%s'", token.c_str());
        return false;
    }
}

bool parse_double_opt(std::string const& token, double& value) {
    FUNCTION_SCOPEF("'%s'", token.c_str());
    try {
        value = std::stod(token);
        VERBOSEF("parsed: %.6f", value);
        return true;
    } catch (...) {
        VERBOSEF("failed to parse, using default: '%s'", token.c_str());
        value = 0;
        return true;
    }
}
