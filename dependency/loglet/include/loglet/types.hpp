#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdarg>

#ifndef LOGLET_EXPLICIT
#define LOGLET_EXPLICIT explicit
#endif

#ifndef LOGLET_NOEXCEPT
#define LOGLET_NOEXCEPT noexcept
#endif

#ifndef LOGLET_CONSTEXPR
#define LOGLET_CONSTEXPR constexpr
#endif

#ifndef LOGLET_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define LOGLET_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef LOGLET_UNUSED
#define LOGLET_UNUSED
#endif
#endif

#ifndef LOGLET_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define LOGLET_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef LOGLET_NODISCARD
#define LOGLET_NODISCARD
#endif
#endif

#ifndef LOGLET_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define LOGLET_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef LOGLET_UNREACHABLE
#define LOGLET_UNREACHABLE() loglet_unreachable()
__attribute__((noreturn)) inline void loglet_unreachable() {
    assert(false);
}
#endif
#endif
