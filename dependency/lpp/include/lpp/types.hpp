#pragma once
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>

#ifndef LPP_EXPLICIT
#define LPP_EXPLICIT explicit
#endif

#ifndef LPP_NOEXCEPT
#define LPP_NOEXCEPT noexcept
#endif

#ifndef LPP_CONSTEXPR
#define LPP_CONSTEXPR constexpr
#endif

#ifndef LPP_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define LPP_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef LPP_UNUSED
#define LPP_UNUSED
#endif
#endif

#ifndef LPP_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define LPP_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef LPP_NODISCARD
#define LPP_NODISCARD
#endif
#endif

#ifndef LPP_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define LPP_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef LPP_UNREACHABLE
#define LPP_UNREACHABLE() lpp_unreachable()
__attribute__((noreturn)) inline void lpp_unreachable() {
    assert(false);
}
#endif
#endif

#ifndef LPP_ASSERT
#define LPP_ASSERT(cond, msg) assert((cond) && msg)
#endif
