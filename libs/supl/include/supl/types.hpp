#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdarg>

#ifndef SUPL_EXPLICIT
#define SUPL_EXPLICIT explicit
#endif

#ifndef SUPL_NOEXCEPT
#define SUPL_NOEXCEPT noexcept
#endif

#ifndef SUPL_CONSTEXPR
#define SUPL_CONSTEXPR constexpr
#endif

#ifndef SUPL_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define SUPL_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef SUPL_UNUSED
#define SUPL_UNUSED
#endif
#endif

#ifndef SUPL_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define SUPL_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef SUPL_NODISCARD
#define SUPL_NODISCARD
#endif
#endif

#ifndef SUPL_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define SUPL_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef SUPL_UNREACHABLE
#define SUPL_UNREACHABLE() supl_unreachable()
__attribute__((noreturn)) inline void supl_unreachable() {
    assert(false);
}
#endif
#endif
