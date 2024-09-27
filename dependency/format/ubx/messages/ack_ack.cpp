#include "messages/ack_ack.hpp"
#include "decoder.hpp"
#include "parser.hpp"

#include <stdio.h>

namespace format {
namespace ubx {

UbxAckAck::UbxAckAck(raw::AckAck payload, std::vector<uint8_t> data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

void UbxAckAck::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-ACK-ACK:\n", message_class(), message_id());
    printf("[.....]    cls_id: 0x%02X\n", mPayload.cls_id);
    printf("[.....]    msg_id: 0x%02X\n", mPayload.msg_id);
}

std::unique_ptr<Message> UbxAckAck::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UbxAckAck{*this}};
}

std::unique_ptr<Message> UbxAckAck::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    if (decoder.remaining() < 2) {
        return nullptr;
    }

    auto payload   = raw::AckAck{};
    payload.cls_id = decoder.U1();
    payload.msg_id = decoder.U1();

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxAckAck(std::move(payload), std::move(data))};
    }
}

}  // namespace ubx
}  // namespace format
