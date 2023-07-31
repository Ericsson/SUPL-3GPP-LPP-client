#pragma once

#ifndef IF_EXPLICIT
#define IF_EXPLICIT explicit
#endif

#ifndef IF_NOEXCEPT
#define IF_NOEXCEPT noexcept
#endif

#ifndef IF_CONSTEXPR
#define IF_CONSTEXPR constexpr
#endif

#ifndef IF_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define IF_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef IF_UNUSED
#define IF_UNUSED
#endif
#endif

#ifndef IF_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define IF_NODISCARD [[nodiscard]]
#endif
#endif
#ifndef IF_NODISCARD
#define IF_NODISCARD
#endif
#endif

#include <cstdint>

