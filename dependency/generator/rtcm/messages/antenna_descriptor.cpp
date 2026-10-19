#include "antenna_descriptor.hpp"
#include "encoder.hpp"

#include <algorithm>

namespace generator {
namespace rtcm {

static void encode_string(Encoder& encoder, std::string const& value) {
    auto length = static_cast<uint8_t>(std::min<size_t>(value.size(), 255));
    encoder.u8(8, length);
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

    encode_string(encoder, antenna_descriptor.antenna_descriptor);
    encoder.u8(8, antenna_descriptor.antenna_setup_id);
    encode_string(encoder, antenna_descriptor.antenna_serial_number);
    encode_string(encoder, antenna_descriptor.receiver_type_descriptor);
    encode_string(encoder, antenna_descriptor.receiver_firmware_version);
    encode_string(encoder, antenna_descriptor.receiver_serial_number);

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
