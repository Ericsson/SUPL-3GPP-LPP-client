#include "message.hpp"
#include <stdio.h>

namespace receiver {
namespace ublox {

Message::Message(uint8_t message_class, uint8_t message_id,
                 std::vector<uint8_t>&& data) UBLOX_NOEXCEPT : mClass(message_class),
                                                               mId(message_id),
                                                               mData(std::move(data)) {}

//
//
//

UnsupportedMessage::UnsupportedMessage(uint8_t message_class, uint8_t message_id,
                                       std::vector<uint8_t>&& data) UBLOX_NOEXCEPT
    : Message(message_class, message_id, std::move(data)) {}

void UnsupportedMessage::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-%s-%s: %zu bytes\n", message_class(), message_id(), "???", "???",
           data().size());
}

}  // namespace ublox
}  // namespace receiver
