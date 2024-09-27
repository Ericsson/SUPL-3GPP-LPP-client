#pragma once
#include <core/core.hpp>

#include <stdio.h>
#include <vector>

namespace format {
namespace nav {

struct Words {
    uint8_t  mData[64];
    uint32_t mBitCount;

    Words() NOEXCEPT : mData{}, mBitCount(0) {}
    Words(uint32_t bit_count) NOEXCEPT : mData{}, mBitCount(bit_count) {}

    NODISCARD uint32_t size() const NOEXCEPT { return mBitCount; }

    NODISCARD uint64_t u64(uint32_t index, uint32_t length) const NOEXCEPT {
        uint64_t value = 0;
        for (auto i = index; i < index + length; i++) {
            if (i >= mBitCount) {
                break;
            }
            auto byte = i / 8;
            auto bit  = 7 - i % 8;
            auto mask = 1 << bit;
            value <<= 1;
            value |= (mData[byte] & mask) ? 1 : 0;
        }
        return value;
    }

    NODISCARD uint32_t u32(uint32_t index, uint32_t length) const NOEXCEPT {
        return static_cast<uint32_t>(u64(index, length));
    }

    NODISCARD uint16_t u16(uint32_t index, uint32_t length) const NOEXCEPT {
        return static_cast<uint16_t>(u64(index, length));
    }

    NODISCARD uint8_t u8(uint32_t index, uint32_t length) const NOEXCEPT {
        return static_cast<uint8_t>(u64(index, length));
    }

    NODISCARD bool b1(uint32_t index) const NOEXCEPT { return get_bit(index); }

    bool get_bit(uint32_t index) const NOEXCEPT {
        if (index >= mBitCount) {
            return false;
        }
        auto byte = index / 8;
        auto bit  = 7 - index % 8;
        auto mask = 1 << bit;
        return mData[byte] & mask;
    }

    void set_bit(uint32_t index, bool value) NOEXCEPT {
        if (index >= sizeof(mData) * 8) {
            return;
        }
        auto byte = index / 8;
        auto bit  = 7 - index % 8;
        auto mask = 1 << bit;
        if (value) {
            mData[byte] |= mask;
        } else {
            mData[byte] &= ~mask;
        }
    }

    void set(uint32_t index, uint32_t length, uint32_t value) NOEXCEPT {
        for (uint32_t i = 0; i < length; i++) {
            set_bit(index + i, value & (1 << (length - i - 1)));
        }
    }

    NODISCARD static Words from_sfrbx_l1ca(std::vector<uint32_t> const& words) NOEXCEPT {
        Words result{300};

        uint32_t index = 0;
        for (auto word : words) {
            auto data = word & 0x3FFFFFFF;
            result.set(index, 30, data);
            index += 30;
        }
        return result;
    }

    NODISCARD static Words from_sfrbx_e5b(std::vector<uint32_t> const& words) NOEXCEPT {
        Words result{256};

        uint32_t index = 0;
        for (auto word : words) {
            result.set(index, 32, word);
            index += 32;
        }
        return result;
    }

    NODISCARD static Words from_sfrbx_bds_d1(std::vector<uint32_t> const& words) NOEXCEPT {
        Words result{300};

        uint32_t index = 0;
        for (auto word : words) {
            auto data = word & 0x3FFFFFFF;
            result.set(index, 30, data);
            index += 30;
        }
        return result;
    }

    void print(uint32_t index, uint32_t length) const NOEXCEPT {
        for (uint32_t i = 0; i < length; i++) {
            printf("%d", get_bit(index + i));
        }
    }

    void print() const NOEXCEPT {
        print(0, 24);
        printf("|");
        print(30, 24);
        printf("|");
        print(60, 10);
        printf(" ");
        print(70, 2);
        printf(" ");
        print(72, 4);
        printf(" ");
        print(76, 6);
        printf(" ");
        print(82, 2);
        printf("|");
        print(90, 1);
        printf("|");
        printf("|");
        printf("|");
        print(180, 16);
        printf(" ");
        print(196, 8);
        printf("|");
        print(210, 8);
        printf(" ");
        print(218, 16);
        printf("|");
        print(240, 8);
        printf(" ");
        print(248, 16);
        printf("|");
        print(270, 22);
        printf("\n");
    }
};

}  // namespace nav
}  // namespace format
