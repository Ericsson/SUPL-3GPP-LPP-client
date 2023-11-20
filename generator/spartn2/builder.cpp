#include "builder.hpp"

#include <cmath>
#include <stdio.h>

Builder::Builder(uint32_t capacity) : mData(capacity), mBitOffset(0) {}

void Builder::double_to_bits(double min_range, double max_range, double resolution, double value,
                             uint8_t bits) {
    auto clamped_value  = std::max(min_range, std::min(max_range, value));
    auto scaled_value   = (clamped_value - min_range) / resolution;
    auto rounded_value  = std::lround(scaled_value);
    auto unsigned_value = static_cast<uint64_t>(rounded_value);
    this->bits(unsigned_value, bits);
}

void Builder::reserve(uint32_t bits) {
    auto new_size = (mBitOffset + bits + 7) / 8;
    if (new_size > mData.size()) {
        mData.resize(new_size);
    }
}

void Builder::bits(uint64_t value, uint8_t bits) {
    assert(bits <= 64);
    reserve(bits);

    for (uint8_t i = 0; i < bits; ++i) {
        auto value_bit_offset = bits - i - 1;
        auto value_bit        = static_cast<uint8_t>((value >> value_bit_offset) & 1);
        auto byte_offset = mBitOffset / 8;
        auto bit_offset  = 7 - (mBitOffset % 8);
        auto bit         = value_bit << bit_offset;
        mData[byte_offset] |= bit;
        mBitOffset++;
    }
}

void Builder::pad(uint8_t bits) {
    reserve(bits);
    mBitOffset += bits;
}

void Builder::align(uint8_t bits) {
    uint8_t remainder = mBitOffset % bits;
    if (remainder) {
        pad(bits - remainder);
    }
}
