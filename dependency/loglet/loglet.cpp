#include "loglet/loglet.hpp"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
    char const* tag;
};

namespace loglet {

struct HashModuleName {
    size_t operator()(char const* module) const {
        size_t hash = 0;
        for (size_t i = 0; module[i] != '\0'; i++) {
            hash = 31 * hash + static_cast<size_t>(module[i]);
        }
        return hash;
    }
};

struct EqualModuleName {
    bool operator()(char const* a, char const* b) const { return strcmp(a, b) == 0; }
};

struct Module {
    char const* name;
    Level       level;
};

static Level              sLevel        = Level::Debug;
static bool               sColorEnabled = true;
static bool               sAlwaysFlush  = false;
static std::vector<Scope> sScopes;

static std::unordered_map<char const*, Module, HashModuleName, EqualModuleName> sModules;

Module& get_or_add_module(char const* reference) {
    auto it = sModules.find(reference);
    if (it == sModules.end()) {
        auto constant_name = new char[strlen(reference) + 1];
        strcpy(constant_name, reference);

        Module module{};
        module.name             = constant_name;
        module.level            = sLevel;
        sModules[constant_name] = std::move(module);
        return sModules[constant_name];
    } else {
        return it->second;
    }
}

void uninitialize() {
    sScopes.clear();
    for (auto it = sModules.begin(); it != sModules.end();) {
        delete[] it->second.name;
        it = sModules.erase(it);
    }
}

void set_level(Level level) {
    sLevel = level;
}

void set_color_enable(bool enabled) {
    sColorEnabled = enabled;
}

void set_always_flush(bool flush) {
    sAlwaysFlush = flush;
}

void set_module_level(char const* module, Level level) {
    auto& module_ref = get_or_add_module(module);
    module_ref.level = level;
}

void disable_module(char const* module) {
    auto& module_ref = get_or_add_module(module);
    module_ref.level = Level::Disabled;
}

bool is_module_enabled(char const* module) {
    auto it = sModules.find(module);
    if (it == sModules.end()) {
        return true;
    }
    return it->second.level != Level::Disabled;
}

bool is_level_enabled(Level level) {
    return level >= sLevel;
}

bool is_module_level_enabled(char const* module, Level level) {
    auto it = sModules.find(module);
    if (it == sModules.end()) {
        return is_level_enabled(level);
    }
    return level >= it->second.level;
}

void push_indent() {
    sScopes.push_back({""});
}

void pop_indent() {
    sScopes.pop_back();
}

static char const* level_to_string(Level level) {
    switch (level) {
    case Level::Trace: return "T";
    case Level::Verbose: return "V";
    case Level::Debug: return "D";
    case Level::Info: return "I";
    case Level::Notice: return "N";
    case Level::Warning: return "W";
    case Level::Error: return "E";
    case Level::Disabled: CORE_UNREACHABLE();
    }
}

static char const* level_to_color(Level level) {
    if (!sColorEnabled) return "";
    switch (level) {
    case Level::Trace: return COLOR_CYAN;
    case Level::Verbose: return COLOR_BLUE;
    case Level::Debug: return COLOR_GREEN;
    case Level::Info: return COLOR_WHITE;
    case Level::Notice: return COLOR_UNDERLINE COLOR_MAGENTA;
    case Level::Warning: return COLOR_UNDERLINE COLOR_YELLOW;
    case Level::Error: return COLOR_BOLD COLOR_RED;
    case Level::Disabled: CORE_UNREACHABLE();
    }
}

void log(char const* module, Level level, char const* message) {
    if (!is_module_level_enabled(module, level)) {
        return;
    }

    auto        now   = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char        buffer[64];
    strftime(buffer, sizeof(buffer), "%y%m%d %H:%M:%S", std::localtime(&now_c));

    auto start_color = level_to_color(level);
    auto stop_color  = sColorEnabled ? COLOR_RESET : "";

    auto file = stdout;
    if (level == Level::Error || level == Level::Warning) {
        file = stderr;
        // flush stdout to ensure that the log messages are in order
        fflush(stdout);
    }

    if (sAlwaysFlush) {
        fflush(file);
    }

    char const indent_buffer[64 + 1] =
        "                                                                ";
    auto indent_length = static_cast<int>(sScopes.size() * 2);
    if (indent_length > 64) {
        indent_length = 64;
    }
    fprintf(file, "%s%s%s[%10s] %.*s%s%s\n", start_color, level_to_string(level), buffer, module,
            static_cast<int>(sScopes.size() * 2), indent_buffer, message, stop_color);
}

void logf(char const* module, Level level, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vlogf(module, level, format, args);
    va_end(args);
}

void vlogf(char const* module, Level level, char const* format, va_list args) {
    char buffer[32 * 1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    log(module, level, buffer);
}

void vtracef(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Trace, format, args);
}

void vverbosef(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Verbose, format, args);
}

void vdebugf(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Debug, format, args);
}

void vinfof(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Info, format, args);
}

void vnoticef(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Notice, format, args);
}

void vwarnf(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Warning, format, args);
}

void verrorf(char const* module, char const* format, va_list args) {
    vlogf(module, Level::Error, format, args);
}

void tracef(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vtracef(module, format, args);
    va_end(args);
}

void verbosef(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vverbosef(module, format, args);
    va_end(args);
}

void debugf(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vdebugf(module, format, args);
    va_end(args);
}

void infof(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vinfof(module, format, args);
    va_end(args);
}

void noticef(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vnoticef(module, format, args);
    va_end(args);
}

void warnf(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vwarnf(module, format, args);
    va_end(args);
}

void errorf(char const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    verrorf(module, format, args);
    va_end(args);
}

}  // namespace loglet
