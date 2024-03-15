#pragma once
#include <loglet/types.hpp>

#define LOGLET_CURRENT_FUNCTION __FUNCTION__
#define LOGLET_INDENT_SCOPE() loglet::ScopeFunction _loglet_scope_function{};

#define VERBOSEF(fmt, ...) loglet::verbosef(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define DEBUGF(fmt, ...) loglet::debugf(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define INFOF(fmt, ...) loglet::infof(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define WARNF(fmt, ...) loglet::warnf(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define ERRORF(fmt, ...) loglet::errorf(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)

namespace loglet {

enum class Level {
    Verbose,
    Debug,
    Info,
    Warning,
    Error,
};

void set_level(Level level);
void push_indent();
void pop_indent();

void log(const char* module, Level level, const char* message);
void logf(const char* module, Level level, const char* format, ...);
void vlogf(const char* module, Level level, const char* format, va_list args);

void verbosef(const char* module, const char* format, ...);
void debugf(const char* module, const char* format, ...);
void infof(const char* module, const char* format, ...);
void warnf(const char* module, const char* format, ...);
void errorf(const char* module, const char* format, ...);

void vverbosef(const char* module, const char* format, va_list args);
void vdebugf(const char* module, const char* format, va_list args);
void vinfof(const char* module, const char* format, va_list args);
void vwarnf(const char* module, const char* format, va_list args);
void verrorf(const char* module, const char* format, va_list args);

struct ScopeFunction {
    ScopeFunction() { push_indent(); }
    ~ScopeFunction() { pop_indent(); }
};

}  // namespace loglet
