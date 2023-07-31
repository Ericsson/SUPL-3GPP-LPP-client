#pragma once

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
#define UBLOX_NODISCARD [[nodiscard]]
#endif
#endif
#ifndef UBLOX_NODISCARD
#define UBLOX_NODISCARD
#endif
#endif

#include <cstdint>

