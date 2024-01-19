#include "message.hpp"

#include <stdio.h>

namespace receiver {
namespace nmea {

Message::Message(std::string prefix) NMEA_NOEXCEPT : mPrefix(prefix) {}

//
//
//

UnsupportedMessage::UnsupportedMessage(std::string prefix, std::string payload) NMEA_NOEXCEPT
    : Message(prefix),
      mPayload(payload) {}

void UnsupportedMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s] UNSUPPORTED %s\n", prefix().c_str(), payload().c_str());
}

//
//
//

ErrorMessage::ErrorMessage(std::string prefix, std::string payload) NMEA_NOEXCEPT
    : Message(prefix),
      mPayload(payload) {}

void ErrorMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s] ERROR %s\n", prefix().c_str(), payload().c_str());
}

}  // namespace nmea
}  // namespace receiver
