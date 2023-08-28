#include "encoder.hpp"
#include <cstdio>
#include <inttypes.h>
#include "crc_24q.hpp"

void Encoder::append_bit(uint8_t bit) {
    if (mBitIndex == 0) {
        mBuffer.push_back(0);
    }

    auto index = mBuffer.size() - 1;
    mBuffer[index] |= bit << (7 - mBitIndex);
    mBitIndex = (mBitIndex + 1) % 8;
}

void Encoder::u8(size_t bits, uint8_t value) {
    assert(bits > 0 && bits <= 8);
    u64(bits, static_cast<uint64_t>(value));
}

void Encoder::u16(size_t bits, uint16_t value) {
    assert(bits > 0 && bits <= 16);
    u64(bits, static_cast<uint64_t>(value));
}

void Encoder::u32(size_t bits, uint32_t value) {
    assert(bits > 0 && bits <= 32);
    u64(bits, static_cast<uint64_t>(value));
}

void Encoder::u64(size_t bits, uint64_t value) {
    assert(bits > 0 && bits <= 64);
    for (size_t i = 0; i < bits; i++) {
        auto bit = (value >> (bits - i - 1)) & 1;
        append_bit(bit);
    }
}

void Encoder::i8(size_t bits, int8_t value) {
    assert(bits > 0 && bits <= 8);
    i64(bits, static_cast<int64_t>(value));
}

void Encoder::i16(size_t bits, int16_t value) {
    assert(bits > 0 && bits <= 16);
    i64(bits, static_cast<int64_t>(value));
}

void Encoder::i32(size_t bits, int32_t value) {
    assert(bits > 0 && bits <= 32);
    i64(bits, static_cast<int64_t>(value));
}

void Encoder::i64(size_t bits, int64_t value) {
    assert(bits > 0 && bits <= 64);
    auto unsigned_value = static_cast<uint64_t>(value);
    if (value < 0) {
        unsigned_value |= static_cast<uint64_t>(1) << (bits - 1);
    } else {
        unsigned_value &= ~(static_cast<uint64_t>(1) << (bits - 1));
    }
    u64(bits, unsigned_value);
}

void Encoder::copy(std::vector<uint8_t> buffer) {
    mBuffer.insert(mBuffer.end(), buffer.begin(), buffer.end());
    mBitIndex = 0;
}

void Encoder::checksum() {
    auto crc = crc24q_hash(mBuffer.data(), mBuffer.size());
    u32(24, crc);
}

std::vector<uint8_t> Encoder::buffer() {
    return std::move(mBuffer);
}
