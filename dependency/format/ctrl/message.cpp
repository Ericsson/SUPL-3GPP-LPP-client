#include "message.hpp"

namespace format {
namespace ctrl {

Message::Message(std::string payload) NOEXCEPT : mPayload(std::move(payload)) {}

Message::~Message() = default;

}  // namespace ctrl
}  // namespace format
