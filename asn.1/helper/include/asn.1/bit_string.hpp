#pragma once
#include <BIT_STRING.h>
#include <asn.1/helper.hpp>
#include <string>

namespace helper {

class BitString : public BIT_STRING_s {
public:
    explicit BitString(size_t bits);

    void set_bit(ssize_t);
    void clear_bit(ssize_t);
    bool get_bit(ssize_t) const;
    void set_integer(size_t, size_t, size_t);

    int64_t     as_int64() const;
    std::string as_string() const;

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

    static const BitString* from(const BIT_STRING_s* inner) {
        auto bit_string = reinterpret_cast<const BitString*>(inner);
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
}  // namespace helper
