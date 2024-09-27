#include "loglet/loglet.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <vector>
#include <cstring>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BOLD "\033[1m"
#define COLOR_UNDERLINE "\033[4m"
#define COLOR_GREEN "\033[32m"

struct Scope {
    const char* tag;
};

namespace loglet {

static Level                    sLevel = Level::Debug;
static std::vector<Scope>       sScopes;
static std::vector<const char*> sDisabledModules;

void set_level(Level level) {
    sLevel = level;
}

void disable_module(const char* module) {
    sDisabledModules.push_back(module);
}

bool is_module_enabled(const char* module) {
    for (const auto& disabled_module : sDisabledModules) {
        if (strcmp(module, disabled_module) == 0) {
            return false;
        }
    }
    return true;
}

void push_indent() {
    sScopes.push_back({""});
}

void pop_indent() {
    sScopes.pop_back();
}

static const char* level_to_string(Level level) {
    switch (level) {
    case Level::Verbose: return "V";
    case Level::Debug: return "D";
    case Level::Info: return "I";
    case Level::Warning: return "W";
    case Level::Error: return "E";
    default: LOGLET_UNREACHABLE();
    }
}

static const char* level_to_color(Level level) {
    switch (level) {
    case Level::Verbose: return COLOR_CYAN;
    case Level::Debug: return COLOR_GREEN;
    case Level::Info: return COLOR_WHITE;
    case Level::Warning: return COLOR_UNDERLINE COLOR_YELLOW;
    case Level::Error: return COLOR_BOLD COLOR_RED;
    default: LOGLET_UNREACHABLE();
    }
}

void log(const char* module, Level level, const char* message) {
    if (level < sLevel) {
        return;
    }

    if(!is_module_enabled(module)) {
        return;
    }

    auto        now   = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char        buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));

    auto start_color = level_to_color(level);
    auto stop_color  = COLOR_RESET;

    printf("%s %s %s: [%6s] ", start_color, level_to_string(level), buffer, module);
    for (size_t i = 0; i < sScopes.size(); i++) {
        printf("  ");
    }
    printf("%s%s\n", message, stop_color);
}

void logf(const char* module, Level level, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlogf(module, level, format, args);
    va_end(args);
}

void vlogf(const char* module, Level level, const char* format, va_list args) {
    char buffer[32 * 1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    log(module, level, buffer);
}

void vverbosef(const char* module, const char* format, va_list args) {
    vlogf(module, Level::Verbose, format, args);
}

void vdebugf(const char* module, const char* format, va_list args) {
    vlogf(module, Level::Debug, format, args);
}

void vinfof(const char* module, const char* format, va_list args) {
    vlogf(module, Level::Info, format, args);
}

void vwarnf(const char* module, const char* format, va_list args) {
    vlogf(module, Level::Warning, format, args);
}

void verrorf(const char* module, const char* format, va_list args) {
    vlogf(module, Level::Error, format, args);
}

void verbosef(const char* module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vverbosef(module, format, args);
    va_end(args);
}

void debugf(const char* module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vdebugf(module, format, args);
    va_end(args);
}

void infof(const char* module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vinfof(module, format, args);
    va_end(args);
}

void warnf(const char* module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vwarnf(module, format, args);
    va_end(args);
}

void errorf(const char* module, const char* format, ...) {
    va_list args;
    va_start(args, format);
    verrorf(module, format, args);
    va_end(args);
}

}  // namespace loglet
