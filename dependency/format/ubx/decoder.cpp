#include "decoder.hpp"

#include <cstring>
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#else
#error "Big endian is not supported"
#endif

namespace format {
namespace ubx {

Decoder::Decoder(uint8_t* payload, uint32_t payload_length) NOEXCEPT
    : mPayload(payload),
      mPayloadLength(payload_length),
      mError(false) {}

uint8_t Decoder::X1() NOEXCEPT {
    if (mPayloadLength < 1) {
        mError = true;
        return 0;
    } else {
        uint8_t value = mPayload[0];
        mPayload++;
        mPayloadLength--;
        return value;
    }
}

uint16_t Decoder::X2() NOEXCEPT {
    if (mPayloadLength < 2) {
        mError = true;
        return 0;
    } else {
        auto value = static_cast<uint16_t>((mPayload[1] << 8) | mPayload[0]);
        mPayload += 2;
        mPayloadLength -= 2;
        return value;
    }
}

uint32_t Decoder::X4() NOEXCEPT {
    if (mPayloadLength < 4) {
        mError = true;
        return 0;
    } else {
        auto value = (static_cast<uint32_t>(mPayload[3]) << 24) |
                     (static_cast<uint32_t>(mPayload[2]) << 16) |
                     (static_cast<uint32_t>(mPayload[1]) << 8) | static_cast<uint32_t>(mPayload[0]);
        mPayload += 4;
        mPayloadLength -= 4;
        return value;
    }
}

uint64_t Decoder::X8() NOEXCEPT {
    if (mPayloadLength < 8) {
        mError = true;
        return 0;
    } else {
        auto value = (static_cast<uint64_t>(mPayload[7]) << 56) |
                     (static_cast<uint64_t>(mPayload[6]) << 48) |
                     (static_cast<uint64_t>(mPayload[5]) << 40) |
                     (static_cast<uint64_t>(mPayload[4]) << 32) |
                     (static_cast<uint64_t>(mPayload[3]) << 24) |
                     (static_cast<uint64_t>(mPayload[2]) << 16) |
                     (static_cast<uint64_t>(mPayload[1]) << 8) | static_cast<uint64_t>(mPayload[0]);
        mPayload += 8;
        mPayloadLength -= 8;
        return value;
    }
}

uint8_t Decoder::E1() NOEXCEPT {
    return X1();
}

uint16_t Decoder::E2() NOEXCEPT {
    return X2();
}

uint32_t Decoder::E4() NOEXCEPT {
    return X4();
}

uint8_t Decoder::U1() NOEXCEPT {
    return X1();
}

uint16_t Decoder::U2() NOEXCEPT {
    return X2();
}

uint32_t Decoder::U4() NOEXCEPT {
    return X4();
}

uint64_t Decoder::U8() NOEXCEPT {
    return X8();
}

int8_t Decoder::I1() NOEXCEPT {
    auto value = X1();
    return *reinterpret_cast<int8_t*>(&value);
}

int16_t Decoder::I2() NOEXCEPT {
    auto value = X2();
    return *reinterpret_cast<int16_t*>(&value);
}

int32_t Decoder::I4() NOEXCEPT {
    auto value = X4();
    return *reinterpret_cast<int32_t*>(&value);
}

int64_t Decoder::I8() NOEXCEPT {
    auto value = X8();
    return *reinterpret_cast<int64_t*>(&value);
}

float Decoder::R4() NOEXCEPT {
    auto  value = X4();
    float result;
    static_assert(sizeof(float) == sizeof(uint32_t), "float is not 32 bits");
    std::memcpy(&result, &value, sizeof(float));
    return result;
}

double Decoder::R8() NOEXCEPT {
    auto   value = X8();
    double result;
    static_assert(sizeof(double) == sizeof(uint64_t), "double is not 64 bits");
    std::memcpy(&result, &value, sizeof(double));
    return result;
}

bool Decoder::L() NOEXCEPT {
    return X1() != 0;
}

std::string Decoder::CH(uint32_t length) NOEXCEPT {
    if (mPayloadLength < length) {
        mError = true;
        return "";
    } else {
        // NOTE: this should be null terminated according to the spec
        std::string value(reinterpret_cast<char*>(mPayload));
        mPayload += length;
        mPayloadLength -= length;
        return value;
    }
}

void Decoder::skip(uint32_t length) NOEXCEPT {
    if (mPayloadLength < length) {
        mPayload += mPayloadLength;
        mPayloadLength = 0;
    } else {
        mPayload += length;
        mPayloadLength -= length;
    }
}

uint32_t Decoder::remaining() const NOEXCEPT {
    return mPayloadLength;
}

bool Decoder::error() const NOEXCEPT {
    return mError;
}

}  // namespace ubx
}  // namespace format
