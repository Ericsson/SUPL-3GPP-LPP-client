#pragma once
#include <generator/spartn2/generator.hpp>
#include <generator/spartn2/types.hpp>

#include "builder.hpp"
#include "data.hpp"

#include <map>
#include <stdio.h>

#define SPARTN_CRC_16_CCITT 1

struct BIT_STRING_s;
struct GNSS_SSR_STEC_Correction_r16;

struct ByteRange {
    uint8_t* ptr;
    size_t   size;
};

class TransportBuilder {
public:
    SPARTN_EXPLICIT TransportBuilder();

    std::vector<uint8_t> build();
    ByteRange            range(size_t begin_bit, size_t end_bit);
    size_t               bit_length();

    // TF001 - Preamble
    inline void tf001() { mBuilder.u8(0x73); }

    // TF002 - Message type
    inline void tf002(uint8_t type) { mBuilder.bits(type, 7); }

    // TF003 - Payload length
    inline void tf003(size_t length) { mBuilder.bits(static_cast<uint32_t>(length), 10); }

    // TF004 - Encryption and authentication flag (EAF)
    inline void tf004(bool use_encryption) { mBuilder.b(use_encryption); }

    // TF005 - Message CRC type
    inline void tf005(uint8_t crc_type) { mBuilder.bits(crc_type, 2); }

    // TF006 - Frame CRC
    inline void tf006() {
        // this assumes that tf002-tf005 has been added before running.
        auto length = mBuilder.data().size();
        assert(length >= 3);

        // 20 bits of data with 4 bits of padding.
        uint8_t bytes[3];
        bytes[0] = mBuilder.data()[length - 3];
        bytes[1] = mBuilder.data()[length - 2];
        bytes[2] = mBuilder.data()[length - 1];

        // CRC 4:
        //     polynomial = 0x09
        //     initial value = 0
        //     input reflected = true
        //     result reflected = true
        //     final XOR value = 0
        static SPARTN_CONSTEXPR const uint8_t CRC4_LOOKUP[256] = {
            0x00, 0x0B, 0x05, 0x0E, 0x0A, 0x01, 0x0F, 0x04, 0x07, 0x0C, 0x02, 0x09, 0x0D, 0x06,
            0x08, 0x03, 0x0E, 0x05, 0x0B, 0x00, 0x04, 0x0F, 0x01, 0x0A, 0x09, 0x02, 0x0C, 0x07,
            0x03, 0x08, 0x06, 0x0D, 0x0F, 0x04, 0x0A, 0x01, 0x05, 0x0E, 0x00, 0x0B, 0x08, 0x03,
            0x0D, 0x06, 0x02, 0x09, 0x07, 0x0C, 0x01, 0x0A, 0x04, 0x0F, 0x0B, 0x00, 0x0E, 0x05,
            0x06, 0x0D, 0x03, 0x08, 0x0C, 0x07, 0x09, 0x02, 0x0D, 0x06, 0x08, 0x03, 0x07, 0x0C,
            0x02, 0x09, 0x0A, 0x01, 0x0F, 0x04, 0x00, 0x0B, 0x05, 0x0E, 0x03, 0x08, 0x06, 0x0D,
            0x09, 0x02, 0x0C, 0x07, 0x04, 0x0F, 0x01, 0x0A, 0x0E, 0x05, 0x0B, 0x00, 0x02, 0x09,
            0x07, 0x0C, 0x08, 0x03, 0x0D, 0x06, 0x05, 0x0E, 0x00, 0x0B, 0x0F, 0x04, 0x0A, 0x01,
            0x0C, 0x07, 0x09, 0x02, 0x06, 0x0D, 0x03, 0x08, 0x0B, 0x00, 0x0E, 0x05, 0x01, 0x0A,
            0x04, 0x0F, 0x09, 0x02, 0x0C, 0x07, 0x03, 0x08, 0x06, 0x0D, 0x0E, 0x05, 0x0B, 0x00,
            0x04, 0x0F, 0x01, 0x0A, 0x07, 0x0C, 0x02, 0x09, 0x0D, 0x06, 0x08, 0x03, 0x00, 0x0B,
            0x05, 0x0E, 0x0A, 0x01, 0x0F, 0x04, 0x06, 0x0D, 0x03, 0x08, 0x0C, 0x07, 0x09, 0x02,
            0x01, 0x0A, 0x04, 0x0F, 0x0B, 0x00, 0x0E, 0x05, 0x08, 0x03, 0x0D, 0x06, 0x02, 0x09,
            0x07, 0x0C, 0x0F, 0x04, 0x0A, 0x01, 0x05, 0x0E, 0x00, 0x0B, 0x04, 0x0F, 0x01, 0x0A,
            0x0E, 0x05, 0x0B, 0x00, 0x03, 0x08, 0x06, 0x0D, 0x09, 0x02, 0x0C, 0x07, 0x0A, 0x01,
            0x0F, 0x04, 0x00, 0x0B, 0x05, 0x0E, 0x0D, 0x06, 0x08, 0x03, 0x07, 0x0C, 0x02, 0x09,
            0x0B, 0x00, 0x0E, 0x05, 0x01, 0x0A, 0x04, 0x0F, 0x0C, 0x07, 0x09, 0x02, 0x06, 0x0D,
            0x03, 0x08, 0x05, 0x0E, 0x00, 0x0B, 0x0F, 0x04, 0x0A, 0x01, 0x02, 0x09, 0x07, 0x0C,
            0x08, 0x03, 0x0D, 0x06,
        };

        uint8_t crc = 0;
        for (const auto byte : bytes) {
            auto index = byte ^ crc;
            crc        = CRC4_LOOKUP[index];
            crc &= 0xF;
        }

        mBuilder.bits(crc, 4);
    }

    // TF007 - Message Subtype
    inline void tf007(uint8_t type) { mBuilder.bits(type, 4); }

    // TF008 - Time tag type
    inline void tf008(bool full_time) { mBuilder.b(full_time); }

    // TF009 - GNSS time tag
    inline void tf009_16bit(uint16_t time) { mBuilder.u16(time); }
    inline void tf009_32bit(uint32_t time) { mBuilder.u32(time); }

    // TF010 - Solution ID
    inline void tf010(uint8_t solution_id) { mBuilder.bits(solution_id, 7); }

    // TF011 -
    inline void tf011(uint8_t solution_processor_id) { mBuilder.bits(solution_processor_id, 4); }

    // TF016 -
    inline void tf016(std::vector<uint8_t> payload) {
        // TODO(ewasjon): [low-priority] This is really inefficient, we should be able to memcpy
        // most of the payload data.
        for (auto byte : payload) {
            mBuilder.u8(byte);
        }
    }

    // TF018 - Message CRC
    inline void tf018_16bit(uint16_t crc) { mBuilder.u16(crc); }

private:
    Builder mBuilder;
};

class MessageBuilder {
public:
    SPARTN_EXPLICIT MessageBuilder(uint8_t message_type, uint8_t message_subtype,
                                   uint32_t message_time);

    generator::spartn::Message build();

    // SF005 - Solution issue of update (SIOU)
    inline void sf005(uint16_t siou) { mBuilder.bits(siou, 9); }

    // SF008 - Yaw present flag
    inline void sf008(bool present) { mBuilder.b(present); }

    // SF009 - Satellite reference datum
    inline void sf009(uint8_t datum) { mBuilder.bits(datum, 1); }

    // SF010 - End of OCB set
    inline void sf010(bool eos) { mBuilder.b(eos); }

    // Satellite Mask: (SF011, SF012, ...)
    void satellite_mask(long                                                gnss_id,
                        const std::vector<generator::spartn::OcbSatellite>& satellites);
    void satellite_mask(long                                                 gnss_id,
                        const std::vector<generator::spartn::HpacSatellite>& satellites);
    void satellite_mask(long gnss_id, uint64_t count, bool* bits);

    // SF013 - Do not use (DNU)
    inline void sf013(bool dnu) { mBuilder.b(dnu); }

    // SF014 - OCB present flags
    inline void sf014(bool orbit, bool clock, bool bias) {
        mBuilder.b(orbit);
        mBuilder.b(clock);
        mBuilder.b(bias);
    }

    // SF015 - Continuity Indicator
    inline void sf015(double seconds) {
        if (seconds >= 320)
            mBuilder.bits(7, 3);
        else if (seconds >= 120)
            mBuilder.bits(6, 3);
        else if (seconds >= 60)
            mBuilder.bits(5, 3);
        else if (seconds >= 30)
            mBuilder.bits(4, 3);
        else if (seconds >= 10)
            mBuilder.bits(3, 3);
        else if (seconds >= 5)
            mBuilder.bits(2, 3);
        else if (seconds >= 1)
            mBuilder.bits(1, 3);
        else
            mBuilder.bits(0, 3);
    }

    // Ephemeris Type
    void ephemeris_type(long gnss_id);

    // Issue of data ephemeris
    void orbit_iode(long gnss_id, BIT_STRING_s& iode, bool iode_shift);

    // SF020 - Satellite corrections
    inline void sf020(double value) { mBuilder.double_to_bits(-16.382, 16.382, 0.002, value, 14); }

    // SF021 - Satellite yaw
    inline void sf021(double value) { mBuilder.double_to_bits(0, 354, 6, value, 6); }

    // SF022 - IODE continuity
    inline void sf022(double seconds) {
        if (seconds >= 320)
            mBuilder.bits(7, 3);
        else if (seconds >= 120)
            mBuilder.bits(6, 3);
        else if (seconds >= 60)
            mBuilder.bits(5, 3);
        else if (seconds >= 30)
            mBuilder.bits(4, 3);
        else if (seconds >= 10)
            mBuilder.bits(3, 3);
        else if (seconds >= 5)
            mBuilder.bits(2, 3);
        else if (seconds >= 1)
            mBuilder.bits(1, 3);
        else
            mBuilder.bits(0, 3);
    }

    // SF023 - Fix flag (float=0, fixed=1)
    inline void sf023(bool fix_or_float) { mBuilder.b(fix_or_float); }

    // SF024 - User range error (URE)
    inline void sf024_raw(uint8_t ure) { mBuilder.bits(ure, 3); }
    inline void sf024(double ure) {
        if (ure <= 0.00)
            sf024_raw(0);
        else if (ure <= 0.01)
            sf024_raw(1);
        else if (ure <= 0.02)
            sf024_raw(2);
        else if (ure <= 0.05)
            sf024_raw(3);
        else if (ure <= 0.10)
            sf024_raw(4);
        else if (ure <= 0.30)
            sf024_raw(5);
        else if (ure <= 1.00)
            sf024_raw(6);
        else
            sf024_raw(7);
    }

    // SFXXX - General bias mask
    inline void sfxxx_bias_mask_raw(uint8_t low, uint8_t high, bool extended, uint32_t mask) {
        mBuilder.b(extended);
        if (extended) {
            mBuilder.bits(mask, high);
        } else {
            mBuilder.bits(mask, low);
        }
    }

    template <typename T>
    inline void sfxxx_bias_mask(uint8_t low, uint8_t high, const std::map<uint8_t, T>* types) {
        uint8_t size = 0;
        if (types->size() > low) {
            size = high;
            mBuilder.b(true);
        } else {
            size = low;
            mBuilder.b(false);
        }

        for (uint8_t i = 0; i < size; ++i) {
            mBuilder.b(types->count(i) > 0);
        }
    }

    // SF025 - GPS phase bias mask
    inline void sf025_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf025(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(6, 11, types);
    }

    // SF026 - GLONASS phase bias mask
    inline void sf026_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(5, 9, extended, mask);
    }

    template <typename T>
    inline void sf026(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(5, 9, types);
    }

    // SF027 - GPS code bias mask
    inline void sf027_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf027(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(6, 11, types);
    }

    // SF028 - GLONASS code bias mask
    inline void sf028_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(5, 9, extended, mask);
    }

    template <typename T>
    inline void sf028(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(5, 9, types);
    }

    // SF029 - Code bias correction
    inline void sf029(double value) { mBuilder.double_to_bits(-20.46, 20.46, 0.02, value, 11); }

    // SF030 - Area Count
    inline void sf030(uint8_t count) {
        assert(count >= 1);
        assert(count <= 32);
        mBuilder.bits(count - 1, 5);
    }

    // SF031 - Area ID
    inline void sf031(uint8_t id) { mBuilder.bits(id, 8); }

    // SF032 - Area reference latitude
    inline void sf032(double latitude) { mBuilder.double_to_bits(-90, 90, 0.1, latitude, 11); }

    // SF033 - Area reference longitude
    inline void sf033(double longitude) { mBuilder.double_to_bits(-180, 180, 0.1, longitude, 12); }

    // SF034 - Area latitude grid node count
    inline void sf034(uint8_t count) {
        assert(count >= 1);
        assert(count <= 8);
        mBuilder.bits(count - 1, 3);
    }

    // SF035 - Area longitude grid node count
    inline void sf035(uint8_t count) {
        assert(count >= 1);
        assert(count <= 8);
        mBuilder.bits(count - 1, 3);
    }

    // SF036 - Area latitude grid node spacing
    inline void sf036(double spacing) { mBuilder.double_to_bits(0.1, 3.2, 0.1, spacing, 5); }

    // SF037 - Area longitude grid node spacing
    inline void sf037(double spacing) { mBuilder.double_to_bits(0.1, 3.2, 0.1, spacing, 5); }

    // SF039 - Number of grid points present
    inline void sf039(uint8_t count) {
        assert(count >= 0);
        assert(count <= 127);
        mBuilder.bits(count, 7);
    }

    // SF040 - Poly/Grid block present indicator
    inline void sf040(uint8_t type) { mBuilder.bits(type, 2); }

    // SF041 - Troposphere equation type
    inline void sf041(uint8_t type) { mBuilder.bits(type, 3); }

    // SF042 - Troposphere quality
    inline void sf042_raw(uint8_t quality) { mBuilder.bits(quality, 3); }
    inline void sf042(double quality) {
        if (quality <= 0.010)
            sf042_raw(1);
        else if (quality <= 0.020)
            sf042_raw(2);
        else if (quality <= 0.040)
            sf042_raw(3);
        else if (quality <= 0.080)
            sf042_raw(4);
        else if (quality <= 0.160)
            sf042_raw(5);
        else if (quality <= 0.320)
            sf042_raw(6);
        else
            sf042_raw(7);
    }

    // SF043 - Area average vertical hydrostatic delay
    inline void sf043(double delay) { mBuilder.double_to_bits(-0.508, 0.505, 0.004, delay, 8); }

    // SF044 - Troposphere polynomial coefficient size indicator
    inline void sf044(uint8_t size) { mBuilder.bits(size, 1); }

    // SF045 - Small troposphere coefficient T_00
    inline void sf045(double value) { mBuilder.double_to_bits(-0.252, 0.252, 0.004, value, 7); }

    // SF046 - Troposphere polynomial coefficient T_10/T_01
    inline void sf046(double value) { mBuilder.double_to_bits(-0.063, 0.063, 0.001, value, 7); }

    // SF047 - Small troposphere coefficient T_11
    inline void sf047(double value) { mBuilder.double_to_bits(-0.051, 0.051, 0.0002, value, 9); }

    // SF048 - Large troposphere coefficient T_00
    inline void sf048(double value) { mBuilder.double_to_bits(-1.020, 1.020, 0.004, value, 9); }

    // SF049 - Large troposphere coefficient T_10/T_01
    inline void sf049(double value) { mBuilder.double_to_bits(-0.255, 0.255, 0.001, value, 9); }

    // SF050 - Large troposphere coefficient T_11
    inline void sf050(double value) { mBuilder.double_to_bits(-0.2046, 0.2046, 0.0002, value, 11); }

    // SF051 - Troposphere residual field size
    inline void sf051(uint8_t size) { mBuilder.bits(size, 1); }

    // SF052 - Small troposphere residual zenith delay
    inline void sf052_invalid() { mBuilder.bits(0x3F, 6); }
    inline void sf052(double value) { mBuilder.double_to_bits(-0.124, 0.124, 0.004, value, 6); }

    // SF053 - Large troposphere residual zenith delay
    inline void sf053_invalid() { mBuilder.bits(0xFF, 8); }
    inline void sf053(double value) { mBuilder.double_to_bits(-0.508, 0.508, 0.004, value, 8); }

    // SF054 - Ionosphere equation type
    inline void sf054(int type) { mBuilder.bits(type, 3); }

    // SF055 - Ionosphere quality
    inline void sf055_raw(int value) { mBuilder.bits(value, 4); }
    inline void sf055_invalid() { sf055_raw(0); }
    inline void sf055(double quality) {
        if (quality <= 0.03)
            sf055_raw(1);
        else if (quality <= 0.05)
            sf055_raw(2);
        else if (quality <= 0.07)
            sf055_raw(3);
        else if (quality <= 0.14)
            sf055_raw(4);
        else if (quality <= 0.28)
            sf055_raw(5);
        else if (quality <= 0.56)
            sf055_raw(6);
        else if (quality <= 1.12)
            sf055_raw(7);
        else if (quality <= 2.24)
            sf055_raw(8);
        else if (quality <= 4.48)
            sf055_raw(9);
        else if (quality <= 8.96)
            sf055_raw(10);
        else if (quality <= 17.92)
            sf055_raw(11);
        else if (quality <= 35.84)
            sf055_raw(12);
        else if (quality <= 71.68)
            sf055_raw(13);
        else if (quality <= 143.36)
            sf055_raw(14);
        else
            sf055_raw(15);
    }

    // SF056 - Ionosphere polynomial coefficient size indicator
    inline void sf056(bool large_coefficient) { mBuilder.b(large_coefficient); }

    // SF057 - Small ionosphere coefficient C00
    inline void sf057(double value) { mBuilder.double_to_bits(-81.88, 81.88, 0.04, value, 12); }

    // SF058 - Small ionosphere coefficient C10/C01
    inline void sf058(double value) { mBuilder.double_to_bits(-16.376, 16.376, 0.008, value, 12); }

    // SF059 - Small ionosphere coefficient C11
    inline void sf059(double value) { mBuilder.double_to_bits(-8.190, 8.190, 0.002, value, 13); }

    // SF060 - Large ionosphere coefficient C00
    inline void sf060(double value) { mBuilder.double_to_bits(-327.64, 327.64, 0.04, value, 14); }

    // SF061 - Large ionosphere coefficient C10/C01
    inline void sf061(double value) { mBuilder.double_to_bits(-65.528, 65.528, 0.008, value, 14); }

    // SF062 - Large ionosphere coefficient C11
    inline void sf062(double value) { mBuilder.double_to_bits(-32.766, 32.766, 0.002, value, 15); }

    // SF0XX - Ionosphere coefficient C00
    inline void ionosphere_coefficient_c00(bool large_coefficient, double value) {
        if (large_coefficient)
            sf060(value);
        else
            sf057(value);
    }

    // SF0XX - Ionosphere coefficient C10/C01
    inline void ionosphere_coefficient_c10_c01(bool large_coefficient, double value) {
        if (large_coefficient)
            sf061(value);
        else
            sf058(value);
    }

    // SF0XX - Ionosphere coefficient C11
    inline void ionosphere_coefficient_c11(bool large_coefficient, double value) {
        if (large_coefficient)
            sf062(value);
        else
            sf059(value);
    }

    // SF063 - Ionosphere residual field size
    inline void sf063(uint8_t size) { mBuilder.bits(size, 2); }

    // SF064 - Small ionosphere residual slant delay
    inline void sf064_invalid() { mBuilder.bits(0xF, 4); }
    inline void sf064(double value) { mBuilder.double_to_bits(-0.28, 0.28, 0.04, value, 4); }

    // SF065 - Medium ionosphere residual slant delay
    inline void sf065_invalid() { mBuilder.bits(0x7F, 7); }
    inline void sf065(double value) { mBuilder.double_to_bits(-2.52, 2.52, 0.04, value, 7); }

    // SF066 - Large ionosphere residual slant delay
    inline void sf066_invalid() { mBuilder.bits(0x3FF, 10); }
    inline void sf066(double value) { mBuilder.double_to_bits(-20.44, 20.44, 0.04, value, 10); }

    // SF067 - Extra-large ionosphere residual slant delay
    inline void sf067_invalid() { mBuilder.bits(0x3FFF, 14); }
    inline void sf067(double value) { mBuilder.double_to_bits(-327.64, 327.64, 0.04, value, 14); }

    // SF0XX - Ionosphere residual slant delay
    inline void ionosphere_residual(uint8_t size, double value) {
        switch (size) {
        case 0: sf064(value); break;
        case 1: sf065(value); break;
        case 2: sf066(value); break;
        case 3: sf067(value); break;
        default: SPARTN_UNREACHABLE();
        }
    }

    inline void ionosphere_residual_invalid(uint8_t size) {
        switch (size) {
        case 0: sf064_invalid(); break;
        case 1: sf065_invalid(); break;
        case 2: sf066_invalid(); break;
        case 3: sf067_invalid(); break;
        default: SPARTN_UNREACHABLE();
        }
    }

    // SF068 - Area Issue of Update (AIOU)
    inline void sf068(uint8_t aiou) { mBuilder.bits(aiou, 4); }

    // SF069 - Reserved
    inline void sf069() { mBuilder.bits(0, 1); }

    // SF102 - Galileo phase bias mask
    inline void sf102_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf102(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF103 - BDS phase bias mask
    inline void sf103_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf103(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF104 - QZSS phase bias mask
    inline void sf104_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf104(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(6, 11, types);
    }

    // SF105 - Galileo code bias mask
    inline void sf105_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf105(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF106 - BDS code bias mask
    inline void sf106_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf106(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF107 - QZSS code bias mask
    inline void sf107_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf107(const std::map<uint8_t, T>* types) {
        sfxxx_bias_mask(6, 11, types);
    }

private:
    uint8_t  mMessageType;
    uint8_t  mMessageSubtype;
    uint32_t mMessageTime;
    Builder  mBuilder;
};
