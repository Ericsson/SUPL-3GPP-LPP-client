#pragma once

#include <stdint.h>

using OutputFormat                                   = uint64_t;
constexpr static OutputFormat OUTPUT_FORMAT_NONE     = 0;
constexpr static OutputFormat OUTPUT_FORMAT_UBX      = 1;
constexpr static OutputFormat OUTPUT_FORMAT_NMEA     = 2;
constexpr static OutputFormat OUTPUT_FORMAT_RTCM     = 4;
constexpr static OutputFormat OUTPUT_FORMAT_CTRL     = 8;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_XER  = 16;
constexpr static OutputFormat OUTPUT_FORMAT_LPP_UPER = 32;
constexpr static OutputFormat OUTPUT_FORMAT_UNUSED64 = 64;
constexpr static OutputFormat OUTPUT_FORMAT_SPARTN   = 128;
constexpr static OutputFormat OUTPUT_FORMAT_LFR      = 256;
constexpr static OutputFormat OUTPUT_FORMAT_POSSIB   = 512;
constexpr static OutputFormat OUTPUT_FORMAT_LOCATION = 1024;
constexpr static OutputFormat OUTPUT_FORMAT_TLF      = 1llu << 62;
constexpr static OutputFormat OUTPUT_FORMAT_TEST     = 1llu << 63;
constexpr static OutputFormat OUTPUT_FORMAT_ALL =
    OUTPUT_FORMAT_UBX | OUTPUT_FORMAT_NMEA | OUTPUT_FORMAT_RTCM | OUTPUT_FORMAT_CTRL |
    OUTPUT_FORMAT_LPP_XER | OUTPUT_FORMAT_LPP_UPER | OUTPUT_FORMAT_SPARTN | OUTPUT_FORMAT_LFR |
    OUTPUT_FORMAT_POSSIB;
