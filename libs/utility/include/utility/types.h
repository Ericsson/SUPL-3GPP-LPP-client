#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;

typedef float  f32;
typedef double f64;

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define NO_DISCARD [[nodiscard]]
#else
#define NO_DISCARD
#endif
#else
#define NO_DISCARD
#endif
#else
#define NO_DISCARD
#endif

#ifndef UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define UNUSED [[maybe_unused]]
#else
#define UNUSED
#endif
#else
#define UNUSED
#endif
#endif

enum class GNSS_System {
    GPS     = 0,
    SBAS    = 1,
    QZSS    = 2,
    GALILEO = 3,
    GLONASS = 4,
    BDS     = 5,

    UNKNOWN = -1,
};
