#include "ubx_rxm_spartn.hpp"

#include <stdio.h>

#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

UbxRxmSpartn::UbxRxmSpartn(raw::RxmSpartn payload, std::vector<uint8_t>&& data) UBLOX_NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

static char const* msg_used_str(uint8_t msg_used) UBLOX_NOEXCEPT {
    switch (msg_used) {
    case 0: return "DO NOT KNOW";
    case 1: return "NOT USED";
    case 2: return "USED";
    default: return "?";
    }
}

void UbxRxmSpartn::print() const UBLOX_NOEXCEPT {
    if (mPayload.version == 1) {
        printf("[%02X %02X] UBX-RXM-SPARTN: %4u %s%s\n", message_class(), message_id(),
               mPayload.data.v1.msg_type, msg_used_str(mPayload.data.v1.flags.msg_used),
               mPayload.data.v1.flags.crc_failed ? " [CRC-FAILED]" : "");
    } else {
        printf("[%02X %02X] UBX-RXM-SPARTN:\n", message_class(), message_id());
        printf("[.....]    version: %u\n", mPayload.version);
        printf("[.....]    (no data)\n");
    }
}

std::unique_ptr<Message> UbxRxmSpartn::parse(Decoder&               decoder,
                                             std::vector<uint8_t>&& raw_data) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 2) {
        return nullptr;
    }

    auto version = decoder.U1();
    if (decoder.error()) {
        return nullptr;
    }

    raw::RxmSpartn payload;
    if (version == 1) {
        payload.version = version;

        auto flags                       = decoder.U1();
        payload.data.v1.flags.crc_failed = (flags >> 0) & 0x01;
        payload.data.v1.flags.msg_used   = (flags >> 1) & 0x03;

        payload.data.v1.sub_type  = decoder.U2();
        payload.data.v1.padding04 = decoder.U2();
        payload.data.v1.msg_type  = decoder.U2();
    } else {
        payload.version = version;
    }

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxRxmSpartn(std::move(payload), std::move(raw_data))};
    }
}

}  // namespace ublox
}  // namespace receiver
