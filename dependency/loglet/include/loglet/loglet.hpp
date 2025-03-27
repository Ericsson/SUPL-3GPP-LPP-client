#pragma once
#include <core/core.hpp>

#include <cstdarg>
#include <vector>
#include <string>

#define LOGLET_CURRENT_FUNCTION __FUNCTION__
#define LOGLET_NAMEPASTE2(a, b) a##b
#define LOGLET_NAMEPASTE(a, b) LOGLET_NAMEPASTE2(a, b)

#define LOGLET_NAMEPASTE_SEP2(a, b) a##_##b
#define LOGLET_NAMEPASTE_SEP(a, b) LOGLET_NAMEPASTE_SEP2(a, b)

#define LOGLET_DEFAULT_LEVEL loglet::Level::Info

#if !defined(DISABLE_STRERRORNAME_NP)
#if defined(_GNU_SOURCE) && defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#if (__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 30))
#define HAVE_STRERRORNAME_NP
#endif
#endif
#endif

#define LOGLET_XINDENT_SCOPE(module, level)                                                        \
    loglet::ScopeFunction LOGLET_NAMEPASTE(loglet_scope_function, __LINE__) {                      \
        level, module                                                                              \
    }
#define LOGLET_INDENT_SCOPE(level) LOGLET_XINDENT_SCOPE(LOGLET_CURRENT_MODULE, level)

#ifdef HAVE_STRERRORNAME_NP
#define ERRNO_FMT "%3d (%s) %s"
#define ERRNO_ARGS(e) e, strerrorname_np(e), strerror(e)
#else
#define ERRNO_FMT "%3d %s"
#define ERRNO_ARGS(e) e, strerror(e)
#endif

#ifdef DISABLE_TRACE
#define XTRACEF(module, fmt, ...)
#define TRACEF(fmt, ...)
#define XTRACE_INDENT_SCOPE(module)
#define TRACE_INDENT_SCOPE()
#else
#define XTRACEF(module, fmt, ...) loglet::tracef(module, fmt, ##__VA_ARGS__)
#define TRACEF(fmt, ...) XTRACEF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XTRACE_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Trace)
#define TRACE_INDENT_SCOPE() XTRACE_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_VERBOSE
#define XVERBOSEF(module, fmt, ...)
#define VERBOSEF(fmt, ...)
#define XVERBOSE_INDENT_SCOPE(module)
#define VERBOSE_INDENT_SCOPE()
#else
#define XVERBOSEF(module, fmt, ...) loglet::verbosef(module, fmt, ##__VA_ARGS__)
#define VERBOSEF(fmt, ...) XVERBOSEF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XVERBOSE_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Verbose)
#define VERBOSE_INDENT_SCOPE() XVERBOSE_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_DEBUG
#define XDEBUGF(module, fmt, ...)
#define DEBUGF(fmt, ...)
#define XDEBUG_INDENT_SCOPE(module)
#define DEBUG_INDENT_SCOPE()
#else
#define XDEBUGF(module, fmt, ...) loglet::debugf(module, fmt, ##__VA_ARGS__)
#define DEBUGF(fmt, ...) XDEBUGF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XDEBUG_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Debug)
#define DEBUG_INDENT_SCOPE() XDEBUG_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_INFO
#define XINFOF(module, fmt, ...)
#define INFOF(fmt, ...)
#define XINFO_INDENT_SCOPE(module)
#define INFO_INDENT_SCOPE()
#else
#define XINFOF(module, fmt, ...) loglet::infof(module, fmt, ##__VA_ARGS__)
#define INFOF(fmt, ...) XINFOF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XINFO_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Info)
#define INFO_INDENT_SCOPE() XINFO_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_NOTICE
#define XNOTICEF(module, fmt, ...)
#define NOTICEF(fmt, ...)
#define XNOTICE_INDENT_SCOPE(module)
#define NOTICE_INDENT_SCOPE()
#else
#define XNOTICEF(module, fmt, ...) loglet::noticef(module, fmt, ##__VA_ARGS__)
#define NOTICEF(fmt, ...) XNOTICEF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XNOTICE_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Notice)
#define NOTICE_INDENT_SCOPE() XNOTICE_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_WARNING
#define XWARNF(module, fmt, ...)
#define WARNF(fmt, ...)
#define XWARN_INDENT_SCOPE(module)
#define WARN_INDENT_SCOPE()
#else
#define XWARNF(module, fmt, ...) loglet::warnf(module, fmt, ##__VA_ARGS__)
#define WARNF(fmt, ...) XWARNF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XWARN_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Warning)
#define WARN_INDENT_SCOPE() XWARN_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_ERROR
#define XERRORF(module, fmt, ...)
#define ERRORF(fmt, ...)
#define XERROR_INDENT_SCOPE(module)
#define ERROR_INDENT_SCOPE()
#else
#define XERRORF(module, fmt, ...) loglet::errorf(module, fmt, ##__VA_ARGS__)
#define ERRORF(fmt, ...) XERRORF(LOGLET_CURRENT_MODULE, fmt, ##__VA_ARGS__)
#define XERROR_INDENT_SCOPE(module) LOGLET_XINDENT_SCOPE(module, loglet::Level::Error)
#define ERROR_INDENT_SCOPE() XERROR_INDENT_SCOPE(LOGLET_CURRENT_MODULE)
#endif

#ifdef DISABLE_LOGGING
#define TODOF(fmt, ...)
#define XTODOF(module, fmt, ...)
#else
#define TODOF(fmt, ...)                                                                            \
    loglet::errorf(LOGLET_CURRENT_MODULE, "!!! TODO !!! %s:%d\n" fmt, __FILE__, __LINE__,          \
                   ##__VA_ARGS__)
#define XTODOF(module, fmt, ...)                                                                   \
    loglet::errorf(module, "!!! TODO !!! %s:%d\n" fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef DISABLE_TRACE
#define FUNCTION_SCOPE()
#define FUNCTION_SCOPEF(fmt, ...)
#else
#define FUNCTION_SCOPE()                                                                           \
    TRACEF("%s()", LOGLET_CURRENT_FUNCTION);                                                       \
    TRACE_INDENT_SCOPE()
#define FUNCTION_SCOPEF(fmt, ...)                                                                  \
    TRACEF("%s(" fmt ")", LOGLET_CURRENT_FUNCTION, ##__VA_ARGS__);                                 \
    TRACE_INDENT_SCOPE()
#endif

#define VSCOPE_FUNCTIONF(fmt, ...) FUNCTION_SCOPEF(fmt, ##__VA_ARGS__)
#define VSCOPE_FUNCTION() FUNCTION_SCOPE()

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
    Trace    = 0,
    Verbose  = 1,
    Debug    = 2,
    Info     = 3,
    Notice   = 4,
    Warning  = 5,
    Error    = 6,
    Disabled = 999,
};

struct LogModule;
void preinit();
void register_module(LogModule* module);
void iterate_modules(void (*callback)(LogModule const* module, int depth, void* data), void* data);

struct LogModule {
    LogModule*              parent;
    char const*             name;
    std::string             full_name;
    Level                   level;
    bool                    initialized;
    std::vector<LogModule*> children;

    explicit LogModule(LogModule* parent, char const* name)
        : parent(parent), name(name), level(Level::Info), initialized(false) {
        preinit();
        register_module(this);
    }
};

#define LOGLET_MODULE_REF(name) LOGLET_NAMEPASTE(loglet_module_, name)
#define LOGLET_MODULE_REF2(name, child)                                                            \
    LOGLET_NAMEPASTE_SEP(LOGLET_NAMEPASTE(loglet_module_, name), child)
#define LOGLET_MODULE_REF3(name, child, grandchild)                                                \
    LOGLET_NAMEPASTE_SEP(LOGLET_NAMEPASTE_SEP(LOGLET_NAMEPASTE(loglet_module_, name), child),      \
                         grandchild)

#define LOGLET_MODULE_FORWARD_REF(name) extern loglet::LogModule LOGLET_MODULE_REF(name)
#define LOGLET_MODULE_FORWARD_REF2(name, child)                                                    \
    extern loglet::LogModule LOGLET_MODULE_REF2(name, child)
#define LOGLET_MODULE_FORWARD_REF3(name, child, grandchild)                                        \
    extern loglet::LogModule LOGLET_MODULE_REF3(name, child, grandchild)

#define LOGLET_MODULE(module)                                                                      \
    loglet::LogModule LOGLET_MODULE_REF(module) {                                                  \
        nullptr, #module                                                                           \
    }
#define LOGLET_MODULE2(parent, child)                                                              \
    LOGLET_MODULE_FORWARD_REF(parent);                                                             \
    loglet::LogModule LOGLET_MODULE_REF2(parent, child) {                                          \
        &LOGLET_MODULE_REF(parent), #child                                                         \
    }
#define LOGLET_MODULE3(parent, child, grandchild)                                                  \
    LOGLET_MODULE_FORWARD_REF2(parent, child);                                                     \
    loglet::LogModule LOGLET_MODULE_REF3(parent, child, grandchild) {                              \
        &LOGLET_MODULE_REF2(parent, child), #grandchild                                            \
    }

void initialize();
void uninitialize();

void set_prefix(char const* prefix);
void set_level(Level level);
void set_color_enable(bool enabled);
void set_always_flush(bool flush);
void set_module_level(LogModule* module, Level level);
void disable_module(LogModule* module);
bool is_module_enabled(LogModule const* module);
bool is_level_enabled(Level level);
bool is_module_level_enabled(LogModule const* module, Level level);
std::vector<LogModule*> get_modules(std::string const& name);
const char* level_to_full_string(Level level);

void push_indent();
void pop_indent();

void log(LogModule const* module, Level level, char const* message);
void logf(LogModule const* module, Level level, char const* format, ...);
void vlogf(LogModule const* module, Level level, char const* format, va_list args);

void tracef(LogModule const* module, char const* format, ...);
void verbosef(LogModule const* module, char const* format, ...);
void debugf(LogModule const* module, char const* format, ...);
void infof(LogModule const* module, char const* format, ...);
void noticef(LogModule const* module, char const* format, ...);
void warnf(LogModule const* module, char const* format, ...);
void errorf(LogModule const* module, char const* format, ...);

void vtracef(LogModule const* module, char const* format, va_list args);
void vverbosef(LogModule const* module, char const* format, va_list args);
void vdebugf(LogModule const* module, char const* format, va_list args);
void vinfof(LogModule const* module, char const* format, va_list args);
void vnoticef(LogModule const* module, char const* format, va_list args);
void vwarnf(LogModule const* module, char const* format, va_list args);
void verrorf(LogModule const* module, char const* format, va_list args);

struct ScopeFunction {
    bool indent = false;
    ScopeFunction(Level level, LogModule const* module) {
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
