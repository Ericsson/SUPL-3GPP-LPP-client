#include "message.hpp"

namespace format {
namespace ctrl {

Message::Message(std::string payload) NOEXCEPT : mPayload(std::move(payload)) {}

Message::~Message() NOEXCEPT = default;

}  // namespace ctrl
}  // namespace format
