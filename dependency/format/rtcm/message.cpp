#include "message.hpp"

#include <stdio.h>

namespace format {
namespace rtcm {

Message::Message(DF002 type, std::vector<uint8_t> data) NOEXCEPT : mType{type}, mData{data} {}

Message::~Message() = default;

UnsupportedMessage::UnsupportedMessage(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message(type, data) {}

void UnsupportedMessage::print() const NOEXCEPT {
    printf("[RTCM%4u] UNSUPPORTED: %zu bytes", mType.value(), mData.size());
}

std::unique_ptr<Message> UnsupportedMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new UnsupportedMessage(*this));
}

/** RTCM ErrorMessage
 */

ErrorMessage::ErrorMessage(DF002 type, std::vector<uint8_t> data) NOEXCEPT
    : Message(type, std::move(data)) {}

void ErrorMessage::print() const NOEXCEPT {
    printf("[RTCM%4d] ERROR: %zu bytes", mType.value(), mData.size());
}

std::unique_ptr<Message> ErrorMessage::clone() const NOEXCEPT {
    return std::unique_ptr<Message>(new ErrorMessage(*this));
}

}  // namespace rtcm
}  // namespace format
