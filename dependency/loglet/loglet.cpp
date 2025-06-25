#include "loglet/loglet.hpp"

#include <algorithm>
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
#define COLOR_FOREGROUND ""
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
    std::string          name;
    uint64_t             hash;
    Level                level;
    Module*              parent;
    std::vector<Module*> children;
};

static char const*        sPrefix       = nullptr;
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

struct GlobalData {
    std::vector<LogModule*> root_modules;
    std::vector<LogModule*> modules;
    size_t                  max_full_name_length = 0;
};

static GlobalData* sGlobalData = nullptr;

void preinit() {
    if (sGlobalData) return;
    sGlobalData = new GlobalData();
    sGlobalData->root_modules.reserve(100);
    sGlobalData->modules.reserve(256);
    sGlobalData->max_full_name_length = 10;
}

void register_module(LogModule* module) {
    if (!sGlobalData) return;
    sGlobalData->modules.push_back(module);
    if (module->parent == nullptr) {
        sGlobalData->root_modules.push_back(module);
    }
}

void iterate_modules_recursive(LogModule* module, int depth,
                               void (*callback)(LogModule const* module, int depth, void* data),
                               void* data) {
    callback(module, depth, data);
    for (auto child : module->children) {
        iterate_modules_recursive(child, depth + 1, callback, data);
    }
}

void iterate_modules(void (*callback)(LogModule const* module, int depth, void* data), void* data) {
    if (!sGlobalData) return;
    for (auto module : sGlobalData->root_modules) {
        iterate_modules_recursive(module, 0, callback, data);
    }
}

void generate_full_name(LogModule* parent) {
    if (parent && sGlobalData && sGlobalData->max_full_name_length < parent->full_name.length()) {
        sGlobalData->max_full_name_length = parent->full_name.length();
    }
    for (auto child : parent->children) {
        child->full_name = parent->full_name + "/" + child->name;
        generate_full_name(child);
    }
}

void initialize() {
    if (!sGlobalData) return;
    for (auto module : sGlobalData->modules) {
        if (module->parent != nullptr) {
            module->parent->children.push_back(module);
        }
    }

    // Sort modules by name
    std::sort(sGlobalData->root_modules.begin(), sGlobalData->root_modules.end(),
              [](LogModule const* a, LogModule const* b) {
                  return strcmp(a->name, b->name) < 0;
              });

    for (auto module : sGlobalData->root_modules) {
        std::sort(module->children.begin(), module->children.end(),
                  [](LogModule const* a, LogModule const* b) {
                      return strcmp(a->name, b->name) < 0;
                  });
    }

    // Generate full name
    for (auto module : sGlobalData->root_modules) {
        module->full_name = module->name;
        generate_full_name(module);
    }
}

void uninitialize() {
    sScopes.clear();
}

static std::vector<LogModule*> get_all_children(LogModule* module) {
    std::vector<LogModule*> result;
    for (auto child : module->children) {
        result.push_back(child);
        auto children = get_all_children(child);
        result.insert(result.end(), children.begin(), children.end());
    }
    return result;
}

static void find_modules(std::vector<std::string>& parts, size_t index, LogModule* parent,
                         std::vector<LogModule*> const& children, std::vector<LogModule*>& result) {
    if (index >= parts.size()) {
        if (parent != nullptr) {
            result.push_back(parent);
        }
        return;
    }

    if (parts[index] == "+") {
        for (auto child : children) {
            find_modules(parts, index + 1, child, child->children, result);
        }
    } else if (parts[index] == "*") {
        result.push_back(parent);
        for (auto child : get_all_children(parent)) {
            result.push_back(child);
        }
    } else {
        for (auto child : children) {
            if (parts[index] == child->name) {
                find_modules(parts, index + 1, child, child->children, result);
            }
        }
    }
}

std::vector<LogModule*> get_modules(std::string const& name) {
    std::vector<std::string> parts;
    auto                     str = name;
    for (auto it = str.begin(); it != str.end(); it++) {
        if (*it == '/') {
            parts.push_back(std::string(str.begin(), it));
            str = std::string(it + 1, str.end());
            it  = str.begin();
        }
    }

    if (str.size() > 0) {
        parts.push_back(str);
    }

    std::vector<LogModule*> result;
    find_modules(parts, 0, nullptr, sGlobalData->root_modules, result);
    return result;
}

void set_prefix(char const* prefix) {
    sPrefix = prefix;
}

void set_level(Level level) {
    sLevel = level;
    if (!sGlobalData) return;
    for (auto& module : sGlobalData->modules) {
        module->level = level;
    }
}

void set_color_enable(bool enabled) {
    sColorEnabled = enabled;
}

void set_always_flush(bool flush) {
    sAlwaysFlush = flush;
}

void set_module_level(LogModule* module, Level level) {
    module->level = level;
}

void disable_module(LogModule* module) {
    module->level = Level::Disabled;
}

bool is_module_enabled(LogModule const* module) {
    return module->level != Level::Disabled;
}

bool is_level_enabled(Level level) {
    return level >= sLevel;
}

bool is_module_level_enabled(LogModule const* module, Level level) {
    return level >= module->level;
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

char const* level_to_full_string(Level level) {
    switch (level) {
    case Level::Trace: return "trace";
    case Level::Verbose: return "verbose";
    case Level::Debug: return "debug";
    case Level::Info: return "info";
    case Level::Notice: return "notice";
    case Level::Warning: return "warning";
    case Level::Error: return "error";
    case Level::Disabled: CORE_UNREACHABLE();
    }
}

static char const* level_to_color(Level level) {
    if (!sColorEnabled) return "";
    switch (level) {
    case Level::Trace: return COLOR_CYAN;
    case Level::Verbose: return COLOR_BLUE;
    case Level::Debug: return COLOR_GREEN;
    case Level::Info: return COLOR_FOREGROUND;
    case Level::Notice: return COLOR_UNDERLINE COLOR_MAGENTA;
    case Level::Warning: return COLOR_UNDERLINE COLOR_YELLOW;
    case Level::Error: return COLOR_BOLD COLOR_RED;
    case Level::Disabled: CORE_UNREACHABLE();
    }
}

void log(LogModule const* module, Level level, char const* message) {
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

    char indent_buffer[64 + 1] = "                                                                ";
    auto indent_length         = static_cast<int>(sScopes.size() * 2);
    if (indent_length > 64) {
        indent_length = 64;
    }
    indent_buffer[indent_length] = '\0';
    fprintf(file, "%s%s%s[%-*s] %s%s%s\n", start_color, level_to_string(level), buffer,
            static_cast<int>(sGlobalData ? sGlobalData->max_full_name_length : 16),
            module->full_name.c_str(), indent_buffer, message, stop_color);
}

void logf(LogModule const* module, Level level, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vlogf(module, level, format, args);
    va_end(args);
}

void vlogf(LogModule const* module, Level level, char const* format, va_list args) {
    char buffer[32 * 1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    log(module, level, buffer);
}

void vtracef(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Trace, format, args);
}

void vverbosef(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Verbose, format, args);
}

void vdebugf(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Debug, format, args);
}

void vinfof(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Info, format, args);
}

void vnoticef(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Notice, format, args);
}

void vwarnf(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Warning, format, args);
}

void verrorf(LogModule const* module, char const* format, va_list args) {
    vlogf(module, Level::Error, format, args);
}

void tracef(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vtracef(module, format, args);
    va_end(args);
}

void verbosef(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vverbosef(module, format, args);
    va_end(args);
}

void debugf(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vdebugf(module, format, args);
    va_end(args);
}

void infof(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vinfof(module, format, args);
    va_end(args);
}

void noticef(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vnoticef(module, format, args);
    va_end(args);
}

void warnf(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    vwarnf(module, format, args);
    va_end(args);
}

void errorf(LogModule const* module, char const* format, ...) {
    va_list args;
    va_start(args, format);
    verrorf(module, format, args);
    va_end(args);
}

}  // namespace loglet
