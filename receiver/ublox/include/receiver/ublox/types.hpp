#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>

#ifndef UBLOX_EXPLICIT
#define UBLOX_EXPLICIT explicit
#endif

#ifndef UBLOX_NOEXCEPT
#define UBLOX_NOEXCEPT noexcept
#endif

#ifndef UBLOX_CONSTEXPR
#define UBLOX_CONSTEXPR constexpr
#endif

#ifndef UBLOX_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define UBLOX_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef UBLOX_UNUSED
#define UBLOX_UNUSED
#endif
#endif

#ifndef UBLOX_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define UBLOX_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef UBLOX_NODISCARD
#define UBLOX_NODISCARD
#endif
#endif

#ifndef UBLOX_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define UBLOX_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef UBLOX_UNREACHABLE
#define UBLOX_UNREACHABLE() ublox_unreachable()
__attribute__((noreturn)) inline void ublox_unreachable() {
    assert(false);
}
#endif
#endif

