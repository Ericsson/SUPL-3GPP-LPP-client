#pragma once
#include <bitset>
#include <cmath>
#include <stdint.h>
#include <iostream>
#include "datatypes.hpp"

#define PI_DF 3.1415926535897932

enum struct Conversion {
    NONE,
    SC2RAD,  // Semi circle to radians
    MINUTE,  // Multiply by 60
    PICO100,
};

template <int NUM, typename T, typename DT, int E = 0, Conversion C = Conversion::NONE>
struct DataField {
    DataField() : d{} {}

    DataField(T const& t) : d{t} {}
    operator T() const { return d; }

    T value() const { return d; }

    DataField& operator=(T const& t) {
        d = t;
        return *this;
    }

    using InternalType = T;
    static constexpr std::size_t num{NUM};
    static constexpr std::size_t len{DT::len};
    static constexpr Sign        sign{DT::sign};
    static constexpr int         exponent{E};

    static T convert(unsigned long long b) {
        if constexpr (sign == Sign::UNSIGNED)
            return multiply_by_factor(b);
        else if constexpr (sign == Sign::SIGNED)
            return multiply_by_factor(as_signed(b));
        else if constexpr (sign == Sign::SIGNED_MAGNITUDE)
            return multiply_by_factor(as_signed_magnitude(b));
    }

    template <typename K>
    static T multiply_by_factor(K value) {
        if constexpr (C == Conversion::PICO100 || C == Conversion::MINUTE ||
                      C == Conversion::SC2RAD) {
            return static_cast<T>(static_cast<double>(value) * factor);
        } else {
            return static_cast<T>(value);
        }
    }

private:
    T d;

    static signed long long as_signed(unsigned long long b) {
        if (b & (1ull << (len - 1)))
            return static_cast<signed long long>(b | (~0ull << len));
        else
            return static_cast<signed long long>(b);
    }
    static signed long long as_signed_magnitude(unsigned long long b) {
        if (b & (1ull << (len - 1)))
            return static_cast<signed long long>(-(b & ((1ull << (len - 1)) - 1ull)));
        else
            return static_cast<signed long long>(b);
    }

    static constexpr double pow2(int n) {
        return (n == 0) ? 1.0 : (n > 0) ? pow2(n - 1) * 2.0 : pow2(n + 1) / 2.0;
    }
    static double constexpr _factor() {
        return pow2(exponent) * (C == Conversion::SC2RAD  ? PI_DF :
                                 C == Conversion::MINUTE  ? 60.0 :
                                 C == Conversion::PICO100 ? 1e-10 :
                                                            1);
    }
    static constexpr T factor{static_cast<T>(_factor())};
};

template <std::size_t N>
unsigned long long getsubbits(std::bitset<N> data, std::size_t i, std::size_t l) {
    auto d = (data >> (N - i - l) & std::bitset<N>((1ull << l) - 1)).to_ullong();
    return d;
}

template <std::size_t N, typename DF>
void getdatafield(std::bitset<N> const& data, std::size_t& i, DF& dest) {
    auto bits = getsubbits(data, i, DF::len);
    dest      = DF::convert(bits);
    i += DF::len;
}

// clang-format off
//                          Desired storage type in S3LP client
//                            |       Data Type of Data Field in RTCM message
//                            |         |     Exponent of first conversion factor
//                            |         |       |         Conversion of second factor
//                            |         |       |           |
//                            V         V       V           V
using DF001 = DataField<  1, uint8_t , df_bit<2>                          >;
using DF002 = DataField<  2, uint16_t, df_uint12                          >;
using DF009 = DataField<  9, uint8_t , df_uint6                           >;
using DF071 = DataField< 71, uint8_t , df_uint8                           >;
using DF076 = DataField< 76, uint16_t, df_uint10                          >; // GPS Week Number
using DF077 = DataField< 77, uint8_t , df_uint4                           >; // GPS SV ACCURACY
using DF078 = DataField< 78, uint8_t , df_uint2                           >;
using DF079 = DataField< 79, double  , df_int14 , -43, Conversion::SC2RAD >;
using DF081 = DataField< 81, double  , df_uint16,   4                     >;
using DF082 = DataField< 82, double  , df_int8  , -55                     >;
using DF083 = DataField< 83, double  , df_int16 , -43                     >;
using DF084 = DataField< 84, double  , df_int22 , -31                     >;
using DF085 = DataField< 85, uint16_t, df_uint10                          >;
using DF086 = DataField< 86, double  , df_int16 ,  -5                     >;
using DF087 = DataField< 87, double  , df_int16 , -43, Conversion::SC2RAD >;
using DF088 = DataField< 88, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF089 = DataField< 89, double  , df_int16 , -29                     >;
using DF090 = DataField< 90, double  , df_uint32, -33                     >;
using DF091 = DataField< 91, double  , df_int16 , -29                     >;
using DF092 = DataField< 92, double  , df_uint32, -19                     >;
using DF093 = DataField< 93, double  , df_uint16,   4                     >;
using DF094 = DataField< 94, double  , df_int16 , -29                     >;
using DF095 = DataField< 95, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF096 = DataField< 96, double  , df_int16 , -29                     >;
using DF097 = DataField< 97, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF098 = DataField< 98, double  , df_int16 ,  -5                     >;
using DF099 = DataField< 99, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF100 = DataField<100, double  , df_int24 , -43, Conversion::SC2RAD >;
using DF101 = DataField<101, double  , df_int8  , -31                     >;
using DF102 = DataField<102, uint8_t , df_uint6                           >;
using DF103 = DataField<103, bool    , df_bit<1>                          >;
using DF137 = DataField<137, bool    , df_bit<1>                          >;
using DF252 = DataField<252, uint8_t , df_uint6                           >;
using DF286 = DataField<286, uint8_t , df_uint8                           >;
using DF287 = DataField<287, uint8_t , df_bit<2>                          >;
using DF288 = DataField<288, bool    , df_bit<1>                          >;
using DF289 = DataField<289, uint16_t, df_uint12                          >;
using DF290 = DataField<290, uint16_t, df_uint10                          >;
using DF292 = DataField<292, double  , df_int14 , -43, Conversion::SC2RAD >;
using DF293 = DataField<293, double  , df_uint14,   0, Conversion::MINUTE >;
using DF294 = DataField<294, double  , df_int6  , -59                     >;
using DF295 = DataField<295, double  , df_int21 , -46                     >;
using DF296 = DataField<296, double  , df_int31 , -34                     >;
using DF297 = DataField<297, double  , df_int16 ,  -5                     >;
using DF298 = DataField<298, double  , df_int16 , -43, Conversion::SC2RAD >;
using DF299 = DataField<299, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF300 = DataField<300, double  , df_int16 , -29                     >;
using DF301 = DataField<301, double  , df_uint32, -33                     >;
using DF302 = DataField<302, double  , df_int16 , -29                     >;
using DF303 = DataField<303, double  , df_uint32, -19                     >;
using DF304 = DataField<304, double  , df_uint14,   0, Conversion::MINUTE >;
using DF305 = DataField<305, double  , df_int16 , -29                     >;
using DF306 = DataField<306, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF307 = DataField<307, double  , df_int16 , -29                     >;
using DF308 = DataField<308, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF309 = DataField<309, double  , df_int16 ,  -5                     >;
using DF310 = DataField<310, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF311 = DataField<311, double  , df_int24 , -43, Conversion::SC2RAD >;
using DF312 = DataField<312, double  , df_int10 , -32                     >;
using DF313 = DataField<313, double  , df_int10 , -32                     >;
using DF316 = DataField<316, uint8_t , df_bit<2>                          >;
using DF317 = DataField<317, bool    , df_bit<1>                          >;
using DF488 = DataField<488, uint8_t , df_uint6                           >;
using DF489 = DataField<489, uint16_t, df_uint13                          >;
using DF490 = DataField<490, uint8_t , df_bit<4>                          >;
using DF491 = DataField<491, double  , df_int14 , -43, Conversion::SC2RAD >;
using DF492 = DataField<492, uint8_t , df_uint5                           >;
using DF493 = DataField<493, double  , df_uint17,   3                     >;
using DF494 = DataField<494, double  , df_int11 , -66                     >;
using DF495 = DataField<495, double  , df_int22 , -50                     >;
using DF496 = DataField<496, double  , df_int24 , -33                     >;
using DF497 = DataField<497, uint8_t , df_uint5                           >;
using DF498 = DataField<498, double  , df_int18 ,  -6                     >;
using DF499 = DataField<499, double  , df_int16 , -43, Conversion::SC2RAD >;
using DF500 = DataField<500, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF501 = DataField<501, double  , df_int18 , -31                     >;
using DF502 = DataField<502, double  , df_uint32, -33                     >;
using DF503 = DataField<503, double  , df_int18 , -31                     >;
using DF504 = DataField<504, double  , df_uint32, -19                     >;
using DF505 = DataField<505, double  , df_uint17,   3                     >;
using DF506 = DataField<506, double  , df_int18 , -31                     >;
using DF507 = DataField<507, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF508 = DataField<508, double  , df_int18 , -31                     >;
using DF509 = DataField<509, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF510 = DataField<510, double  , df_int18 ,  -6                     >;
using DF511 = DataField<511, double  , df_int32 , -31, Conversion::SC2RAD >;
using DF512 = DataField<512, double  , df_int24 , -43, Conversion::SC2RAD >;
using DF513 = DataField<513, double  , df_int10 ,   0, Conversion::PICO100>;
using DF514 = DataField<514, double  , df_int10 ,   0, Conversion::PICO100>;
using DF515 = DataField<515, uint8_t , df_bit<1>                          >; // Is uint8_t in ephem, bool?
// clang-format on

namespace std {
ostream& operator<<(ostream& os, const DF002 d);
ostream& operator<<(ostream& os, const DF009 d);
ostream& operator<<(ostream& os, const DF071 d);
ostream& operator<<(ostream& os, const DF076 d);
ostream& operator<<(ostream& os, const DF077 d);
ostream& operator<<(ostream& os, const DF078 d);
ostream& operator<<(ostream& os, const DF079 d);
ostream& operator<<(ostream& os, const DF081 d);
ostream& operator<<(ostream& os, const DF082 d);
ostream& operator<<(ostream& os, const DF083 d);
ostream& operator<<(ostream& os, const DF084 d);
ostream& operator<<(ostream& os, const DF085 d);
ostream& operator<<(ostream& os, const DF086 d);
ostream& operator<<(ostream& os, const DF087 d);
ostream& operator<<(ostream& os, const DF088 d);
ostream& operator<<(ostream& os, const DF089 d);
ostream& operator<<(ostream& os, const DF090 d);
ostream& operator<<(ostream& os, const DF091 d);
ostream& operator<<(ostream& os, const DF092 d);
ostream& operator<<(ostream& os, const DF093 d);
ostream& operator<<(ostream& os, const DF094 d);
ostream& operator<<(ostream& os, const DF095 d);
ostream& operator<<(ostream& os, const DF096 d);
ostream& operator<<(ostream& os, const DF097 d);
ostream& operator<<(ostream& os, const DF098 d);
ostream& operator<<(ostream& os, const DF099 d);
ostream& operator<<(ostream& os, const DF100 d);
ostream& operator<<(ostream& os, const DF101 d);
ostream& operator<<(ostream& os, const DF102 d);
ostream& operator<<(ostream& os, const DF103 d);
ostream& operator<<(ostream& os, const DF137 d);
}  // namespace std
