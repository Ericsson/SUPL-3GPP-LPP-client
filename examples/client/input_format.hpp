#pragma once

#include <stdint.h>

using InputFormat                                      = uint64_t;
constexpr static InputFormat INPUT_FORMAT_NONE         = 0;
constexpr static InputFormat INPUT_FORMAT_UBX          = 1;
constexpr static InputFormat INPUT_FORMAT_NMEA         = 2;
constexpr static InputFormat INPUT_FORMAT_RTCM         = 4;
constexpr static InputFormat INPUT_FORMAT_CTRL         = 8;
constexpr static InputFormat INPUT_FORMAT_LPP_UPER     = 16;
constexpr static InputFormat INPUT_FORMAT_LPP_UPER_PAD = 32;
constexpr static InputFormat INPUT_FORMAT_RAW          = 64;
constexpr static InputFormat INPUT_FORMAT_ALL          = INPUT_FORMAT_UBX | INPUT_FORMAT_NMEA |
                                                INPUT_FORMAT_RTCM | INPUT_FORMAT_CTRL |
                                                INPUT_FORMAT_LPP_UPER | INPUT_FORMAT_RAW;
