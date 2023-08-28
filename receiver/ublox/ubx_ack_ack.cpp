#include "ubx_ack_ack.hpp"
#include <stdio.h>
#include "decoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

UbxAckAck::UbxAckAck(raw::AckAck payload) UBLOX_NOEXCEPT : Message(CLASS_ID, MESSAGE_ID),
                                                           mPayload(std::move(payload)) {}

void UbxAckAck::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-ACK-ACK:\n", message_class(), message_id());
    printf("[.....]    cls_id: 0x%02X\n", mPayload.cls_id);
    printf("[.....]    msg_id: 0x%02X\n", mPayload.msg_id);
}

std::unique_ptr<Message> UbxAckAck::parse(Decoder& decoder) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 2) {
        return nullptr;
    }

    auto payload   = raw::AckAck{};
    payload.cls_id = decoder.U1();
    payload.msg_id = decoder.U1();

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxAckAck(std::move(payload))};
    }
}

}  // namespace ublox
}  // namespace receiver
