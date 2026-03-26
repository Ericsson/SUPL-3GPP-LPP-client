#include "message.hpp"

#include <external_warnings.hpp>

EXTERNAL_WARNINGS_PUSH
#include <BIT_STRING.h>
#include <GNSS-SSR-STEC-Correction-r16.h>
EXTERNAL_WARNINGS_POP

#include <asn.1/bit_string.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(spartn, message);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(spartn, message)

#define GNSS_ID_GPS 0
#define GNSS_ID_GLO 4
#define GNSS_ID_GAL 3
#define GNSS_ID_BDS 5
#define GNSS_ID_QZS 2

static uint8_t crc8(uint8_t* data, size_t length) {
    // CRC 8
    //     polynomial = 0x07
    //     initial value = 0
    //     input reflected = false
    //     result reflected = false
    //     final XOR value = 0
    static CONSTEXPR const uint8_t CRC8_LOOKUP[256] = {
        0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A,
        0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53,
        0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4,
        0xC3, 0xCA, 0xCD, 0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
        0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1,
        0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88,
        0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F,
        0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
        0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B,
        0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2,
        0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75,
        0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
        0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34, 0x4E, 0x49, 0x40,
        0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39,
        0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE,
        0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
        0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4,
        0xF3,
    };
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc = CRC8_LOOKUP[crc ^ data[i]];
    }
    return crc;
}

static uint32_t crc24q(uint8_t* data, size_t length) {
    // CRC 24Q
    //     polynomial = 0x1864CFB
    //     initial value = 0
    //     input reflected = false
    //     result reflected = false
    //     final XOR value = 0
    uint32_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= static_cast<uint32_t>(data[i]) << 16;
        for (int j = 0; j < 8; j++) {
            crc <<= 1;
            if (crc & 0x1000000) crc ^= 0x1864CFB;
        }
    }
    return crc & 0xFFFFFF;
}
static uint16_t crc16_ccitt(uint8_t* data, size_t length) {
    // CRC 16 CCITT
    //     polynomial = 0x1021U
    //     initial value = 0
    //     input reflected = false
    //     result reflected = false
    //     final XOR value = 0

    static CONSTEXPR const uint16_t CRC16_LOOKUP[256] = {
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
        crc        = CRC16_LOOKUP[index] ^ static_cast<uint16_t>(crc << 8);
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
    // TF007-TF011: 4 + 1 + 32 + 7 + 4 = 48 bits = 6 bytes
    CONSTEXPR const size_t PAYLOAD_DESCRIPTION_SIZE = 6;

    auto payload_length = mPayload.size();
    if (payload_length > 1023) {
        return {};
    }

    TransportBuilder builder{};
    builder.tf001();
    builder.tf002(mMessageType);
    builder.tf003(payload_length);
    builder.tf004(false);
    builder.tf005(static_cast<uint8_t>(mCrcType));
    builder.tf006();
    builder.tf007(mMessageSubtype);
    builder.tf008(true);
    builder.tf009_32bit(mMessageTime);
    builder.tf010(mSolutionId);
    builder.tf011(mSolutionProcessorId);

    builder.tf016(mPayload);

    auto tf002_to_tf016 = builder.range(8, builder.bit_length() - 8);
    switch (mCrcType) {
    case CrcType::CRC8: builder.tf018_8bit(crc8(tf002_to_tf016.ptr, tf002_to_tf016.size)); break;
    case CrcType::CRC24Q:
        builder.tf018_24bit(crc24q(tf002_to_tf016.ptr, tf002_to_tf016.size));
        break;
    default: builder.tf018_16bit(crc16_ccitt(tf002_to_tf016.ptr, tf002_to_tf016.size)); break;
    }

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
    case GNSS_ID_BDS:  // SF094 - BeiDou Satellite Mask
        if (count > 55) {
            mBuilder.bits(3, 2);
            count = 64;
        } else if (count > 46) {
            mBuilder.bits(2, 2);
            count = 55;
        } else if (count > 37) {
            mBuilder.bits(1, 2);
            count = 46;
        } else {
            mBuilder.bits(0, 2);
            count = 37;
        }
        break;
    default: UNREACHABLE();
    }

    for (uint8_t i = 0; i < count; i++) {
        mBuilder.b(bits[i]);
    }
}

void MessageBuilder::satellite_mask(
    long gnss_id, std::vector<generator::spartn::OcbSatellite> const& satellites) {
    uint64_t highest_prn = 0;
    bool     bits[64]    = {false};
    for (auto& satellite : satellites) {
        auto prn = satellite.prn();
        // NOTE(ewasjon): 0th bit is used for PRN 1
        auto bit  = prn - 1;
        bits[bit] = true;
        if (prn > highest_prn) highest_prn = prn;
    }

    satellite_mask(gnss_id, highest_prn, bits);
}

void MessageBuilder::satellite_mask(
    long gnss_id, std::vector<generator::spartn::HpacSatellite> const& satellites) {
    uint64_t highest_prn = 0;
    bool     bits[64]    = {false};
    for (auto& satellite : satellites) {
        auto prn = satellite.prn();
        // NOTE(ewasjon): 0th bit is used for PRN 1
        auto bit  = prn - 1;
        bits[bit] = true;
        if (prn > highest_prn) highest_prn = prn;
    }

    satellite_mask(gnss_id, highest_prn, bits);
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
    default: UNREACHABLE();
    }
}

void MessageBuilder::orbit_iode(long gnss_id, BIT_STRING_s& bit_string) {
    auto iode = helper::BitString::from(&bit_string)->as_int64();

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
    case GNSS_ID_BDS:  // SF100 - BeiDou IODE/IODC
        mBuilder.bits(iode & 0xFF, 8);
        break;
    default: UNREACHABLE();
    }
}
