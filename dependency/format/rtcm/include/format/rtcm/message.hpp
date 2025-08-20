#pragma once
#include <core/core.hpp>

#include <memory>
#include <vector>
#include "datafields.hpp"

namespace format {
namespace rtcm {

/// Base class for all messages.
class Message {
public:
    EXPLICIT Message(DF002 type, std::vector<uint8_t> data) NOEXCEPT;
    virtual ~Message() NOEXCEPT;

    Message(Message const& other) : mData{other.mData} {}
    Message(Message&&)                 = delete;
    Message& operator=(Message const&) = delete;
    Message& operator=(Message&&)      = delete;

    NODISCARD std::vector<uint8_t> const& data() const NOEXCEPT { return mData; }

    NODISCARD int type() const NOEXCEPT { return mType; }

    /// Print the message to stdout.
    virtual void print() const NOEXCEPT = 0;

    /// Clone the message.
    virtual std::unique_ptr<Message> clone() const NOEXCEPT = 0;

protected:
    DF002 mType;
    std::vector<uint8_t> mData;
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    EXPLICIT UnsupportedMessage(DF002 type, std::vector<uint8_t> mData) NOEXCEPT;
    ~UnsupportedMessage() override = default;

    UnsupportedMessage(UnsupportedMessage const& other) : Message(other) {}
    UnsupportedMessage(UnsupportedMessage&&)                 = delete;
    UnsupportedMessage& operator=(UnsupportedMessage const&) = delete;
    UnsupportedMessage& operator=(UnsupportedMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;
};

/// Error message. This is used to indicate that the message could not be parsed.
class ErrorMessage final : public Message {
public:
    EXPLICIT ErrorMessage(DF002 type, std::vector<uint8_t> mData) NOEXCEPT;
    ~ErrorMessage() override = default;

    ErrorMessage(ErrorMessage const& other) : Message(other) {}
    ErrorMessage(ErrorMessage&&)                 = delete;
    ErrorMessage& operator=(ErrorMessage const&) = delete;
    ErrorMessage& operator=(ErrorMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;
};

}  // namespace rtcm
}  // namespace format
