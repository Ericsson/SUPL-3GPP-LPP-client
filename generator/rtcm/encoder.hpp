#pragma once
#include "types.hpp"
#include <vector>

class Encoder {
public:
    RTCM_EXPLICIT Encoder() : mBitIndex(0) {}

    void append_bit(uint8_t bit);

    void u8(size_t bits, uint8_t value);
    void u16(size_t bits, uint16_t value);
    void u32(size_t bits, uint32_t value);
    void u64(size_t bits, uint64_t value);

    void i8(size_t bits, int8_t value);
    void i16(size_t bits, int16_t value);
    void i32(size_t bits, int32_t value);
    void i64(size_t bits, int64_t value);

    void b(bool value) { u8(1, value ? 1 : 0); }
    void reserve(size_t bits) { u64(bits, 0); }
    void copy(std::vector<uint8_t> buffer);
    void checksum();

    std::vector<uint8_t> buffer();
    size_t byte_count() const { return mBuffer.size(); }

private:
    std::vector<uint8_t> mBuffer;
    size_t mBitIndex;
};
