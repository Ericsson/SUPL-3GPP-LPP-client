#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-macro-identifier"
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-function"
#include <BIT_STRING.h>
#pragma GCC diagnostic pop

#include <asn.1/helper.hpp>
#include <string>

namespace helper {

#if 1

class BitString : public BIT_STRING_s {
public:
    explicit BitString(size_t bits);

    void set_bit(ssize_t);
    void clear_bit(ssize_t);
    bool get_bit(ssize_t) const;
    void set_integer(size_t, size_t, size_t);

    int64_t as_int64() const;
#if 0
    std::string as_string() const;
#endif

    static BitString* allocate(size_t bits) {
        auto data = asn1_allocate<BIT_STRING_s>();
        return allocate(bits, data);
    }

    static BitString* allocate(size_t bits, BIT_STRING_s* inner) {
        auto bit_string = reinterpret_cast<BitString*>(inner);
        bit_string->initialize(bits);
        return bit_string;
    }

    static BitString* from(BIT_STRING_s* inner) {
        auto bit_string = reinterpret_cast<BitString*>(inner);
        return bit_string;
    }

    static BitString const* from(BIT_STRING_s const* inner) {
        auto bit_string = reinterpret_cast<BitString const*>(inner);
        return bit_string;
    }

    void destroy();

private:
    void initialize(size_t bits);

    struct Index {
        size_t byte_index;
        size_t local_bit;
    };

    Index bit_index(ssize_t index) const;
};

static_assert(sizeof(BitString) == sizeof(BIT_STRING_s),
              "BitString must be the same size as BIT_STRING_s");

#endif

class BitStringBuilder {
public:
    explicit BitStringBuilder() { mBits = 0; }

    template <typename T>
    BitStringBuilder& set(T index) {
        assert(index < 64);
        mBits |= 1llu << index;
        return *this;
    }

    template <typename T>
    BitStringBuilder& clear(T index) {
        assert(index < 64);
        mBits &= ~(1llu << index);
        return *this;
    }

    template <typename T>
    BitStringBuilder& integer(T index, T bits, uint64_t value) {
        // NOTE(ewasjon): A bit string is numbered from left to right, so the
        // first bit is the most significant bit. This is the opposite of how
        // we usually number bits in C, so we need to reverse the order of the
        // bits.
        for (T i = 0; i < bits; i++) {
            auto bit = bits - i - 1;
            if (value & (1llu << bit)) {
                set(index + i);
            } else {
                clear(index + i);
            }
        }
        return *this;
    }

    BIT_STRING_s* to_bit_string(size_t bits);
    BIT_STRING_s* into_bit_string(size_t bits, BIT_STRING_s* bit_string);

private:
    uint64_t mBits;
};

class BitStringReader {
public:
    explicit BitStringReader(BIT_STRING_s* bit_string) : mBits(0), mCount(0) {
        read_all_bits(bit_string);
    }

    size_t count() const { return mCount; }

    template <typename T>
    bool get(T index) const {
        assert(index < 64);
        return (mBits & (1llu << index)) != 0;
    }

    template <typename X, typename T>
    X integer(T index, T bits) const {
        assert(bits <= 64);
        assert(bits <= static_cast<T>(sizeof(X) * 8));
        X value = 0;
        for (T i = 0; i < bits; i++) {
            auto bit       = get(index + i);
            auto bit_index = bits - i - 1;
            auto bit_value = static_cast<X>(bit ? 1 : 0);
            value |= (bit_value << bit_index);
        }

        return value;
    }

private:
    void read_all_bits(BIT_STRING_s* bit_string);

    uint64_t mBits;
    size_t   mCount;
};

}  // namespace helper
