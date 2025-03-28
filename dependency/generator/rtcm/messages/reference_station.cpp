#include "reference_station.hpp"
#include "encoder.hpp"
#include "helper.hpp"

#include <stdio.h>

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

namespace generator {
namespace rtcm {
Message generate_1005(ReferenceStation const& reference_station, bool gps_indicator,
                      bool glonass_indicator, bool galileo_indicator) {
    uint16_t message_id = 1005U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, static_cast<uint16_t>(reference_station.reference_station_id));
    encoder.reserve(6);
    encoder.b(gps_indicator);
    encoder.b(glonass_indicator);
    encoder.b(galileo_indicator);
    encoder.b(!reference_station.is_physical_reference_station);
    df025(encoder, reference_station.x);
    encoder.u8(1, 0 /* single receiver oscillator indicator */);
    encoder.reserve(1);
    df026(encoder, reference_station.y);
    encoder.u8(2, 0 /* quarter cycle indicator */);
    df027(encoder, reference_station.z);
    auto length = static_cast<uint16_t>(encoder.byte_count());

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, length);
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return Message(message_id, frame_encoder.buffer());
}

Message generate_1006(ReferenceStation const& reference_station, bool gps_indicator,
                      bool glonass_indicator, bool galileo_indicator) {
    uint16_t message_id = 1006U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, static_cast<uint16_t>(reference_station.reference_station_id));
    encoder.reserve(6);
    encoder.b(gps_indicator);
    encoder.b(glonass_indicator);
    encoder.b(galileo_indicator);
    encoder.b(!reference_station.is_physical_reference_station);
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
    auto length = static_cast<uint16_t>(encoder.byte_count());

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, length);
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return Message(message_id, frame_encoder.buffer());
}

Message generate_1032(ReferenceStation const&         reference_station,
                      PhysicalReferenceStation const& physical_reference_station) {
    uint16_t message_id = 1032U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, static_cast<uint16_t>(reference_station.reference_station_id));
    encoder.u16(12, static_cast<uint16_t>(physical_reference_station.reference_station_id));
    encoder.reserve(6);

    df025(encoder, physical_reference_station.x);
    df026(encoder, physical_reference_station.y);
    df027(encoder, physical_reference_station.z);
    auto length = static_cast<uint16_t>(encoder.byte_count());

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, length);
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return Message(message_id, frame_encoder.buffer());
}
}  // namespace rtcm
}  // namespace generator
