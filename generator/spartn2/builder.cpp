#include "builder.hpp"

#include <cmath>

Builder::Builder(uint32_t capacity) : mData(capacity), mBitOffset(0) {}

void Builder::double_to_bits(double min_range, double max_range, double resolution, double value,
                             uint8_t bits) {
    auto clamped_value  = std::max(min_range, std::min(max_range, value));
    auto rounded_value  = std::lround((clamped_value - min_range) / resolution);
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
        uint8_t bit = (value >> (bits - i - 1)) & 1;
        mData[mBitOffset / 8] |= bit << (7 - (mBitOffset % 8));
        ++mBitOffset;
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
