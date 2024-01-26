#pragma once
#include <receiver/ublox/types.hpp>

namespace receiver {
namespace ublox {

/// Base class for all messages.
class Message {
public:
    UBLOX_EXPLICIT Message(uint8_t message_class, uint8_t message_id) UBLOX_NOEXCEPT;
    virtual ~Message() = default;

    Message(const Message& other) : mClass(other.mClass), mId(other.mId) {}
    Message(Message&&)                 = delete;
    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Get the message class.
    UBLOX_NODISCARD uint8_t message_class() const UBLOX_NOEXCEPT { return mClass; }

    /// Get the message id.
    UBLOX_NODISCARD uint8_t message_id() const UBLOX_NOEXCEPT { return mId; }

    /// Print the message to stdout.
    virtual void print() const UBLOX_NOEXCEPT = 0;

private:
    uint8_t mClass;
    uint8_t mId;
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    UBLOX_EXPLICIT UnsupportedMessage(uint8_t message_class, uint8_t message_id) UBLOX_NOEXCEPT;
    ~UnsupportedMessage() override = default;

    UnsupportedMessage(const UnsupportedMessage& other) : Message(other) {}
    UnsupportedMessage(UnsupportedMessage&&)                 = delete;
    UnsupportedMessage& operator=(const UnsupportedMessage&) = delete;
    UnsupportedMessage& operator=(UnsupportedMessage&&)      = delete;

    void print() const UBLOX_NOEXCEPT override;
};

}  // namespace ublox
}  // namespace receiver
