#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace format {
namespace rtcm {

// MSB-first bit reader over a framed RTCM message
class BitReader {
public:
    BitReader(std::vector<uint8_t> const& data, size_t bit) : mData(data), mBit(bit) {}

    // Reading past the end of the data (or more than 64 bits) returns 0 and makes `ok()` false, so
    // a message that is shorter than its definition can not read out of bounds.
    uint64_t u(size_t n) {
        if (n > 64 || mBit + n > mData.size() * 8) {
            mError = true;
            mBit += n;
            return 0;
        }

        uint64_t value = 0;
        for (size_t i = 0; i < n; i++, mBit++) {
            auto byte = mData[mBit / 8];
            value     = (value << 1) | ((byte >> (7 - (mBit % 8))) & 1u);
        }
        return value;
    }

    int64_t s(size_t n) {
        auto value = u(n);
        if (value & (1ull << (n - 1))) return static_cast<int64_t>(value | (~0ull << n));
        return static_cast<int64_t>(value);
    }

    // sign-magnitude, as used by the GLONASS data fields
    int64_t sm(size_t n) {
        auto value     = u(n);
        auto magnitude = static_cast<int64_t>(value & ((1ull << (n - 1)) - 1));
        return (value & (1ull << (n - 1))) ? -magnitude : magnitude;
    }

    double u(size_t n, int exponent) { return std::ldexp(static_cast<double>(u(n)), exponent); }
    double s(size_t n, int exponent) { return std::ldexp(static_cast<double>(s(n)), exponent); }
    double sm(size_t n, int exponent) { return std::ldexp(static_cast<double>(sm(n)), exponent); }

    size_t position() const { return mBit; }
    bool   ok() const { return !mError; }

private:
    std::vector<uint8_t> const& mData;
    size_t                      mBit;
    bool                        mError{false};
};

}  // namespace rtcm
}  // namespace format
