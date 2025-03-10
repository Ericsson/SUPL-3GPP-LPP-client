#pragma once
#include <core/core.hpp>

#include <vector>

/// Builds binary blobs of bits for the SPARTN format
class Builder {
public:
    EXPLICIT Builder(uint32_t capacity);

    inline void u8(uint8_t value) { bits(static_cast<uint64_t>(value), 8); }
    inline void u16(uint16_t value) { bits(static_cast<uint64_t>(value), 16); }
    inline void u32(uint32_t value) { bits(static_cast<uint64_t>(value), 32); }
    inline void u64(uint64_t value) { bits(static_cast<uint64_t>(value), 64); }

    inline void i8(int8_t value) { signed_bits(static_cast<int64_t>(value), 8); }
    inline void i16(int16_t value) { signed_bits(static_cast<int64_t>(value), 16); }
    inline void i32(int32_t value) { signed_bits(static_cast<int64_t>(value), 32); }
    inline void i64(int64_t value) { signed_bits(static_cast<int64_t>(value), 64); }

    inline void b(bool value) { bits(static_cast<uint64_t>(value), 1); }

    double double_to_bits(double min_range, double max_range, double resolution, double value,
                        uint8_t bits);

    // TODO: float, double
    void        reserve(uint32_t bits);
    void        bits(uint64_t value, uint8_t bits);
    void        signed_bits(int64_t value, uint8_t bits);
    void        pad(uint8_t bits);
    void        align(uint8_t bits);
    inline void align_byte() { align(8); }

    std::vector<uint8_t> data() const {
        auto data = std::move(mData);
        data.resize((mBitOffset + 7) / 8);
        return data;
    }

    uint8_t* data_ptr() { return mData.data(); }

    size_t bit_length() const { return mBitOffset; }

private:
    std::vector<uint8_t> mData;
    size_t               mBitOffset;
};
