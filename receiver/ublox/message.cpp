#include "message.hpp"
#include <stdio.h>

namespace receiver {
namespace ublox {

Message::Message(uint8_t message_class, uint8_t message_id) UBLOX_NOEXCEPT : mClass(message_class),
                                                                             mId(message_id) {}

//
//
//

UnsupportedMessage::UnsupportedMessage(uint8_t message_class, uint8_t message_id) UBLOX_NOEXCEPT
    : Message(message_class, message_id) {}

void UnsupportedMessage::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-%s-%s\n", message_class(), message_id(), "???", "???");
}

}  // namespace ublox
}  // namespace receiver
