#pragma once
#include <core/core.hpp>

#include <memory>
#include <vector>

namespace format {
namespace ubx {
class Message {
public:
    EXPLICIT Message(uint8_t message_class, uint8_t message_id, std::vector<uint8_t> data) NOEXCEPT;
    virtual ~Message() NOEXCEPT;

    Message(Message const& other) : mClass(other.mClass), mId(other.mId) {}
    Message(Message&&)                 = delete;
    Message& operator=(Message const&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Get the message class.
    NODISCARD uint8_t message_class() const NOEXCEPT { return mClass; }

    /// Get the message id.
    NODISCARD uint8_t message_id() const NOEXCEPT { return mId; }

    /// Get the raw message data.
    NODISCARD std::vector<uint8_t> const& data() const NOEXCEPT { return mData; }

    /// Print the message to stdout.
    virtual void print() const NOEXCEPT = 0;

    /// Clone the message.
    virtual std::unique_ptr<Message> clone() const NOEXCEPT = 0;

private:
    uint8_t              mClass;
    uint8_t              mId;
    std::vector<uint8_t> mData;  // Raw message data
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    EXPLICIT UnsupportedMessage(uint8_t message_class, uint8_t message_id, std::vector<uint8_t> data) NOEXCEPT;
    ~UnsupportedMessage() override = default;

    UnsupportedMessage(UnsupportedMessage const& other) : Message(other) {}
    UnsupportedMessage(UnsupportedMessage&&)                 = delete;
    UnsupportedMessage& operator=(UnsupportedMessage const&) = delete;
    UnsupportedMessage& operator=(UnsupportedMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;
};

}  // namespace ubx
}  // namespace format
