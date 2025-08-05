#include "message.hpp"

#include <stdio.h>

namespace format {
namespace rtcm {

Message::Message(std::vector<uint8_t> mData) NOEXCEPT : mData{mData} {}

Message::~Message() NOEXCEPT = default;

UnsupportedMessage::UnsupportedMessage(unsigned type, std::vector<uint8_t> mData) NOEXCEPT
    : Message(mData), type{type} {}

void UnsupportedMessage::print() const NOEXCEPT {
    printf("[RTCM%x] UNSUPPORTED MESSAGE", type);
}

std::unique_ptr<Message> UnsupportedMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new UnsupportedMessage(*this));
}

/** RTCM ErrorMessage
 */

ErrorMessage::ErrorMessage() NOEXCEPT : Message({}) {
}

void ErrorMessage::print() const NOEXCEPT {
    printf("[RTCM] ERROR");
}

std::unique_ptr<Message> ErrorMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new ErrorMessage(*this));
}

}  // namespace rtcm
}  // namespace format
