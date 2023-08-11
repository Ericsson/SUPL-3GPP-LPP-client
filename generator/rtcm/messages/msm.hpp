#pragma once
#include "generator.hpp"
#include "rtk_data.hpp"

extern generator::rtcm::Message generate_msm(uint32_t msm, bool last_msm, GenericGnssId gnss,
                                             const generator::rtcm::CommonObservationInfo& common,
                                             const generator::rtcm::Observations& observations);

RTCM_CONSTEXPR static int      mlt_size                  = 22;
RTCM_CONSTEXPR static uint64_t mlt_coefficient[mlt_size] = {
    1,    2,    4,    8,     16,    32,    64,     128,    256,    512,     1024,
    2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152,
};

RTCM_CONSTEXPR static uint64_t mlt_offset[mlt_size] = {
    0,        64,       256,       768,       2048,      5120,       12288,   28672,
    65536,    147456,   327680,    720896,    1572864,   3407872,    7340032, 15728640,
    33554432, 71303168, 150994944, 318767104, 671088640, 1409286144,
};

RTCM_CONSTEXPR static uint64_t mlt_base[mlt_size] = {
    0,   64,  96,  128, 160, 192, 224, 256, 288, 320, 352,
    384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704,
};

// Table 3.5-75 GNSS Phaserange Lock Time Indicator with Extended Range and
// Resolution (DF407)
inline double from_msm_lock_ex(long input_value) {
    auto value = static_cast<uint64_t>(input_value);
    if (value >= mlt_base[mlt_size - 1]) {
        return 67108864 / 1000.0;
    }

    for (auto i = 0; i < mlt_size - 1; i++) {
        auto start = mlt_base[i];
        auto end   = mlt_base[i + 1];
        if (value >= start && value < end) {
            return (mlt_coefficient[i] * value - mlt_offset[i]) / 1000.0;
        }
    }

    return 0.0;
}

RTCM_CONSTEXPR static uint64_t mlt2_table[16] = {
    0, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288,
};

inline double from_msm_lock(long value) {
    if (value < 0) return 0;
    if (value > 15) return mlt2_table[15] / 1000.0;
    return mlt2_table[value] / 1000.0;
}

inline long to_msm_lock_ex(double lock) {
    auto lock_ms = static_cast<long>(lock * 1000.0);

    if (lock < 0.0) return 0;
    if (lock < 0.064) return lock_ms;
    if (lock < 0.128) return (lock_ms + 64) / 2;
    if (lock < 0.256) return (lock_ms + 256) / 4;
    if (lock < 0.512) return (lock_ms + 768) / 8;
    if (lock < 1.024) return (lock_ms + 2048) / 16;
    if (lock < 2.048) return (lock_ms + 5120) / 32;
    if (lock < 4.096) return (lock_ms + 12288) / 64;
    if (lock < 8.192) return (lock_ms + 28672) / 128;
    if (lock < 16.384) return (lock_ms + 65536) / 256;
    if (lock < 32.768) return (lock_ms + 147456) / 512;
    if (lock < 65.536) return (lock_ms + 327680) / 1024;
    if (lock < 131.072) return (lock_ms + 720896) / 2048;
    if (lock < 262.144) return (lock_ms + 1572864) / 4096;
    if (lock < 524.288) return (lock_ms + 3407872) / 8192;
    if (lock < 1048.576) return (lock_ms + 7340032) / 16384;
    if (lock < 2097.152) return (lock_ms + 15728640) / 32768;
    if (lock < 4194.304) return (lock_ms + 33554432) / 65536;
    if (lock < 8388.608) return (lock_ms + 71303168) / 131072;
    if (lock < 16777.216) return (lock_ms + 150994944) / 262144;
    if (lock < 33554.432) return (lock_ms + 318767104) / 524288;
    if (lock < 67108.864) return (lock_ms + 671088640) / 1048576;
    return 704;
}

inline long to_msm_lock(double lock) {
    if (lock < 0.032) return 0;
    if (lock < 0.064) return 1;
    if (lock < 0.128) return 2;
    if (lock < 0.256) return 3;
    if (lock < 0.512) return 4;
    if (lock < 1.024) return 5;
    if (lock < 2.048) return 6;
    if (lock < 4.096) return 7;
    if (lock < 8.192) return 8;
    if (lock < 16.384) return 9;
    if (lock < 32.768) return 10;
    if (lock < 65.536) return 11;
    if (lock < 131.072) return 12;
    if (lock < 262.144) return 13;
    if (lock < 524.288) return 14;
    return 15;
}
