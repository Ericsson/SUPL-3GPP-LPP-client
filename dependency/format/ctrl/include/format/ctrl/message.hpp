#pragma once
#include <core/core.hpp>

#include <memory>
#include <string>

namespace format {
namespace ctrl {

class Message {
public:
    EXPLICIT Message(std::string payload) NOEXCEPT;
    virtual ~Message();

    Message(Message const& other) : mPayload(other.mPayload) {}
    Message(Message&&)                 = delete;
    Message& operator=(Message const&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Get the message payload.
    NODISCARD const std::string& payload() const NOEXCEPT { return mPayload; }

    /// Print the message to stdout.
    virtual void print() const NOEXCEPT = 0;

    /// Clone the message.
    NODISCARD virtual std::unique_ptr<Message> clone() const NOEXCEPT = 0;

private:
    std::string mPayload;
};
}  // namespace ctrl
}  // namespace format

namespace streamline {
template <typename T>
struct TypeName;

template <>
struct TypeName<std::unique_ptr<format::ctrl::Message>> {
    static char const* name() { return "ctrl::Message"; }
};
}  // namespace streamline
