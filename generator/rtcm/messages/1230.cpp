#include "1230.hpp"
#include "encoder.hpp"

using namespace generator::rtcm;

static void df42X(Encoder& encoder, uint8_t bit, uint8_t mask, Maybe<double> value) {
    if ((bit & mask) == 0) return;
    auto integer_value = static_cast<int64_t>(value.value / 0.02);
    encoder.i16(16, static_cast<int16_t>(integer_value));
}

static void df423(Encoder& encoder, uint8_t bit, uint8_t mask, Maybe<double> value) {
    df42X(encoder, bit, mask, value);
}

static void df424(Encoder& encoder, uint8_t bit, uint8_t mask, Maybe<double> value) {
    df42X(encoder, bit, mask, value);
}

static void df425(Encoder& encoder, uint8_t bit, uint8_t mask, Maybe<double> value) {
    df42X(encoder, bit, mask, value);
}

static void df426(Encoder& encoder, uint8_t bit, uint8_t mask, Maybe<double> value) {
    df42X(encoder, bit, mask, value);
}

extern generator::rtcm::Message generate_1230(const BiasInformation& bias_information) {
    auto message_id = 1230;

    auto encoder = Encoder();
    encoder.u16(12, message_id);
    encoder.u16(12, bias_information.reference_station_id);
    encoder.b(bias_information.indicator);
    encoder.reserve(3);

    encoder.u8(4, bias_information.mask);
    df423(encoder, 0x1, bias_information.mask, bias_information.l1_ca.value);
    df424(encoder, 0x2, bias_information.mask, bias_information.l1_p.value);
    df425(encoder, 0x4, bias_information.mask, bias_information.l2_ca.value);
    df426(encoder, 0x8, bias_information.mask, bias_information.l2_p.value);

    auto frame_encoder = Encoder();
    frame_encoder.u8(8, 0xD3);
    frame_encoder.u8(6, 0);
    frame_encoder.u16(10, encoder.byte_count());
    frame_encoder.copy(encoder.buffer());
    frame_encoder.checksum();

    return generator::rtcm::Message(message_id, frame_encoder.buffer());
}
