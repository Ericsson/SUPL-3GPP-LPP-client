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

uint8_t Decoder::x1() NOEXCEPT {
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

uint16_t Decoder::x2() NOEXCEPT {
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

uint32_t Decoder::x4() NOEXCEPT {
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

uint64_t Decoder::x8() NOEXCEPT {
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

uint8_t Decoder::e1() NOEXCEPT {
    return x1();
}

uint16_t Decoder::e2() NOEXCEPT {
    return x2();
}

uint32_t Decoder::e4() NOEXCEPT {
    return x4();
}

uint8_t Decoder::u1() NOEXCEPT {
    return x1();
}

uint16_t Decoder::u2() NOEXCEPT {
    return x2();
}

uint32_t Decoder::u4() NOEXCEPT {
    return x4();
}

uint64_t Decoder::u8() NOEXCEPT {
    return x8();
}

int8_t Decoder::i1() NOEXCEPT {
    auto value = x1();
    return *reinterpret_cast<int8_t*>(&value);
}

int16_t Decoder::i2() NOEXCEPT {
    auto value = x2();
    return *reinterpret_cast<int16_t*>(&value);
}

int32_t Decoder::i4() NOEXCEPT {
    auto value = x4();
    return *reinterpret_cast<int32_t*>(&value);
}

int64_t Decoder::i8() NOEXCEPT {
    auto value = x8();
    return *reinterpret_cast<int64_t*>(&value);
}

float Decoder::r4() NOEXCEPT {
    auto  value = x4();
    float result;
    static_assert(sizeof(float) == sizeof(uint32_t), "float is not 32 bits");
    std::memcpy(&result, &value, sizeof(float));
    return result;
}

double Decoder::r8() NOEXCEPT {
    auto   value = x8();
    double result;
    static_assert(sizeof(double) == sizeof(uint64_t), "double is not 64 bits");
    std::memcpy(&result, &value, sizeof(double));
    return result;
}

bool Decoder::logical() NOEXCEPT {
    return x1() != 0;
}

std::string Decoder::ch(uint32_t length) NOEXCEPT {
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
