#include "message.hpp"

#include "helper.hpp"

#include <cstdio>

namespace format {
namespace nmea {

Message::Message(std::string prefix, std::string payload, std::string checksum) NOEXCEPT
    : mPrefix(prefix),
      mPayload(payload),
      mChecksum(checksum) {}

Message::~Message() = default;

//
//
//

UnsupportedMessage::UnsupportedMessage(std::string prefix, std::string payload,
                                       std::string checksum) NOEXCEPT
    : Message(prefix, payload, checksum) {}

void UnsupportedMessage::print() const NOEXCEPT {
    printf("[%5s] UNSUPPORTED %s\n", sanitize_text(prefix()).c_str(),
           sanitize_text(payload()).c_str());
}

std::unique_ptr<Message> UnsupportedMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new UnsupportedMessage(*this));
}

//
//
//

ErrorMessage::ErrorMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT
    : Message(prefix, payload, checksum) {}

void ErrorMessage::print() const NOEXCEPT {
    printf("[%5s] ERROR %s\n", sanitize_text(prefix()).c_str(), sanitize_text(payload()).c_str());
}

std::unique_ptr<Message> ErrorMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new ErrorMessage(*this));
}

}  // namespace nmea
}  // namespace format
