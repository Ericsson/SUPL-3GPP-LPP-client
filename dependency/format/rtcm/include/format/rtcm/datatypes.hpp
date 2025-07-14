#pragma once
#include <cstddef>

enum struct Sign {
    UNSIGNED,
    SIGNED,
    SIGNED_MAGNITUDE,
};

template<std::size_t L, Sign S>
struct DataType {
    static constexpr std::size_t len { L };
    static constexpr Sign sign { S };
};

template<std::size_t N>
struct df_bit    : DataType< N, Sign::UNSIGNED> {};
struct df_int8   : DataType< 8, Sign::SIGNED> {};
struct df_int9   : DataType< 9, Sign::SIGNED> {};
struct df_int10  : DataType<10, Sign::SIGNED> {};
struct df_int14  : DataType<14, Sign::SIGNED> {};
struct df_int15  : DataType<15, Sign::SIGNED> {};
struct df_int16  : DataType<16, Sign::SIGNED> {};
struct df_int17  : DataType<17, Sign::SIGNED> {};
struct df_int19  : DataType<19, Sign::SIGNED> {};
struct df_int20  : DataType<20, Sign::SIGNED> {};
struct df_int21  : DataType<21, Sign::SIGNED> {};
struct df_int22  : DataType<22, Sign::SIGNED> {};
struct df_int23  : DataType<23, Sign::SIGNED> {};
struct df_int24  : DataType<24, Sign::SIGNED> {};
struct df_int25  : DataType<25, Sign::SIGNED> {};
struct df_int26  : DataType<26, Sign::SIGNED> {};
struct df_int27  : DataType<27, Sign::SIGNED> {};
struct df_int30  : DataType<30, Sign::SIGNED> {};
struct df_int32  : DataType<32, Sign::SIGNED> {};
struct df_int34  : DataType<34, Sign::SIGNED> {};
struct df_int35  : DataType<35, Sign::SIGNED> {};
struct df_int38  : DataType<38, Sign::SIGNED> {};
struct df_uint2  : DataType< 2, Sign::UNSIGNED> {};
struct df_uint3  : DataType< 3, Sign::UNSIGNED> {};
struct df_uint4  : DataType< 4, Sign::UNSIGNED> {};
struct df_uint5  : DataType< 5, Sign::UNSIGNED> {};
struct df_uint6  : DataType< 6, Sign::UNSIGNED> {};
struct df_uint7  : DataType< 7, Sign::UNSIGNED> {};
struct df_uint8  : DataType< 8, Sign::UNSIGNED> {};
struct df_uint9  : DataType< 9, Sign::UNSIGNED> {};
struct df_uint10 : DataType<10, Sign::UNSIGNED> {};
struct df_uint11 : DataType<11, Sign::UNSIGNED> {};
struct df_uint12 : DataType<12, Sign::UNSIGNED> {};
struct df_uint14 : DataType<14, Sign::UNSIGNED> {};
struct df_uint16 : DataType<16, Sign::UNSIGNED> {};
struct df_uint17 : DataType<17, Sign::UNSIGNED> {};
struct df_uint18 : DataType<18, Sign::UNSIGNED> {};
struct df_uint20 : DataType<20, Sign::UNSIGNED> {};
struct df_uint23 : DataType<23, Sign::UNSIGNED> {};
struct df_uint24 : DataType<24, Sign::UNSIGNED> {};
struct df_uint25 : DataType<25, Sign::UNSIGNED> {};
struct df_uint26 : DataType<26, Sign::UNSIGNED> {};
struct df_uint27 : DataType<27, Sign::UNSIGNED> {};
struct df_uint30 : DataType<30, Sign::UNSIGNED> {};
struct df_uint32 : DataType<32, Sign::UNSIGNED> {};
struct df_uint35 : DataType<35, Sign::UNSIGNED> {};
struct df_uint36 : DataType<36, Sign::UNSIGNED> {};
struct df_intS5  : DataType< 5, Sign::SIGNED_MAGNITUDE> {};
struct df_intS11 : DataType<11, Sign::SIGNED_MAGNITUDE> {};
struct df_intS22 : DataType<22, Sign::SIGNED_MAGNITUDE> {};
struct df_intS24 : DataType<24, Sign::SIGNED_MAGNITUDE> {};
struct df_intS27 : DataType<27, Sign::SIGNED_MAGNITUDE> {};
struct df_intS32 : DataType<32, Sign::SIGNED_MAGNITUDE> {};
// utf8(N) Unicode UTF-8 Code Unit 00h to FFh 8-bit value that contains all or part of a Unicode UTF-8 encoded character
