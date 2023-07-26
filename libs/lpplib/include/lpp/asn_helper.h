#pragma once

#include <stdint.h>
#include "asnlib.h"

template <typename T>
inline T* asn1_allocate(size_t count = 1) {
    return reinterpret_cast<T*>(calloc(count, sizeof(T)));
}

#include <BIT_STRING.h>

class BitString : public BIT_STRING_s {
public:
    explicit BitString(size_t bits);

    void set_bit(ssize_t);
    void clear_bit(ssize_t);
    bool get_bit(ssize_t);
    void set_integer(size_t, size_t, size_t);

    int64_t as_int64();

    std::string as_string();

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

    void destroy();

private:
    void initialize(size_t bits);

    struct Index {
        size_t byte_index;
        size_t local_bit;
    };

    Index bit_index(ssize_t index);
};

static_assert(sizeof(BitString) == sizeof(BIT_STRING_s), "BitString must be the same size as BIT_STRING_s");

void supl_fill_tracking_area_code(TrackingAreaCode_t* tac, int tac_value);
void supl_fill_cell_identity(CellIdentity_t*, size_t value);
MCC* supl_create_mcc(int mcc_value);
void supl_fill_mcc(MCC* mcc, int mcc_value);
void supl_fill_mnc(MNC* mnc, int mnc_value);
ECGI* ecgi_create(long mcc, long mnc, long id);
double long_pointer2scaled_double(long* ptr, double def, double arg);
double long_pointer(long* ptr, long def);
long gnss2long(GNSS_SignalID_t gnss_id);

inline long* newLong(long value) {
    long* x = ALLOC_ZERO(long);
    *x      = value;
    return x;
}

