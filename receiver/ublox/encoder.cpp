#include "encoder.hpp"
#include <cstring>
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#else
#error "Big endian is not supported"
#endif

namespace receiver {
namespace ublox {

Encoder::Encoder(uint8_t* payload, uint32_t payload_length) UBLOX_NOEXCEPT
    : mPayload(payload),
      mPayloadLength(payload_length),
      mError(false) {}

void Encoder::X1(uint8_t value) UBLOX_NOEXCEPT {
    if (mPayloadLength < 1) {
        mError = true;
    } else {
        mPayload[0] = value;
        mPayload++;
        mPayloadLength--;
    }
}

void Encoder::X2(uint16_t value) UBLOX_NOEXCEPT {
    if (mPayloadLength < 2) {
        mError = true;
    } else {
        mPayload[0] = static_cast<uint8_t>(value & 0xFFU);
        mPayload[1] = static_cast<uint8_t>((value & 0xFF00U) >> 8U);
        mPayload += 2;
        mPayloadLength -= 2;
    }
}

void Encoder::X4(uint32_t value) UBLOX_NOEXCEPT {
    if (mPayloadLength < 4) {
        mError = true;
    } else {
        mPayload[0] = static_cast<uint8_t>(value & 0xFFU);
        mPayload[1] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
        mPayload[2] = static_cast<uint8_t>((value >> 16U) & 0xFFU);
        mPayload[3] = static_cast<uint8_t>((value >> 24U) & 0xFFU);
        mPayload += 4;
        mPayloadLength -= 4;
    }
}

void Encoder::X8(uint64_t value) UBLOX_NOEXCEPT {
    if (mPayloadLength < 8) {
        mError = true;
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

void Encoder::E1(uint8_t value) UBLOX_NOEXCEPT {
    X1(value);
}

void Encoder::E2(uint16_t value) UBLOX_NOEXCEPT {
    X2(value);
}

void Encoder::E4(uint32_t value) UBLOX_NOEXCEPT {
    X4(value);
}

void Encoder::U1(uint8_t value) UBLOX_NOEXCEPT {
    X1(value);
}

void Encoder::U2(uint16_t value) UBLOX_NOEXCEPT {
    X2(value);
}

void Encoder::U4(uint32_t value) UBLOX_NOEXCEPT {
    X4(value);
}

void Encoder::U8(uint64_t value) UBLOX_NOEXCEPT {
    X8(value);
}

void Encoder::I1(int8_t value) UBLOX_NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint8_t*>(&value);
    X1(unsigned_value);
}

void Encoder::I2(int16_t value) UBLOX_NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint16_t*>(&value);
    X2(unsigned_value);
}

void Encoder::I4(int32_t value) UBLOX_NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint32_t*>(&value);
    X4(unsigned_value);
}

void Encoder::I8(int64_t value) UBLOX_NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint64_t*>(&value);
    X8(unsigned_value);
}

void Encoder::R4(float value) UBLOX_NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint32_t*>(&value);
    X4(unsigned_value);
}

void Encoder::R8(double value) UBLOX_NOEXCEPT {
    auto unsigned_value = *reinterpret_cast<uint64_t*>(&value);
    X8(unsigned_value);
}

void Encoder::L(bool value) UBLOX_NOEXCEPT {
    X1(value ? 1 : 0);
}

void Encoder::CH(const std::string& value, uint32_t length) UBLOX_NOEXCEPT {
    auto bytes = value.size() + 1;
    if (bytes > length) {
        bytes = length;
    }

    if (mPayloadLength < bytes) {
        mError = true;
    } else {
        memcpy(mPayload, value.c_str(), bytes - 1);
        mPayload[bytes - 1] = 0;
        mPayload += bytes;
        mPayloadLength -= bytes;
    }
}

void Encoder::pad(uint32_t length) UBLOX_NOEXCEPT {
    if (mPayloadLength < length) {
        mError = true;
    } else {
        memset(mPayload, 0, length);
        mPayload += length;
        mPayloadLength -= length;
    }
}

uint32_t Encoder::remaining() const UBLOX_NOEXCEPT {
    return mPayloadLength;
}

bool Encoder::error() const UBLOX_NOEXCEPT {
    return mError;
}

uint8_t* Encoder::ptr() const UBLOX_NOEXCEPT {
    return mPayload;
}

}  // namespace ublox
}  // namespace receiver
