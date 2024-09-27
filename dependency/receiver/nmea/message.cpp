#include "message.hpp"

#include <stdio.h>

namespace receiver {
namespace nmea {

Message::Message(std::string prefix, std::string payload, std::string checksum) NMEA_NOEXCEPT
    : mPrefix(prefix),
      mPayload(payload),
      mChecksum(checksum) {}

//
//
//

UnsupportedMessage::UnsupportedMessage(std::string prefix, std::string payload,
                                       std::string checksum) NMEA_NOEXCEPT
    : Message(prefix, payload, checksum) {}

void UnsupportedMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s] UNSUPPORTED %s\n", prefix().c_str(), payload().c_str());
}

//
//
//

ErrorMessage::ErrorMessage(std::string prefix, std::string payload,
                           std::string checksum) NMEA_NOEXCEPT
    : Message(prefix, payload, checksum) {}

void ErrorMessage::print() const NMEA_NOEXCEPT {
    printf("[%5s] ERROR %s\n", prefix().c_str(), payload().c_str());
}

}  // namespace nmea
}  // namespace receiver
