#pragma once
#include <bitset>
#include "datatypes.hpp"
#include <cmath>
#include <stdint.h>

#define PI_DF 3.1415926535897932

enum struct Conversion {
    NONE,
    SC2RAD, // Semi circle to radians
};


template<typename T, typename DT, int E = 0, Conversion C = Conversion::NONE>
struct DataField {
    DataField( ) : d{} { }

    DataField(T const& t) : d{t} { }
    operator T() const { return d; }

    DataField& operator=(T const& t) {
        d = t;
        return *this;
    }

    using InternalType = T;
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
        if (b&(1u<<(len-1))) return static_cast<signed long long>(b|(~0u<<len));
        else                 return static_cast<signed long long>(b);
    }
    static signed long long as_signed_magnitude(unsigned long long b) {
        if (b&(1u<<(len-1))) return static_cast<signed long long>(-(b&((1u<<(len-1))-1)));
        else                 return static_cast<signed long long>(b);
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
    auto d = (data >> (N-i-l) & std::bitset<N>((1<<l) - 1)).to_ullong();
    return d;
}

template<std::size_t N, typename DF>
void getdatafield(std::bitset<N> const& data, std::size_t& i, DF &dest) {
    auto bits = getsubbits(data, i, DF::len);
    dest      = DF::convert(bits);
    i += DF::len;
}

//                      Desired storage type in S3LP client
//                        |       Data Type of Data Field in RTCM message
//                        |         |     Exponent of first conversion factor
//                        |         |       |         Conversion of second factor
//                        |         |       |           |
//                        V         V       V           V
using DF002 = DataField<int     , df_uint12                         >;
using DF009 = DataField<uint8_t , df_uint6                          >;
using DF076 = DataField<uint16_t, df_uint10                         >; // GPS Week Number
using DF077 = DataField<uint8_t , df_uint4                          >; // GPS SV ACCURACY
using DF078 = DataField<uint8_t , df_uint2                          >;
using DF079 = DataField<double  , df_int14 , -43, Conversion::SC2RAD>;
using DF071 = DataField<uint8_t , df_uint8                          >;
using DF081 = DataField<double  , df_uint16,   4                    >;
using DF082 = DataField<double  , df_uint8 , -55                    >;
using DF083 = DataField<double  , df_uint16, -43                    >;
using DF084 = DataField<double  , df_int22 , -31                    >;
using DF085 = DataField<uint16_t, df_uint10                         >;
using DF086 = DataField<double  , df_uint16,  -5                    >;
using DF087 = DataField<double  , df_uint16, -43, Conversion::SC2RAD>;
using DF088 = DataField<double  , df_uint32, -31, Conversion::SC2RAD>;
using DF089 = DataField<double  , df_uint16, -29                    >;
using DF090 = DataField<double  , df_uint32, -33                    >;
using DF091 = DataField<double  , df_uint16, -29                    >;
using DF092 = DataField<double  , df_uint32, -19                    >;
using DF093 = DataField<double  , df_uint16,   4                    >;
using DF094 = DataField<double  , df_uint16, -29                    >;
using DF095 = DataField<double  , df_uint32, -31, Conversion::SC2RAD>;
using DF096 = DataField<double  , df_uint16, -29                    >;
using DF097 = DataField<double  , df_uint32, -31, Conversion::SC2RAD>;
using DF098 = DataField<double  , df_uint16,  -5                    >;
using DF099 = DataField<double  , df_uint32, -31, Conversion::SC2RAD>;
using DF100 = DataField<double  , df_uint24, -43, Conversion::SC2RAD>;
using DF101 = DataField<double  , df_uint8 , -31                    >;
using DF102 = DataField<uint8_t , df_uint6                          >;
using DF103 = DataField<bool    , df_bit<1>                         >;
using DF137 = DataField<bool    , df_bit<1>                         >;

/*
using DF252 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF289 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF290 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF286 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF292 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF293 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF294 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF295 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF296 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF297 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF298 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF299 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF300 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF301 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF302 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF303 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF304 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF305 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF306 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF307 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF308 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF309 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF310 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF311 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF312 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF313 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF316 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF317 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF287 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
using DF288 = DataField<double, df_uint24, -43, Conversion::SC2RAD>;
*/
