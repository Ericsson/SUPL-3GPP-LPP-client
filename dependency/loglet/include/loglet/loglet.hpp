#pragma once
#include <core/core.hpp>

#include <cstdarg>

#define LOGLET_CURRENT_FUNCTION __FUNCTION__
#define LOGLET_NAMEPASTE2(a, b) a##b
#define LOGLET_NAMEPASTE(a, b) LOGLET_NAMEPASTE2(a, b)

#define LOGLET_VINDENT_SCOPE()                                                                     \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        loglet::Level::Verbose, LOGLET_CURRENT_MODULE                                              \
    }
#define LOGLET_DINDENT_SCOPE()                                                                     \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        loglet::Level::Debug, LOGLET_CURRENT_MODULE                                                \
    }
#define LOGLET_IINDENT_SCOPE()                                                                     \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        loglet::Level::Info, LOGLET_CURRENT_MODULE                                                 \
    }
#define LOGLET_WINDENT_SCOPE()                                                                     \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        loglet::Level::Warning, LOGLET_CURRENT_MODULE                                              \
    }
#define LOGLET_EINDENT_SCOPE()                                                                     \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        loglet::Level::Error, LOGLET_CURRENT_MODULE                                                \
    }
#define LOGLET_XINDENT_SCOPE(module, level)                                                        \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        level, module                                                                              \
    }

#define VSCOPE_FUNCTION()                                                                          \
    VERBOSEF("%s()", LOGLET_CURRENT_FUNCTION);                                                     \
    LOGLET_VINDENT_SCOPE()

#define VSCOPE_FUNCTIONF(fmt, ...)                                                                 \
    VERBOSEF("%s(" fmt ")", LOGLET_CURRENT_FUNCTION, ##__VA_ARGS__);                               \
    LOGLET_VINDENT_SCOPE()

#ifdef _GNU_SOURCE
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#if (__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 30))
#define HAVE_STRERRORNAME_NP
#endif
#endif
#endif

#ifdef HAVE_STRERRORNAME_NP
#define ERRNO_FMT "%3d (%s) %s"
#define ERRNO_ARGS(e) e, strerrorname_np(e), strerror(e)
#else
#define ERRNO_FMT "%3d %s"
#define ERRNO_ARGS(e) e, strerror(e)
#endif

#define VERBOSEF(fmt, ...) loglet::verbosef(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define DEBUGF(fmt, ...) loglet::debugf(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define INFOF(fmt, ...) loglet::infof(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define WARNF(fmt, ...) loglet::warnf(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define ERRORF(fmt, ...) loglet::errorf(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)

#define XVERBOSEF(module, fmt, ...) loglet::verbosef(module, fmt, ##__VA_ARGS__)
#define XDEBUGF(module, fmt, ...) loglet::debugf(module, fmt, ##__VA_ARGS__)
#define XINFOF(module, fmt, ...) loglet::infof(module, fmt, ##__VA_ARGS__)
#define XWARNF(module, fmt, ...) loglet::warnf(module, fmt, ##__VA_ARGS__)
#define XERRORF(module, fmt, ...) loglet::errorf(module, fmt, ##__VA_ARGS__)

#define UNREACHABLE()                                                                              \
    do {                                                                                           \
        ERRORF("unreachable code reached: %s:%d", __FILE__, __LINE__);                             \
        CORE_UNREACHABLE();                                                                        \
    } while (0)

#define XUNREACHABLE(module)                                                                       \
    do {                                                                                           \
        XERRORF(module, "unreachable code reached: %s:%d", __FILE__, __LINE__);                    \
        CORE_UNREACHABLE();                                                                        \
    } while (0)

#define ASSERT(cond, reason)                                                                       \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            ERRORF("!!! assertion failed !!!");                                                    \
            ERRORF("condition: %s", #cond);                                                        \
            ERRORF("reason: %s", reason);                                                          \
            ERRORF("file: %s", __FILE__);                                                          \
            ERRORF("line: %d", __LINE__);                                                          \
            CORE_UNREACHABLE();                                                                    \
        }                                                                                          \
    } while (0)

#define UNIMPLEMENTED(reason)                                                                      \
    do {                                                                                           \
        ERRORF("!!! unimplemented code reached !!!");                                              \
        ERRORF("reason: %s", reason);                                                              \
        ERRORF("file: %s", __FILE__);                                                              \
        ERRORF("line: %d", __LINE__);                                                              \
        CORE_UNREACHABLE();                                                                        \
    } while (0)

namespace loglet {

enum class Level {
    Verbose,
    Debug,
    Info,
    Warning,
    Error,
    Disabled,
};

void uninitialize();

void set_level(Level level);
void set_module_level(char const* module, Level level);
void disable_module(char const* module);
bool is_module_enabled(char const* module);
bool is_level_enabled(Level level);
bool is_module_level_enabled(char const* module, Level level);
void push_indent();
void pop_indent();

void log(char const* module, Level level, char const* message);
void logf(char const* module, Level level, char const* format, ...);
void vlogf(char const* module, Level level, char const* format, va_list args);

void verbosef(char const* module, char const* format, ...);
void debugf(char const* module, char const* format, ...);
void infof(char const* module, char const* format, ...);
void warnf(char const* module, char const* format, ...);
void errorf(char const* module, char const* format, ...);

void vverbosef(char const* module, char const* format, va_list args);
void vdebugf(char const* module, char const* format, va_list args);
void vinfof(char const* module, char const* format, va_list args);
void vwarnf(char const* module, char const* format, va_list args);
void verrorf(char const* module, char const* format, va_list args);

struct ScopeFunction {
    bool indent = false;
    ScopeFunction(Level level, char const* module) {
        if (is_module_level_enabled(module, level)) {
            push_indent();
            indent = true;
        }
    }
    ~ScopeFunction() {
        if (indent) {
            pop_indent();
        }
    }
};

}  // namespace loglet
