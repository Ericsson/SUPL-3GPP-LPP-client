#pragma once
#include <generator/spartn2/generator.hpp>
#include <generator/spartn2/types.hpp>

#include "builder.hpp"
#include "data.hpp"

#define SPARTN_CRC_16_CCITT 1

struct BIT_STRING_s;

struct ByteRange {
    uint8_t* ptr;
    size_t size;
};

class TransportBuilder {
public:
    SPARTN_EXPLICIT TransportBuilder();

    std::vector<uint8_t> build();
    ByteRange range(size_t begin_bit, size_t end_bit);
    size_t bit_length();

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
        // TODO(ewasjon): You should be able to memcpy this!
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
    void orbit_iode(long gnss_id, BIT_STRING_s& iode);

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

    // SF025 - GPS phase bias mask
    inline void sf025_raw(bool extended, uint32_t mask) {
        mBuilder.b(extended);
        if (extended) {
            mBuilder.bits(mask, 11);
        } else {
            mBuilder.bits(mask, 6);
        }
    }

    template <typename T>
    inline void sf025(const std::unordered_map<uint8_t, T>* types) {
        uint32_t mask = 0;
        for (auto& kvp : (*types)) {
            if (kvp.first >= 11) continue;
            mask |= 1U << kvp.first;
        }

        sf025_raw((mask & 0x7E0) != 0, mask);
    }

    // SF027 - GPS code bias mask
    inline void sf027_raw(bool extended, uint32_t mask) {
        mBuilder.b(extended);
        if (extended) {
            mBuilder.bits(mask, 11);
        } else {
            mBuilder.bits(mask, 6);
        }
    }

    // SF069 - Reserved
    inline void sf069() { mBuilder.bits(0, 1); }

private:
    uint8_t  mMessageType;
    uint8_t  mMessageSubtype;
    uint32_t mMessageTime;
    Builder  mBuilder;
};
