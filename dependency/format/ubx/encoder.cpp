#include "encoder.hpp"
#include <cstring>
#include <endian.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(ubx, encoder);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(ubx, encoder)

#if __BYTE_ORDER == __LITTLE_ENDIAN
#else
#error "Big endian is not supported"
#endif

namespace format {
namespace ubx {

Encoder::Encoder(uint8_t* payload, uint32_t payload_length) NOEXCEPT
    : mPayload(payload),
      mPayloadLength(payload_length),
      mError(false) {}

void Encoder::x1(uint8_t value) NOEXCEPT {
    if (mPayloadLength < 1) {
        mError = true;
        VERBOSEF("x1 failed: insufficient space (need 1, have %u)", mPayloadLength);
    } else {
        mPayload[0] = value;
        mPayload++;
        mPayloadLength--;
    }
}

void Encoder::x2(uint16_t value) NOEXCEPT {
    if (mPayloadLength < 2) {
        mError = true;
        VERBOSEF("x2 failed: insufficient space (need 2, have %u)", mPayloadLength);
    } else {
        mPayload[0] = static_cast<uint8_t>(value & 0xFFU);
        mPayload[1] = static_cast<uint8_t>((value & 0xFF00U) >> 8U);
        mPayload += 2;
        mPayloadLength -= 2;
    }
}

void Encoder::x4(uint32_t value) NOEXCEPT {
    if (mPayloadLength < 4) {
        mError = true;
        VERBOSEF("x4 failed: insufficient space (need 4, have %u)", mPayloadLength);
    } else {
        mPayload[0] = static_cast<uint8_t>(value & 0xFFU);
        mPayload[1] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
        mPayload[2] = static_cast<uint8_t>((value >> 16U) & 0xFFU);
        mPayload[3] = static_cast<uint8_t>((value >> 24U) & 0xFFU);
        mPayload += 4;
        mPayloadLength -= 4;
    }
}

void Encoder::x8(uint64_t value) NOEXCEPT {
    if (mPayloadLength < 8) {
        mError = true;
        VERBOSEF("x8 failed: insufficient space (need 8, have %u)", mPayloadLength);
    } else {
        mPayload[0] = static_cast<uint8_t>(value & 0xFFU);
        mPayload[1] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
        mPayload[2] = static_cast<uint8_t>((value >> 16U) & 0xFFU);
        mPayload[3] = static_cast<uint8_t>((value >> 24U) & 0xFFU);
        mPayload[4] = static_cast<uint8_t>((value >> 32U) & 0xFFU);
        mPayload[5] = static_cast<uint8_t>((value >> 40U) & 0xFFU);
        mPayload[6] = static_cast<uint8_t>((value >> 48U) & 0xFFU);
        mPayload[7] = static_cast<uint8_t>((value >> 56U) & 0xFFU);
        mPayload += 8;
        mPayloadLength -= 8;
    }
}

void Encoder::e1(uint8_t value) NOEXCEPT {
    x1(value);
}

void Encoder::e2(uint16_t value) NOEXCEPT {
    x2(value);
}

void Encoder::e4(uint32_t value) NOEXCEPT {
    x4(value);
}

void Encoder::u1(uint8_t value) NOEXCEPT {
    x1(value);
}

void Encoder::u2(uint16_t value) NOEXCEPT {
    x2(value);
}

void Encoder::u4(uint32_t value) NOEXCEPT {
    x4(value);
}

void Encoder::u8(uint64_t value) NOEXCEPT {
    x8(value);
}

void Encoder::i1(int8_t value) NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint8_t*>(&value);
    x1(unsigned_value);
}

void Encoder::i2(int16_t value) NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint16_t*>(&value);
    x2(unsigned_value);
}

void Encoder::i4(int32_t value) NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint32_t*>(&value);
    x4(unsigned_value);
}

void Encoder::i8(int64_t value) NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint64_t*>(&value);
    x8(unsigned_value);
}

void Encoder::r4(float value) NOEXCEPT {
    static_assert(sizeof(float) == sizeof(uint32_t), "float is not 32 bits");
    uint32_t unsigned_value;
    std::memcpy(&unsigned_value, &value, sizeof(float));
    x4(unsigned_value);
}

void Encoder::r8(double value) NOEXCEPT {
    static_assert(sizeof(double) == sizeof(uint64_t), "double is not 64 bits");
    uint64_t unsigned_value;
    std::memcpy(&unsigned_value, &value, sizeof(double));
    x8(unsigned_value);
}

void Encoder::logical(bool value) NOEXCEPT {
    x1(value ? 1 : 0);
}

void Encoder::ch(std::string const& value, uint32_t length) NOEXCEPT {
    auto bytes = value.size() + 1;
    if (bytes > length) {
        bytes = length;
    }

    if (mPayloadLength < bytes) {
        mError = true;
        VERBOSEF("ch failed: insufficient space (need %u, have %u)", bytes, mPayloadLength);
    } else {
        memcpy(mPayload, value.c_str(), bytes - 1);
        mPayload[bytes - 1] = 0;
        mPayload += bytes;
        mPayloadLength -= bytes;
    }
}

void Encoder::pad(uint32_t length) NOEXCEPT {
    if (mPayloadLength < length) {
        mError = true;
        VERBOSEF("pad failed: insufficient space (need %u, have %u)", length, mPayloadLength);
    } else {
        memset(mPayload, 0, length);
        mPayload += length;
        mPayloadLength -= length;
    }
}

uint32_t Encoder::remaining() const NOEXCEPT {
    return mPayloadLength;
}

bool Encoder::error() const NOEXCEPT {
    return mError;
}

uint8_t* Encoder::ptr() const NOEXCEPT {
    return mPayload;
}

}  // namespace ubx
}  // namespace format
