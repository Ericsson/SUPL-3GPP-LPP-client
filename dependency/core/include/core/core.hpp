#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cinttypes>

#ifndef EXPLICIT
#define EXPLICIT explicit
#endif

#ifndef NOEXCEPT
#define NOEXCEPT noexcept
#endif

#ifndef CONSTEXPR
#define CONSTEXPR constexpr
#endif

#ifndef UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define UNUSED [[maybe_unused]]
#endif
#endif
#ifndef UNUSED
#define UNUSED
#endif
#endif

#ifndef NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef NODISCARD
#define NODISCARD
#endif
#endif

#ifndef CORE_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define CORE_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef CORE_UNREACHABLE
#define CORE_UNREACHABLE() core_unreachable()
__attribute__((noreturn)) inline void core_unreachable() {
    assert(false);
}
#endif
#endif

#ifndef CORE_ASSERT
#define CORE_ASSERT(cond, msg) assert((cond) && msg)
#endif

#ifndef NORETURN
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn)
#define NORETURN [[noreturn]]
#endif
#endif
#ifndef NORETURN
#define NORETURN
#endif
#endif
