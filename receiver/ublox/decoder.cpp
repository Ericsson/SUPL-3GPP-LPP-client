#include "decoder.hpp"
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#else
#error "Big endian is not supported"
#endif

namespace receiver {
namespace ublox {

Decoder::Decoder(uint8_t* payload, uint16_t payload_length)
    : mPayload(payload), mPayloadLength(payload_length), mError(false) {}

uint8_t Decoder::X1() UBLOX_NOEXCEPT {
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

uint16_t Decoder::X2() UBLOX_NOEXCEPT {
    if (mPayloadLength < 2) {
        mError = true;
        return 0;
    } else {
        uint16_t value = (mPayload[0] << 8) | mPayload[1];
        mPayload += 2;
        mPayloadLength -= 2;
        return value;
    }
}

uint32_t Decoder::X4() UBLOX_NOEXCEPT {
    if (mPayloadLength < 4) {
        mError = true;
        return 0;
    } else {
        uint32_t value = (mPayload[0] << 24) | (mPayload[1] << 16) | (mPayload[2] << 8) | mPayload[3];
        mPayload += 4;
        mPayloadLength -= 4;
        return value;
    }
}

uint64_t Decoder::X8() UBLOX_NOEXCEPT {
    if (mPayloadLength < 8) {
        mError = true;
        return 0;
    } else {
        uint64_t value = (static_cast<uint64_t>(mPayload[0]) << 56) | (static_cast<uint64_t>(mPayload[1]) << 48) |
                    (static_cast<uint64_t>(mPayload[2]) << 40) | (static_cast<uint64_t>(mPayload[3]) << 32) |
                    (static_cast<uint64_t>(mPayload[4]) << 24) | (static_cast<uint64_t>(mPayload[5]) << 16) |
                    (static_cast<uint64_t>(mPayload[6]) << 8) | static_cast<uint64_t>(mPayload[7]);
        mPayload += 8;
        mPayloadLength -= 8;
        return value;
    }
}

uint8_t Decoder::E1() UBLOX_NOEXCEPT {
    return X1();
}

uint16_t Decoder::E2() UBLOX_NOEXCEPT {
    return X2();
}

uint32_t Decoder::E4() UBLOX_NOEXCEPT {
    return X4();
}

uint8_t Decoder::U1() UBLOX_NOEXCEPT {
    return X1();
}

uint16_t Decoder::U2() UBLOX_NOEXCEPT {
    return X2();
}

uint32_t Decoder::U4() UBLOX_NOEXCEPT {
    return X4();
}

uint64_t Decoder::U8() UBLOX_NOEXCEPT {
    return X8();
}

int8_t Decoder::I1() UBLOX_NOEXCEPT {
    auto value = X1();
    return *reinterpret_cast<int8_t*>(&value);
}

int16_t Decoder::I2() UBLOX_NOEXCEPT {
    auto value = X2();
    return *reinterpret_cast<int16_t*>(&value);
}

int32_t Decoder::I4() UBLOX_NOEXCEPT {
    auto value = X4();
    return *reinterpret_cast<int32_t*>(&value);
}

int64_t Decoder::I8() UBLOX_NOEXCEPT {
    auto value = X8();
    return *reinterpret_cast<int64_t*>(&value);
}

float Decoder::R4() UBLOX_NOEXCEPT {
    auto value = X4();
    return *reinterpret_cast<float*>(&value);
}

double Decoder::R8() UBLOX_NOEXCEPT {
    auto value = X8();
    return *reinterpret_cast<double*>(&value);
}

bool Decoder::L() UBLOX_NOEXCEPT {
    return X1() != 0;
}

void Decoder::skip(uint16_t length) UBLOX_NOEXCEPT {
    if (mPayloadLength < length) {
        mPayload += mPayloadLength;
        mPayloadLength = 0;
    } else {
        mPayload += length;
        mPayloadLength -= length;
    }
}

uint16_t Decoder::remaining() const UBLOX_NOEXCEPT {
    return mPayloadLength;
}

bool Decoder::error() const UBLOX_NOEXCEPT {
    return mError;
}

}  // namespace ublox
}  // namespace receiver
