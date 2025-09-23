#pragma once
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

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
#define NODISCARD [[nodiscard]]
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
#ifndef CORE_UNREACHABLE_DEFINED
#define CORE_UNREACHABLE_DEFINED
__attribute__((noreturn)) inline void core_unreachable() {
    assert(false);
    abort();
}
#endif
#endif
#endif

#ifndef CORE_ASSERT
#define CORE_ASSERT(cond, msg)                                                                     \
    do {                                                                                           \
        assert((cond) && (msg));                                                                   \
    } while (0)
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

#ifndef CORE_UNREACHABLE_CASE
#if defined(__GNUC__) && __GNUC__ < 8
#define CORE_UNREACHABLE_CASE                                                                      \
    default: CORE_UNREACHABLE()
#else
#define CORE_UNREACHABLE_CASE
#endif
#endif
