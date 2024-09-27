#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>

#ifndef SPARTN_EXPLICIT
#define SPARTN_EXPLICIT explicit
#endif

#ifndef SPARTN_NOEXCEPT
#define SPARTN_NOEXCEPT noexcept
#endif

#ifndef SPARTN_CONSTEXPR
#define SPARTN_CONSTEXPR constexpr
#endif

#ifndef SPARTN_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define SPARTN_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef SPARTN_UNUSED
#define SPARTN_UNUSED
#endif
#endif

#ifndef SPARTN_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define SPARTN_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef SPARTN_NODISCARD
#define SPARTN_NODISCARD
#endif
#endif

#ifndef SPARTN_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define SPARTN_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef SPARTN_UNREACHABLE
#define SPARTN_UNREACHABLE() spartn_unreachable()
__attribute__((noreturn)) inline void spartn_unreachable() {
    assert(false);
}
#endif
#endif
