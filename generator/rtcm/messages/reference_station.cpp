#include "reference_station.hpp"
#include "encoder.hpp"
#include "helper.hpp"

using namespace generator::rtcm;

static void df02X(Encoder& encoder, double value) {
    auto integer_value = static_cast<int64_t>(ROUND(value / 0.0001));
    encoder.i64(38, integer_value);
}

static void df025(Encoder& encoder, double value) {
    df02X(encoder, value);
}

static void df026(Encoder& encoder, double value) {
    df02X(encoder, value);
}

static void df027(Encoder& encoder, double value) {
    df02X(encoder, value);
}

static void df028(Encoder& encoder, double value) {
    auto integer_value = static_cast<int64_t>(ROUND(value / 0.0001));
    encoder.u16(16, static_cast<uint16_t>(integer_value));
}

extern generator::rtcm::Message generate_1005(const ReferenceStation& reference_station,
                                              bool gps_indicator, bool glonass_indicator,
                                              bool galileo_indicator) {
    auto message_id = 1005U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, reference_station.reference_station_id);
    encoder.reserve(6);
    encoder.b(gps_indicator);
    encoder.b(glonass_indicator);
    encoder.b(galileo_indicator);
    encoder.b(reference_station.is_physical_reference_station);
    df025(encoder, reference_station.x);
    encoder.u8(1, 0 /* single receiver oscillator indicator */);
    encoder.reserve(1);
    df026(encoder, reference_station.y);
    encoder.u8(2, 0 /* quarter cycle indicator */);
    df027(encoder, reference_station.z);

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}

extern generator::rtcm::Message generate_1006(const ReferenceStation& reference_station,
                                              bool gps_indicator, bool glonass_indicator,
                                              bool galileo_indicator) {
    auto message_id = 1006U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, reference_station.reference_station_id);
    encoder.reserve(6);
    encoder.b(gps_indicator);
    encoder.b(glonass_indicator);
    encoder.b(galileo_indicator);
    encoder.b(reference_station.is_physical_reference_station);
    df025(encoder, reference_station.x);
    encoder.u8(1, 0 /* single receiver oscillator indicator */);
    encoder.reserve(1);
    df026(encoder, reference_station.y);
    encoder.u8(2, 0 /* quarter cycle indicator */);
    df027(encoder, reference_station.z);
    if (reference_station.antenna_height.valid) {
        df028(encoder, reference_station.antenna_height.value);
    } else {
        df028(encoder, 0);
    }

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}

extern generator::rtcm::Message
generate_1032(const ReferenceStation&         reference_station,
              const PhysicalReferenceStation& physical_reference_station) {
    auto message_id = 1032U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, reference_station.reference_station_id);
    encoder.u16(12, physical_reference_station.reference_station_id);
    encoder.reserve(6);

    df025(encoder, physical_reference_station.x);
    df026(encoder, physical_reference_station.y);
    df027(encoder, physical_reference_station.z);

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}
