#include "ubx_ack_nak.hpp"
#include <stdio.h>
#include "decoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

UbxAckNak::UbxAckNak(raw::AckNak payload, std::vector<uint8_t>&& data) UBLOX_NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

void UbxAckNak::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-ACK-ACK:\n", message_class(), message_id());
    printf("[.....]    cls_id: 0x%02X\n", mPayload.cls_id);
    printf("[.....]    msg_id: 0x%02X\n", mPayload.msg_id);
}

std::unique_ptr<Message> UbxAckNak::parse(Decoder&               decoder,
                                          std::vector<uint8_t>&& raw_data) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 2) {
        return nullptr;
    }

    auto payload   = raw::AckNak{};
    payload.cls_id = decoder.U1();
    payload.msg_id = decoder.U1();

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxAckNak(std::move(payload), std::move(raw_data))};
    }
}

}  // namespace ublox
}  // namespace receiver
