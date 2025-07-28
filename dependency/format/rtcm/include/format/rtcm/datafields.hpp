#pragma once
#include <bitset>
#include "datatypes.hpp"
#include <cmath>
#include <iostream>
#include <stdint.h>

#define PI_DF 3.1415926535897932

enum struct Conversion {
    NONE,
    SC2RAD, // Semi circle to radians
};


template<int NUM, typename T, typename DT, int E = 0, Conversion C = Conversion::NONE>
struct DataField {
    DataField( ) : d{} { }

    DataField(T const& t) : d{t} { }
    operator T() const { return d; }

    DataField& operator=(T const& t) {
        d = t;
        return *this;
    }

    using InternalType = T;
    static constexpr std::size_t num      { NUM      };
    static constexpr std::size_t len      { DT::len  };
    static constexpr Sign        sign     { DT::sign };
    static constexpr int         exponent { E        };

    static T convert(unsigned long long b) {
        if      constexpr (sign==Sign::UNSIGNED)
            return b * factor;
        else if constexpr (sign==Sign::SIGNED)
            return as_signed(b) * factor;
        else if constexpr (sign==Sign::SIGNED_MAGNITUDE)
            return as_signed_magnitude(b) * factor;
    }
private:
    T d;

    static signed long long as_signed(unsigned long long b) {
        if (b&(1ull<<(len-1))) return static_cast<signed long long>(b|(~0ull<<len));
        else                   return static_cast<signed long long>(b);
    }
    static signed long long as_signed_magnitude(unsigned long long b) {
        if (b&(1ull<<(len-1))) return static_cast<signed long long>(-(b&((1ull<<(len-1))-1ull)));
        else                   return static_cast<signed long long>(b);
    }

    static constexpr double pow2(int n) {
        return (n == 0) ? 1.0               :
               (n > 0)  ? pow2(n - 1) * 2.0 :
                          pow2(n + 1) / 2.0;
    }
    static T constexpr _factor() {
        return pow2(exponent) * (C==Conversion::SC2RAD ? PI_DF : 1);
    }
    static constexpr T factor { _factor() };
};

template<std::size_t N>
unsigned long long getsubbits(std::bitset<N> data, std::size_t i, std::size_t l) {
    auto d = (data >> (N-i-l) & std::bitset<N>((1ull<<l) - 1)).to_ullong();
    return d;
}

template<std::size_t N, typename DF>
void getdatafield(std::bitset<N> const& data, std::size_t& i, DF &dest) {
    auto bits = getsubbits(data, i, DF::len);
    dest      = DF::convert(bits);
    i += DF::len;
}

//                          Desired storage type in S3LP client
//                            |       Data Type of Data Field in RTCM message
//                            |         |     Exponent of first conversion factor
//                            |         |       |         Conversion of second factor
//                            |         |       |           |
//                            V         V       V           V
using DF002 = DataField<  2, int   , df_uint12                         >;
using DF009 = DataField<  9, int   , df_uint6                          >;
using DF076 = DataField< 76, int   , df_uint10                         >; // GPS Week Number
using DF077 = DataField< 77, int   , df_uint4                          >; // GPS SV ACCURACY
using DF078 = DataField< 78, int   , df_uint2                          >;
using DF079 = DataField< 79, double, df_int14 , -43, Conversion::SC2RAD>;
using DF071 = DataField< 71, int   , df_uint8                          >;
using DF081 = DataField< 81, double, df_uint16,   4                    >;
using DF082 = DataField< 82, double, df_int8  , -55                    >;
using DF083 = DataField< 83, double, df_int16 , -43                    >;
using DF084 = DataField< 84, double, df_int22 , -31                    >;
using DF085 = DataField< 85, int   , df_uint10                         >;
using DF086 = DataField< 86, double, df_int16 ,  -5                    >;
using DF087 = DataField< 87, double, df_int16 , -43, Conversion::SC2RAD>;
using DF088 = DataField< 88, double, df_int32 , -31, Conversion::SC2RAD>;
using DF089 = DataField< 89, double, df_int16 , -29                    >;
using DF090 = DataField< 90, double, df_uint32, -33                    >;
using DF091 = DataField< 91, double, df_int16 , -29                    >;
using DF092 = DataField< 92, double, df_uint32, -19                    >;
using DF093 = DataField< 93, double, df_uint16,   4                    >;
using DF094 = DataField< 94, double, df_int16 , -29                    >;
using DF095 = DataField< 95, double, df_int32 , -31, Conversion::SC2RAD>;
using DF096 = DataField< 96, double, df_int16 , -29                    >;
using DF097 = DataField< 97, double, df_int32 , -31, Conversion::SC2RAD>;
using DF098 = DataField< 98, double, df_int16 ,  -5                    >;
using DF099 = DataField< 99, double, df_int32 , -31, Conversion::SC2RAD>;
using DF100 = DataField<100, double, df_int24 , -43, Conversion::SC2RAD>;
using DF101 = DataField<101, double, df_int8  , -31                    >;
using DF102 = DataField<102, int   , df_uint6                          >;
using DF103 = DataField<103, int   , df_bit<1>                         >;
using DF137 = DataField<137, double, df_bit<1>                         >;

/*
using DF252 = DataField<252, double, df_uint24, -43, Conversion::SC2RAD>;
using DF289 = DataField<289, double, df_uint24, -43, Conversion::SC2RAD>;
using DF290 = DataField<290, double, df_uint24, -43, Conversion::SC2RAD>;
using DF286 = DataField<286, double, df_uint24, -43, Conversion::SC2RAD>;
using DF292 = DataField<292, double, df_uint24, -43, Conversion::SC2RAD>;
using DF293 = DataField<293, double, df_uint24, -43, Conversion::SC2RAD>;
using DF294 = DataField<294, double, df_uint24, -43, Conversion::SC2RAD>;
using DF295 = DataField<295, double, df_uint24, -43, Conversion::SC2RAD>;
using DF296 = DataField<296, double, df_uint24, -43, Conversion::SC2RAD>;
using DF297 = DataField<297, double, df_uint24, -43, Conversion::SC2RAD>;
using DF298 = DataField<298, double, df_uint24, -43, Conversion::SC2RAD>;
using DF299 = DataField<299, double, df_uint24, -43, Conversion::SC2RAD>;
using DF300 = DataField<300, double, df_uint24, -43, Conversion::SC2RAD>;
using DF301 = DataField<301, double, df_uint24, -43, Conversion::SC2RAD>;
using DF302 = DataField<302, double, df_uint24, -43, Conversion::SC2RAD>;
using DF303 = DataField<303, double, df_uint24, -43, Conversion::SC2RAD>;
using DF304 = DataField<304, double, df_uint24, -43, Conversion::SC2RAD>;
using DF305 = DataField<305, double, df_uint24, -43, Conversion::SC2RAD>;
using DF306 = DataField<306, double, df_uint24, -43, Conversion::SC2RAD>;
using DF307 = DataField<307, double, df_uint24, -43, Conversion::SC2RAD>;
using DF308 = DataField<308, double, df_uint24, -43, Conversion::SC2RAD>;
using DF309 = DataField<309, double, df_uint24, -43, Conversion::SC2RAD>;
using DF310 = DataField<310, double, df_uint24, -43, Conversion::SC2RAD>;
using DF311 = DataField<311, double, df_uint24, -43, Conversion::SC2RAD>;
using DF312 = DataField<312, double, df_uint24, -43, Conversion::SC2RAD>;
using DF313 = DataField<313, double, df_uint24, -43, Conversion::SC2RAD>;
using DF316 = DataField<316, double, df_uint24, -43, Conversion::SC2RAD>;
using DF317 = DataField<317, double, df_uint24, -43, Conversion::SC2RAD>;
using DF287 = DataField<287, double, df_uint24, -43, Conversion::SC2RAD>;
using DF288 = DataField<288, double, df_uint24, -43, Conversion::SC2RAD>;
*/

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
} // namespace std
