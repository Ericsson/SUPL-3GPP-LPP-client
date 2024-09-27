#include "bit_string.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

namespace helper {

#if 1
BitString::BitString(size_t bits) {
    buf         = nullptr;
    size        = 0;
    bits_unused = 0;
    _asn_ctx    = {};
    initialize(bits);
}

void BitString::destroy() {
    size        = 0;
    bits_unused = 0;
    _asn_ctx    = {};
    if (buf) {
        free(buf);
        buf = nullptr;
    }
}

void BitString::initialize(size_t bits) {
    assert(bits > 0);
    destroy();

    auto bytes       = (bits + 7) / 8;
    auto total_bits  = bytes * 8;
    auto unused_bits = total_bits - bits;
    size             = bytes;
    assert(unused_bits <= 7);
    bits_unused = unused_bits;
    buf         = reinterpret_cast<uint8_t*>(calloc(size, sizeof(uint8_t)));
    memset(buf, 0, size);
}

BitString::Index BitString::bit_index(ssize_t index) const {
    auto bit = static_cast<size_t>(index);
    assert(index >= 0);
    assert(bit < size * 8 - bits_unused);
    bit += bits_unused;

    auto byte       = bit / 8;
    auto byte_index = size - 1 - byte;
    auto local_bit  = (bit - byte * 8);

    assert(byte_index < size);
    assert(local_bit < 8);
    return {byte_index, local_bit};
}

void BitString::set_bit(ssize_t index) {
    auto i = bit_index(index);
    buf[i.byte_index] |= 1 << i.local_bit;
}

void BitString::clear_bit(ssize_t index) {
    auto i = bit_index(index);
    buf[i.byte_index] &= ~(1 << i.local_bit);
}

bool BitString::get_bit(ssize_t index) const {
    auto i = bit_index(index);
    return ((buf[i.byte_index] >> i.local_bit) & 1) != 0;
}

void BitString::set_integer(size_t begin, size_t length, size_t value) {
    for (size_t i = 0; i < length; i++) {
        auto index = begin + i;
        if (value & (1 << i)) set_bit(index);
    }
}

int64_t BitString::as_int64() const {
    uint64_t value = 0;
    for (size_t i = 0; i < size * 8 - bits_unused; i++) {
        auto   bit       = get_bit(i);
        size_t bit_value = bit ? 1 : 0;
        value |= (bit_value << i);
    }

    return *reinterpret_cast<int64_t*>(&value);
}

#if 0
std::string BitString::as_string() const {
    std::stringstream stream;

    stream << "BitString(";
    stream << size;
    stream << ", ";
    stream << bits_unused;
    stream << ")\n";

    for (size_t i = 0; i < size; i++) {
        stream << " [";
        for (auto j = 0; j < 8; j++) {
            if (i + 1 == size && j < bits_unused) {
                stream << '*';
                continue;
            }

            if (buf[i] & (1 << j))
                stream << '1';
            else
                stream << '0';
        }
        stream << "]";
    }

    stream << '\n';

    auto data = new char[size * 8];
    for (size_t i = 0; i < size * 8; i++) {
        data[i] = '?';
    }

    for (size_t i = 0; i < size * 8 - bits_unused; i++) {
        auto index                                   = bit_index(i);
        data[index.byte_index * 8 + index.local_bit] = "0123456789ABCDEF"[i % 16];
    }

    for (size_t i = 0; i < size; i++) {
        stream << " [";
        for (auto j = 0; j < 8; j++) {
            stream << data[i * 8 + j];
        }
        stream << "]";
    }
    stream << '\n';

    auto func = [](const void* buffer, size_t size, void* application_specific_key) {
        auto& stream = *reinterpret_cast<std::stringstream*>(application_specific_key);
        stream << std::string{reinterpret_cast<const char*>(buffer), size};
        return static_cast<int>(size);
    };

    BIT_STRING_print(&asn_DEF_BIT_STRING, this, 0, func, &stream);

    std::string buffer;
    buffer.reserve(4096);
    xer_encode(
        &asn_DEF_BIT_STRING, this, XER_F_BASIC,
        [](const void* data, size_t size, void* userdata) {
            auto buffer = static_cast<std::string*>(userdata);
            auto begin  = static_cast<const char*>(data);
            auto end    = begin + size;
            buffer->insert(buffer->end(), begin, end);
            return 0;
        },
        &buffer);

    stream << '\n';
    stream << buffer;

    delete[] data;
    return stream.str();
}
#endif
#endif

static void BIT_STRING_initialize(BIT_STRING_s* bit_string, size_t bits) {
    BIT_STRING_free(&asn_DEF_BIT_STRING, bit_string, ASFM_FREE_UNDERLYING_AND_RESET);

    auto bytes              = (bits + 7) / 8;
    bit_string->size        = bytes;
    bit_string->bits_unused = 0;
    bit_string->buf         = reinterpret_cast<uint8_t*>(calloc(bit_string->size, sizeof(uint8_t)));
}

BIT_STRING_s* BitStringBuilder::to_bit_string(size_t bits) {
    auto bit_string = asn1_allocate<BIT_STRING_s>();
    return into_bit_string(bits, bit_string);
}

BIT_STRING_s* BitStringBuilder::into_bit_string(size_t bits, BIT_STRING_s* bit_string) {
    BIT_STRING_initialize(bit_string, bits);

    assert(bits <= 64);
    for (size_t j = 0; j < 64; j++) {
        if (mBits & (1llu << j)) {
            size_t x = j / 8;
            size_t y = 7 - (j % 8);
            assert(x < bit_string->size);
            if (x < bit_string->size) {
                bit_string->buf[x] |= 1 << y;
            }
        }
    }

    return bit_string;
}

}  // namespace helper
