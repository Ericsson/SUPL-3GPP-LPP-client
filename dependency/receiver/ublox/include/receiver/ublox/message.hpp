#pragma once
#include <receiver/ublox/types.hpp>

#include <vector>

namespace receiver {
namespace ublox {

/// Base class for all messages.
class Message {
public:
    UBLOX_EXPLICIT Message(uint8_t message_class, uint8_t message_id,
                           std::vector<uint8_t>&& data) UBLOX_NOEXCEPT;
    virtual ~Message() = default;

    Message(Message const& other) : mClass(other.mClass), mId(other.mId) {}
    Message(Message&&)                 = delete;
    Message& operator=(Message const&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Get the message class.
    UBLOX_NODISCARD uint8_t message_class() const UBLOX_NOEXCEPT { return mClass; }

    /// Get the message id.
    UBLOX_NODISCARD uint8_t message_id() const UBLOX_NOEXCEPT { return mId; }

    /// Get the raw message data.
    UBLOX_NODISCARD std::vector<uint8_t> const& data() const UBLOX_NOEXCEPT { return mData; }

    /// Print the message to stdout.
    virtual void print() const UBLOX_NOEXCEPT = 0;

private:
    uint8_t              mClass;
    uint8_t              mId;
    std::vector<uint8_t> mData;  // Raw message data
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    UBLOX_EXPLICIT UnsupportedMessage(uint8_t message_class, uint8_t message_id,
                                      std::vector<uint8_t>&& data) UBLOX_NOEXCEPT;
    ~UnsupportedMessage() override = default;

    UnsupportedMessage(UnsupportedMessage const& other) : Message(other) {}
    UnsupportedMessage(UnsupportedMessage&&)                 = delete;
    UnsupportedMessage& operator=(UnsupportedMessage const&) = delete;
    UnsupportedMessage& operator=(UnsupportedMessage&&)      = delete;

    void print() const UBLOX_NOEXCEPT override;
};

}  // namespace ublox
}  // namespace receiver
