#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdarg>

#ifndef MODEM_EXPLICIT
#define MODEM_EXPLICIT explicit
#endif

#ifndef MODEM_NOEXCEPT
#define MODEM_NOEXCEPT noexcept
#endif

#ifndef MODEM_CONSTEXPR
#define MODEM_CONSTEXPR constexpr
#endif

#ifndef MODEM_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define MODEM_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef MODEM_UNUSED
#define MODEM_UNUSED
#endif
#endif

#ifndef MODEM_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define MODEM_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef MODEM_NODISCARD
#define MODEM_NODISCARD
#endif
#endif

#ifndef MODEM_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define MODEM_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef MODEM_UNREACHABLE
#define MODEM_UNREACHABLE() modem_unreachable()
__attribute__((noreturn)) inline void modem_unreachable() {
    assert(false);
}
#endif
#endif
