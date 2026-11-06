#include "antenna_descriptor.hpp"
#include "encoder.hpp"

#include <algorithm>

namespace generator {
namespace rtcm {

// An RTCM frame carries at most 1023 payload bytes (10-bit length). MT1033 is 3 fixed bytes
// (message number, station id), the setup id and five strings each preceded by a length byte;
// each string is at most 255 bytes, so all five at maximum length would overflow the frame.
static constexpr size_t MAX_PAYLOAD_BYTES = 1023;
static constexpr size_t FIXED_BYTES       = 3 + 1 + 5;  // ids + setup id + 5 length bytes

static void encode_string(Encoder& encoder, std::string const& value, size_t length) {
    encoder.u8(8, static_cast<uint8_t>(length));
    for (size_t i = 0; i < length; i++) {
        encoder.u8(8, static_cast<uint8_t>(value[i]));
    }
}

Message generate_1033(ReferenceStation const&  reference_station,
                      AntennaDescriptor const& antenna_descriptor) {
    uint16_t message_id = 1033U;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, static_cast<uint16_t>(reference_station.reference_station_id));

    std::string const* strings[5] = {
        &antenna_descriptor.antenna_descriptor,       &antenna_descriptor.antenna_serial_number,
        &antenna_descriptor.receiver_type_descriptor, &antenna_descriptor.receiver_firmware_version,
        &antenna_descriptor.receiver_serial_number,
    };

    // Per-string cap first, then trim the longest string until the whole message fits the
    // frame. Truncation (rather than emitting an oversized, unframeable message) is the only
    // option since the message cannot be split.
    size_t lengths[5];
    size_t total = FIXED_BYTES;
    for (int i = 0; i < 5; i++) {
        lengths[i] = std::min<size_t>(strings[i]->size(), 255);
        total += lengths[i];
    }
    while (total > MAX_PAYLOAD_BYTES) {
        int longest = 0;
        for (int i = 1; i < 5; i++) {
            if (lengths[i] > lengths[longest]) longest = i;
        }
        lengths[longest]--;
        total--;
    }

    encode_string(encoder, *strings[0], lengths[0]);
    encoder.u8(8, antenna_descriptor.antenna_setup_id);
    encode_string(encoder, *strings[1], lengths[1]);
    encode_string(encoder, *strings[2], lengths[2]);
    encode_string(encoder, *strings[3], lengths[3]);
    encode_string(encoder, *strings[4], lengths[4]);

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
