#include "message.hpp"

#define GNSS_ID_GPS 0
#define GNSS_ID_GLO 4
#define GNSS_ID_GAL 3
#define GNSS_ID_BDS 5
#define GNSS_ID_QZS 2

#include <BIT_STRING.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
#include <asn.1/bit_string.hpp>

static uint16_t crc16_ccitt(uint8_t* data, size_t length) {
    // CRC 16 CCITT
    //     polynomial = 0x1021U
    //     initial value = 0
    //     input reflected = false
    //     result reflected = false
    //     final XOR value = 0

    static SPARTN_CONSTEXPR const uint16_t CRC16_LOOKUP[256] = {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A,
        0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294,
        0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462,
        0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509,
        0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695,
        0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 0x48C4, 0x58E5,
        0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823, 0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948,
        0x9969, 0xA90A, 0xB92B, 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
        0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
        0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B,
        0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F,
        0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB,
        0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046,
        0x6067, 0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290,
        0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E,
        0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691,
        0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
        0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 0xCB7D,
        0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A, 0x4A75, 0x5A54, 0x6A37, 0x7A16,
        0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8,
        0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E,
        0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93,
        0x3EB2, 0x0ED1, 0x1EF0,
    };

    uint16_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        auto value = data[i];
        auto index = static_cast<uint16_t>(value) ^ (crc >> 8);
        crc        = CRC16_LOOKUP[index] ^ (crc << 8);
    }

    return crc;
}

namespace generator {
namespace spartn {
Message::Message(uint8_t message_type, uint8_t message_subtype, uint32_t message_time,
                 std::vector<uint8_t>&& payload)
    : mMessageType(message_type), mMessageSubtype(message_subtype), mMessageTime(message_time),
      mPayload(std::move(payload)) {}

std::vector<uint8_t> Message::build() {
    if(mPayload.size() > 1023) {
        return {};
    }

    TransportBuilder builder{};
    builder.tf001();
    builder.tf002(mMessageType);
    builder.tf003(mPayload.size());
    builder.tf004(false);
    builder.tf005(SPARTN_CRC_16_CCITT);
    builder.tf006();
    builder.tf007(mMessageSubtype);
    builder.tf008(true);  // Full 32-bit time
    builder.tf009_32bit(mMessageTime);
    builder.tf010(0);
    builder.tf011(0);

    builder.tf016(mPayload);

    auto tf002_to_tf016 = builder.range(8 /* skip TF001, 8 bits */, builder.bit_length() - 8);
    auto crc            = crc16_ccitt(tf002_to_tf016.ptr, tf002_to_tf016.size);
    builder.tf018_16bit(crc);

    return builder.build();
}

}  // namespace spartn
}  // namespace generator

TransportBuilder::TransportBuilder() : mBuilder(1228) {}

std::vector<uint8_t> TransportBuilder::build() {
    return mBuilder.data();
}

ByteRange TransportBuilder::range(size_t begin_bit, size_t bits) {
    assert((begin_bit % 8) == 0);
    assert((bits % 8) == 0);

    auto bytes      = bits / 8;
    auto first_byte = begin_bit / 8;
    auto ptr        = mBuilder.data_ptr() + first_byte;
    return ByteRange{ptr, bytes};
}

size_t TransportBuilder::bit_length() {
    return mBuilder.bit_length();
}

MessageBuilder::MessageBuilder(uint8_t message_type, uint8_t message_subtype, uint32_t message_time)
    : mMessageType(message_type), mMessageSubtype(message_subtype), mMessageTime(message_time),
      mBuilder(1024) {}

generator::spartn::Message MessageBuilder::build() {
    auto data = mBuilder.data();
    return generator::spartn::Message{mMessageType, mMessageSubtype, mMessageTime, std::move(data)};
}

void MessageBuilder::satellite_mask(long gnss_id, uint64_t count, bool* bits) {
    switch (gnss_id) {
    case GNSS_ID_GPS:  // SF011 - GPS Satellite Mask
        if (count > 56) {
            mBuilder.bits(3, 2);
            count = 64;
        } else if (count > 44) {
            mBuilder.bits(2, 2);
            count = 56;
        } else if (count > 32) {
            mBuilder.bits(1, 2);
            count = 44;
        } else {
            mBuilder.bits(0, 2);
            count = 32;
        }
        break;
    case GNSS_ID_GLO:  // SF012 - GLONASS Satellite Mask
        if (count > 48) {
            mBuilder.bits(3, 2);
            count = 63;
        } else if (count > 36) {
            mBuilder.bits(2, 2);
            count = 48;
        } else if (count > 24) {
            mBuilder.bits(1, 2);
            count = 36;
        } else {
            mBuilder.bits(0, 2);
            count = 24;
        }
        break;
    case GNSS_ID_GAL:  // SF093 - Galileo Satellite Mask
        if (count > 54) {
            mBuilder.bits(3, 2);
            count = 64;
        } else if (count > 45) {
            mBuilder.bits(2, 2);
            count = 54;
        } else if (count > 36) {
            mBuilder.bits(1, 2);
            count = 45;
        } else {
            mBuilder.bits(0, 2);
            count = 36;
        }
        break;
    default: SPARTN_UNREACHABLE();
    }

    for (uint8_t i = 0; i < count; i++) {
        mBuilder.b(bits[i]);
    }
}

void MessageBuilder::satellite_mask(
    long gnss_id, const std::vector<generator::spartn::OcbSatellite>& satellites) {
    uint64_t count    = 0;
    bool     bits[64] = {false};
    for (auto& satellite : satellites) {
        auto prn = satellite.prn();
        // NOTE(ewasjon): 0th bit is used for PRN 1
        auto bit  = prn - 1;
        bits[bit] = true;
        count++;
    }

    satellite_mask(gnss_id, count, bits);
}

void MessageBuilder::satellite_mask(
    long gnss_id, const std::vector<generator::spartn::HpacSatellite>& satellites) {
    uint64_t count    = 0;
    bool     bits[64] = {false};
    for (auto& satellite : satellites) {
        auto prn = satellite.prn();
        // NOTE(ewasjon): 0th bit is used for PRN 1
        auto bit  = prn - 1;
        bits[bit] = true;
        count++;
    }

    satellite_mask(gnss_id, count, bits);
}

void MessageBuilder::ephemeris_type(long gnss_id) {
    switch (gnss_id) {
    case GNSS_ID_GPS:         // SF016 - GPS Ephemeris Type
        mBuilder.bits(0, 2);  // GPS L1C/A
        break;
    case GNSS_ID_GLO:         // SF017 - GLONASS Ephemeris Type
        mBuilder.bits(0, 2);  // GLONASS L1C/A
        break;
    case GNSS_ID_GAL:         // SF096 - Galileo Ephemeris Type
        mBuilder.bits(1, 3);  // Galileo I/NAV
        break;
    case GNSS_ID_BDS:         // SF097 - BeiDou Ephemeris Type
        mBuilder.bits(0, 4);  // BeiDou D1
        break;
    case GNSS_ID_QZS:         // SF098 - QZSS Ephemeris Type
        mBuilder.bits(0, 3);  // QZSS LNAV (L1C/A)
        break;
    default: SPARTN_UNREACHABLE();
    }
}

void MessageBuilder::orbit_iode(long gnss_id, BIT_STRING_s& bit_string, bool iode_shift) {
    auto iode = helper::BitString::from(&bit_string)->as_int64();
    if (iode_shift) {
        iode >>= 3;
    }

    switch (gnss_id) {
    case GNSS_ID_GPS:  // SF018 - GPS IODE
        mBuilder.bits(iode & 0xFF, 8);
        break;
    case GNSS_ID_GLO:  // SF019 - GLONASS IODE
        mBuilder.bits(iode & 0x7F, 7);
        break;
    case GNSS_ID_GAL:  // SF099 - Galileo IOD
        mBuilder.bits(iode & 0x3FF, 10);
        break;
    default: SPARTN_UNREACHABLE();
    }
}
