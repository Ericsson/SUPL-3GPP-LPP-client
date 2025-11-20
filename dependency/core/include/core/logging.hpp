#pragma once
#include <core/error.hpp>
#include <cstdlib>
#include <loglet/loglet.hpp>

// Print error with proper formatting
#define PRINT_ERROR(err)                                                                           \
    do {                                                                                           \
        auto _err       = (err);                                                                   \
        auto _code_name = _err.code_name();                                                        \
        if (_code_name) {                                                                          \
            if (_err.system_errno != 0) {                                                          \
                ERRORF("domain=%s code=%s " ERRNO_FMT, _err.domain_name(), _code_name,             \
                       ERRNO_ARGS(_err.system_errno));                                             \
            } else {                                                                               \
                ERRORF("domain=%s code=%s", _err.domain_name(), _code_name);                       \
            }                                                                                      \
        } else {                                                                                   \
            if (_err.system_errno != 0) {                                                          \
                ERRORF("domain=%s code=%u " ERRNO_FMT, _err.domain_name(), _err.code,              \
                       ERRNO_ARGS(_err.system_errno));                                             \
            } else {                                                                               \
                ERRORF("domain=%s code=%u", _err.domain_name(), _err.code);                        \
            }                                                                                      \
        }                                                                                          \
    } while (0)

// Panic if operation fails (exit program)
#define PANIC_IF_ERROR(expr)                                                                       \
    do {                                                                                           \
        auto _err = (expr);                                                                        \
        if (!_err) {                                                                               \
            ERRORF("PANIC: %s failed:", #expr);                                                    \
            PRINT_ERROR(_err);                                                                     \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)

// Scheduler-specific panic
#define SCHEDULE_OR_PANIC(task, sched)                                                             \
    do {                                                                                           \
        auto _err = (task).schedule(sched);                                                        \
        if (!_err) {                                                                               \
            ERRORF("PANIC: Failed to schedule %s:", #task);                                        \
            PRINT_ERROR(_err);                                                                     \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)

// Log warning but continue
#define WARN_IF_ERROR(expr)                                                                        \
    do {                                                                                           \
        auto _err = (expr);                                                                        \
        if (!_err) {                                                                               \
            if (_err.system_errno != 0) {                                                          \
                WARNF("%s failed: domain=%s code=%u " ERRNO_FMT, #expr, _err.domain_name(),        \
                      _err.code, ERRNO_ARGS(_err.system_errno));                                   \
            } else {                                                                               \
                WARNF("%s failed: domain=%s code=%u", #expr, _err.domain_name(), _err.code);       \
            }                                                                                      \
        }                                                                                          \
    } while (0)

// Return error to caller
#define RETURN_IF_ERROR(expr)                                                                      \
    do {                                                                                           \
        auto _err = (expr);                                                                        \
        if (!_err) {                                                                               \
            PRINT_ERROR(_err);                                                                     \
            return _err;                                                                           \
        }                                                                                          \
    } while (0)

// Syscall that returns error on failure
#define SYSCALL_ERROR(syscall_name, error_code, ...)                                               \
    ({                                                                                             \
        auto _result = ::syscall_name(__VA_ARGS__);                                                \
        auto _errno  = errno;                                                                      \
        VERBOSEF("::" #syscall_name "(" #__VA_ARGS__ ") = %d", _result);                           \
        if (_result < 0) {                                                                         \
            TRACEF("::" #syscall_name " failed: " ERRNO_FMT, ERRNO_ARGS(_errno));                  \
            return make_error(error_code, _errno);                                                 \
        }                                                                                          \
        _result;                                                                                   \
    })
