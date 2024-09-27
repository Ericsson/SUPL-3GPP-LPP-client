#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>

#ifndef RTCM_EXPLICIT
#define RTCM_EXPLICIT explicit
#endif

#ifndef RTCM_NOEXCEPT
#define RTCM_NOEXCEPT noexcept
#endif

#ifndef RTCM_CONSTEXPR
#define RTCM_CONSTEXPR constexpr
#endif

#ifndef RTCM_UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define RTCM_UNUSED [[maybe_unused]]
#endif
#endif
#ifndef RTCM_UNUSED
#define RTCM_UNUSED
#endif
#endif

#ifndef RTCM_NODISCARD
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#if defined(__cplusplus) && __cplusplus >= 201703L
#define RTCM_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#ifndef RTCM_NODISCARD
#define RTCM_NODISCARD
#endif
#endif

#ifndef RTCM_UNREACHABLE
#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define RTCM_UNREACHABLE() __builtin_unreachable()
#endif
#endif
#ifndef RTCM_UNREACHABLE
#define RTCM_UNREACHABLE() rtcm_unreachable()
__attribute__((noreturn)) inline void rtcm_unreachable() {
    assert(false);
}
#endif
#endif

#if defined(__cplusplus) && __cplusplus >= 201703L
#define RTCM_N2_1 0x1p-1
#define RTCM_N2_2 0x1p-2
#define RTCM_N2_3 0x1p-3
#define RTCM_N2_4 0x1p-4
#define RTCM_N2_5 0x1p-5
#define RTCM_N2_6 0x1p-6
#define RTCM_N2_7 0x1p-7
#define RTCM_N2_8 0x1p-8
#define RTCM_N2_9 0x1p-9
#define RTCM_N2_10 0x1p-10
#define RTCM_N2_11 0x1p-11
#define RTCM_N2_12 0x1p-12
#define RTCM_N2_13 0x1p-13
#define RTCM_N2_14 0x1p-14
#define RTCM_N2_15 0x1p-15
#define RTCM_N2_16 0x1p-16
#define RTCM_N2_17 0x1p-17
#define RTCM_N2_18 0x1p-18
#define RTCM_N2_19 0x1p-19
#define RTCM_N2_20 0x1p-20
#define RTCM_N2_21 0x1p-21
#define RTCM_N2_22 0x1p-22
#define RTCM_N2_23 0x1p-23
#define RTCM_N2_24 0x1p-24
#define RTCM_N2_25 0x1p-25
#define RTCM_N2_26 0x1p-26
#define RTCM_N2_27 0x1p-27
#define RTCM_N2_28 0x1p-28
#define RTCM_N2_29 0x1p-29
#define RTCM_N2_30 0x1p-30
#define RTCM_N2_31 0x1p-32

#define RTCM_P2_1 0x1p1
#define RTCM_P2_2 0x1p2
#define RTCM_P2_3 0x1p3
#define RTCM_P2_4 0x1p4
#define RTCM_P2_5 0x1p5
#define RTCM_P2_6 0x1p6
#define RTCM_P2_7 0x1p7
#define RTCM_P2_8 0x1p8
#define RTCM_P2_9 0x1p9
#define RTCM_P2_10 0x1p10
#define RTCM_P2_11 0x1p11
#define RTCM_P2_12 0x1p12
#define RTCM_P2_13 0x1p13
#define RTCM_P2_14 0x1p14
#define RTCM_P2_15 0x1p15
#define RTCM_P2_16 0x1p16
#define RTCM_P2_17 0x1p17
#define RTCM_P2_18 0x1p18
#define RTCM_P2_19 0x1p19
#define RTCM_P2_20 0x1p20
#define RTCM_P2_21 0x1p21
#define RTCM_P2_22 0x1p22
#define RTCM_P2_23 0x1p23
#define RTCM_P2_24 0x1p24
#define RTCM_P2_25 0x1p25
#define RTCM_P2_26 0x1p26
#define RTCM_P2_27 0x1p27
#define RTCM_P2_28 0x1p28
#define RTCM_P2_29 0x1p29
#define RTCM_P2_30 0x1p30
#define RTCM_P2_31 0x1p31
#else
#define RTCM_CALC_P2_N(N) (static_cast<double>((static_cast<uint64_t>(1) << (N))))
#define RTCM_CALC_N2_N(N) (1.0 / RTCM_CALC_P2_N(N))
#define RTCM_N2_1 RTCM_CALC_N2_N(1)
#define RTCM_N2_2 RTCM_CALC_N2_N(2)
#define RTCM_N2_3 RTCM_CALC_N2_N(3)
#define RTCM_N2_4 RTCM_CALC_N2_N(4)
#define RTCM_N2_5 RTCM_CALC_N2_N(5)
#define RTCM_N2_6 RTCM_CALC_N2_N(6)
#define RTCM_N2_7 RTCM_CALC_N2_N(7)
#define RTCM_N2_8 RTCM_CALC_N2_N(8)
#define RTCM_N2_9 RTCM_CALC_N2_N(9)
#define RTCM_N2_10 RTCM_CALC_N2_N(10)
#define RTCM_N2_11 RTCM_CALC_N2_N(11)
#define RTCM_N2_12 RTCM_CALC_N2_N(12)
#define RTCM_N2_13 RTCM_CALC_N2_N(13)
#define RTCM_N2_14 RTCM_CALC_N2_N(14)
#define RTCM_N2_15 RTCM_CALC_N2_N(15)
#define RTCM_N2_16 RTCM_CALC_N2_N(16)
#define RTCM_N2_17 RTCM_CALC_N2_N(17)
#define RTCM_N2_18 RTCM_CALC_N2_N(18)
#define RTCM_N2_19 RTCM_CALC_N2_N(19)
#define RTCM_N2_20 RTCM_CALC_N2_N(20)
#define RTCM_N2_21 RTCM_CALC_N2_N(21)
#define RTCM_N2_22 RTCM_CALC_N2_N(22)
#define RTCM_N2_23 RTCM_CALC_N2_N(23)
#define RTCM_N2_24 RTCM_CALC_N2_N(24)
#define RTCM_N2_25 RTCM_CALC_N2_N(25)
#define RTCM_N2_26 RTCM_CALC_N2_N(26)
#define RTCM_N2_27 RTCM_CALC_N2_N(27)
#define RTCM_N2_28 RTCM_CALC_N2_N(28)
#define RTCM_N2_29 RTCM_CALC_N2_N(29)
#define RTCM_N2_30 RTCM_CALC_N2_N(30)
#define RTCM_N2_31 RTCM_CALC_N2_N(31)

#define RTCM_P2_1 RTCM_CALC_P2_N(1)
#define RTCM_P2_2 RTCM_CALC_P2_N(2)
#define RTCM_P2_3 RTCM_CALC_P2_N(3)
#define RTCM_P2_4 RTCM_CALC_P2_N(4)
#define RTCM_P2_5 RTCM_CALC_P2_N(5)
#define RTCM_P2_6 RTCM_CALC_P2_N(6)
#define RTCM_P2_7 RTCM_CALC_P2_N(7)
#define RTCM_P2_8 RTCM_CALC_P2_N(8)
#define RTCM_P2_9 RTCM_CALC_P2_N(9)
#define RTCM_P2_10 RTCM_CALC_P2_N(10)
#define RTCM_P2_11 RTCM_CALC_P2_N(11)
#define RTCM_P2_12 RTCM_CALC_P2_N(12)
#define RTCM_P2_13 RTCM_CALC_P2_N(13)
#define RTCM_P2_14 RTCM_CALC_P2_N(14)
#define RTCM_P2_15 RTCM_CALC_P2_N(15)
#define RTCM_P2_16 RTCM_CALC_P2_N(16)
#define RTCM_P2_17 RTCM_CALC_P2_N(17)
#define RTCM_P2_18 RTCM_CALC_P2_N(18)
#define RTCM_P2_19 RTCM_CALC_P2_N(19)
#define RTCM_P2_20 RTCM_CALC_P2_N(20)
#define RTCM_P2_21 RTCM_CALC_P2_N(21)
#define RTCM_P2_22 RTCM_CALC_P2_N(22)
#define RTCM_P2_23 RTCM_CALC_P2_N(23)
#define RTCM_P2_24 RTCM_CALC_P2_N(24)
#define RTCM_P2_25 RTCM_CALC_P2_N(25)
#define RTCM_P2_26 RTCM_CALC_P2_N(26)
#define RTCM_P2_27 RTCM_CALC_P2_N(27)
#define RTCM_P2_28 RTCM_CALC_P2_N(28)
#define RTCM_P2_29 RTCM_CALC_P2_N(29)
#define RTCM_P2_30 RTCM_CALC_P2_N(30)
#define RTCM_P2_31 RTCM_CALC_P2_N(31)
#endif
