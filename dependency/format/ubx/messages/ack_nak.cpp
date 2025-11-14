#include "messages/ack_nak.hpp"
#include "decoder.hpp"
#include "parser.hpp"

#include <cstdio>

namespace format {
namespace ubx {

UbxAckNak::UbxAckNak(raw::AckNak payload, std::vector<uint8_t> data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

void UbxAckNak::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-ACK-ACK:\n", message_class(), message_id());
    printf("[.....]    cls_id: 0x%02X\n", mPayload.cls_id);
    printf("[.....]    msg_id: 0x%02X\n", mPayload.msg_id);
}

std::unique_ptr<Message> UbxAckNak::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UbxAckNak{*this}};
}

std::unique_ptr<Message> UbxAckNak::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    if (decoder.remaining() < 2) {
        return nullptr;
    }

    auto payload   = raw::AckNak{};
    payload.cls_id = decoder.u1();
    payload.msg_id = decoder.u1();

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxAckNak(std::move(payload), std::move(data))};
    }
}

}  // namespace ubx
}  // namespace format
