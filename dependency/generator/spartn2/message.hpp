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
        for (auto const byte : bytes) {
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

    template <typename I, typename V>
    inline I find_closest(V min, V max, V* values, I count, V value) {
        if (count == 0) return 0;
        if (value < min) return 0;
        if (value > max) return count - 1;

        I index = 0;
        V diff  = max * 2;
        for (I i = 0; i < count; ++i) {
            V new_diff = values[i] - value;
            if (new_diff < 0) new_diff = -new_diff;
            if (new_diff < diff) {
                diff  = new_diff;
                index = i;
            }
        }
        return index;
    }

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
                        std::vector<generator::spartn::OcbSatellite> const& satellites);
    void satellite_mask(long                                                 gnss_id,
                        std::vector<generator::spartn::HpacSatellite> const& satellites);
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
    inline uint8_t sf024_raw(uint8_t ure) {
        mBuilder.bits(ure, 3);
        return ure;
    }
    inline uint8_t sf024(double ure) {
        if (ure < 0.0) return sf024_raw(0);
        double  values[] = {0.01, 0.02, 0.05, 0.1, 0.3, 1.0, 2.0};
        uint8_t index    = find_closest(0.0, 2.0, values, 7, ure);
        return sf024_raw(index + 1);
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
    inline void sfxxx_bias_mask(uint8_t low, uint8_t high, std::map<uint8_t, T> const* types) {
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
    inline void sf025(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(6, 11, types);
    }

    // SF026 - GLONASS phase bias mask
    inline void sf026_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(5, 9, extended, mask);
    }

    template <typename T>
    inline void sf026(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(5, 9, types);
    }

    // SF027 - GPS code bias mask
    inline void sf027_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf027(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(6, 11, types);
    }

    // SF028 - GLONASS code bias mask
    inline void sf028_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(5, 9, extended, mask);
    }

    template <typename T>
    inline void sf028(std::map<uint8_t, T> const* types) {
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
    inline uint8_t sf042_raw(uint8_t quality) {
        mBuilder.bits(quality, 3);
        return quality;
    }
    inline uint8_t sf042(double quality) {
        if (quality < 0.0) return sf042_raw(0);
        double  values[] = {0.01, 0.02, 0.04, 0.08, 0.16, 0.32, 0.64};
        uint8_t index    = find_closest(0.0, 0.64, values, 7, quality);
        return sf042_raw(index + 1);
    }

    // SF043 - Area average vertical hydrostatic delay
    inline double sf043(double delay) {
        return mBuilder.double_to_bits(-0.508, 0.505, 0.004, delay, 8);
    }

    // SF044 - Troposphere polynomial coefficient size indicator
    inline void sf044(uint8_t size) { mBuilder.bits(size, 1); }

    // SF045 - Small troposphere coefficient T_00
    inline double sf045(double value) {
        return mBuilder.double_to_bits(-0.252, 0.252, 0.004, value, 7);
    }

    // SF046 - Troposphere polynomial coefficient T_10/T_01
    inline double sf046(double value) {
        return mBuilder.double_to_bits(-0.063, 0.063, 0.001, value, 7);
    }

    // SF047 - Small troposphere coefficient T_11
    inline double sf047(double value) {
        return mBuilder.double_to_bits(-0.051, 0.051, 0.0002, value, 9);
    }

    // SF048 - Large troposphere coefficient T_00
    inline double sf048(double value) {
        return mBuilder.double_to_bits(-1.020, 1.020, 0.004, value, 9);
    }

    // SF049 - Large troposphere coefficient T_10/T_01
    inline double sf049(double value) {
        return mBuilder.double_to_bits(-0.255, 0.255, 0.001, value, 9);
    }

    // SF050 - Large troposphere coefficient T_11
    inline double sf050(double value) {
        return mBuilder.double_to_bits(-0.2046, 0.2046, 0.0002, value, 11);
    }

    // SF051 - Troposphere residual field size
    inline void sf051(uint8_t size) { mBuilder.bits(size, 1); }

    // SF052 - Small troposphere residual zenith delay
    inline void sf052_invalid() { mBuilder.bits(0x3F, 6); }
    inline void sf052(double value) { mBuilder.double_to_bits(-0.124, 0.124, 0.004, value, 6); }

    // SF053 - Large troposphere residual zenith delay
    inline void sf053_invalid() { mBuilder.bits(0xFF, 8); }
    inline void sf053(double value) { mBuilder.double_to_bits(-0.508, 0.508, 0.004, value, 8); }

    // SF054 - Ionosphere equation type
    inline void sf054(int type) { mBuilder.signed_bits(type, 3); }

    // SF055 - Ionosphere quality
    inline uint8_t sf055_raw(int value) {
        mBuilder.signed_bits(value, 4);
        return static_cast<uint8_t>(value);
    }
    inline uint8_t sf055_invalid() { return sf055_raw(0); }
    inline uint8_t sf055(double quality) {
        if (quality < 0.0) return sf055_raw(0);
        double  values[] = {0.03, 0.05, 0.07,  0.14,  0.28,  0.56,   1.12,  2.24,
                            4.48, 8.96, 17.92, 35.84, 71.68, 143.36, 287.52};
        uint8_t index    = find_closest(0.0, 287.52, values, 15, quality);
        return sf055_raw(index + 1);
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
    inline void   sf064_invalid() { mBuilder.bits(0xF, 4); }
    inline double sf064(double value) {
        return mBuilder.double_to_bits(-0.28, 0.28, 0.04, value, 4);
    }

    // SF065 - Medium ionosphere residual slant delay
    inline void   sf065_invalid() { mBuilder.bits(0x7F, 7); }
    inline double sf065(double value) {
        return mBuilder.double_to_bits(-2.52, 2.52, 0.04, value, 7);
    }

    // SF066 - Large ionosphere residual slant delay
    inline void   sf066_invalid() { mBuilder.bits(0x3FF, 10); }
    inline double sf066(double value) {
        return mBuilder.double_to_bits(-20.44, 20.44, 0.04, value, 10);
    }

    // SF067 - Extra-large ionosphere residual slant delay
    inline void   sf067_invalid() { mBuilder.bits(0x3FFF, 14); }
    inline double sf067(double value) {
        return mBuilder.double_to_bits(-327.64, 327.64, 0.04, value, 14);
    }

    // SF0XX - Ionosphere residual slant delay
    inline double ionosphere_residual(uint8_t size, double value) {
        switch (size) {
        case 0: return sf064(value); break;
        case 1: return sf065(value); break;
        case 2: return sf066(value); break;
        case 3: return sf067(value); break;
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
    inline void sf102(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF103 - BDS phase bias mask
    inline void sf103_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf103(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF104 - QZSS phase bias mask
    inline void sf104_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf104(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(6, 11, types);
    }

    // SF105 - Galileo code bias mask
    inline void sf105_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf105(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF106 - BDS code bias mask
    inline void sf106_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(8, 15, extended, mask);
    }

    template <typename T>
    inline void sf106(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(8, 15, types);
    }

    // SF107 - QZSS code bias mask
    inline void sf107_raw(bool extended, uint32_t mask) {
        sfxxx_bias_mask_raw(6, 11, extended, mask);
    }

    template <typename T>
    inline void sf107(std::map<uint8_t, T> const* types) {
        sfxxx_bias_mask(6, 11, types);
    }

private:
    uint8_t  mMessageType;
    uint8_t  mMessageSubtype;
    uint32_t mMessageTime;
    Builder  mBuilder;
};
