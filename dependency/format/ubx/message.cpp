#include "message.hpp"

#include <stdio.h>

namespace format {
namespace ubx {

Message::Message(uint8_t message_class, uint8_t message_id, std::vector<uint8_t> data) NOEXCEPT
    : mClass(message_class),
      mId(message_id),
      mData(std::move(data)) {}

Message::~Message() = default;

UnsupportedMessage::UnsupportedMessage(uint8_t message_class, uint8_t message_id,
                                       std::vector<uint8_t> data) NOEXCEPT
    : Message(message_class, message_id, std::move(data)) {}

void UnsupportedMessage::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-%s-%s\n", message_class(), message_id(), "???", "???");
}

std::unique_ptr<Message> UnsupportedMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UnsupportedMessage{*this}};
}

}  // namespace ubx
}  // namespace format
