#include "asn_helper.h"

#include <assert.h>
#include <iostream>
#include <sstream>

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
    for (int j = 0; j < 64; j++) {
        if (mBits & (1llu << j)) {
            auto x = j / 8;
            auto y = 7 - (j % 8);
            assert(x < bit_string->size);
            if (x < bit_string->size) {
                bit_string->buf[x] |= 1 << y;
            }
        }
    }

    return bit_string;
}

void supl_fill_tracking_area_code(TrackingAreaCode_t* tac, int tac_value) {
    BitStringBuilder{}.integer(0, 16, tac_value).into_bit_string(16, tac);
}

void supl_fill_cell_identity(CellIdentity_t* identity, unsigned long long value) {
    BitStringBuilder{}.integer(0, 28, value).into_bit_string(28, identity);
}

MCC* supl_create_mcc(int mcc_value) {
    if (mcc_value < 0 || mcc_value > 999) {
        return NULL;
    }

    MCC_MNC_Digit* d;
    MCC*           mcc;
    char           tmp[8];
    uint32_t       i = 0;

    mcc = ALLOC_ZERO(MCC);
    sprintf(tmp, "%d", mcc_value);
    for (i = 0; i < strlen(tmp); i++) {
        d  = ALLOC_ZERO(MCC_MNC_Digit_t);
        *d = tmp[i] - '0';
        ASN_SEQUENCE_ADD(mcc, d);
    }

    return mcc;
}

void supl_fill_mcc(MCC* mcc, int mcc_value) {
    if (mcc_value < 0 || mcc_value > 999) {
        return;
    }

    MCC_MNC_Digit* d;
    char           tmp[8];
    uint32_t       i = 0;

    sprintf(tmp, "%d", mcc_value);
    for (i = 0; i < strlen(tmp); i++) {
        d  = ALLOC_ZERO(MCC_MNC_Digit);
        *d = tmp[i] - '0';
        ASN_SEQUENCE_ADD(mcc, d);
    }
}

void supl_fill_mnc(MNC* mnc, int mnc_value) {
    if (mnc_value < 0 || mnc_value > 999) {
        return;
    }

    MCC_MNC_Digit* d;
    char           tmp[8];
    uint32_t       i = 0;

    sprintf(tmp, "%02d", mnc_value);
    for (i = 0; i < strlen(tmp); i++) {
        d  = ALLOC_ZERO(MCC_MNC_Digit);
        *d = tmp[i] - '0';
        ASN_SEQUENCE_ADD(mnc, d);
    }
}

ECGI* ecgi_create(long mcc, long mnc, unsigned long long id) {
    ECGI* primary_cell = ALLOC_ZERO(ECGI);

    supl_fill_mcc((MCC*)&primary_cell->mcc, mcc);
    supl_fill_mnc((MNC*)&primary_cell->mnc, mnc);

    supl_fill_cell_identity(&primary_cell->cellidentity, id);
    return primary_cell;
}

NCGI_r15* ncgi_create(long mcc, long mnc, unsigned long long id) {
    NCGI_r15* primary_cell = ALLOC_ZERO(NCGI_r15);

    supl_fill_mcc((MCC*)&primary_cell->mcc_r15, mcc);
    supl_fill_mnc((MNC*)&primary_cell->mnc_r15, mnc);

    BitStringBuilder{}
        .integer(0, 36, id)
        .into_bit_string(36, &primary_cell->nr_cellidentity_r15);
    return primary_cell;
}

double long_pointer2scaled_double(long* ptr, double def, double arg) {
    if (!ptr) return def;
    return *ptr / arg;
}

double long_pointer(long* ptr, long def) {
    if (!ptr) return def;
    return *ptr;
}

long gnss2long(GNSS_SignalID_t gnss_id) {
    if (gnss_id.ext1) {
        return *gnss_id.ext1->gnss_SignalID_Ext_r15;
    } else {
        return gnss_id.gnss_SignalID;
    }
}
