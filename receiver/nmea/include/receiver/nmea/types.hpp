#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>

#ifndef NMEA_EXPLICIT
#define NMEA_EXPLICIT explicit
#endif

#ifndef NMEA_NOEXCEPT
#define NMEA_NOEXCEPT noexcept
#endif

#ifndef NMEA_CONSTEXPR
#define NMEA_CONSTEXPR constexpr
#endif

#ifndef NMEA_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define NMEA_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef NMEA_UNUSED
#define NMEA_UNUSED
#endif
#endif

#ifndef NMEA_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define NMEA_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef NMEA_NODISCARD
#define NMEA_NODISCARD
#endif
#endif

#ifndef NMEA_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define NMEA_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef NMEA_UNREACHABLE
#define NMEA_UNREACHABLE() nmea_unreachable()
__attribute__((noreturn)) inline void nmea_unreachable() {
    assert(false);
}
#endif
#endif

