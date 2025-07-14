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
    EXPLICIT Message(std::vector<uint8_t> data) NOEXCEPT;
    virtual ~Message() NOEXCEPT;

    Message(Message const& other) : data{other.data} {}
    Message(Message&&)                 = delete;
    Message& operator=(Message const&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Print the message to stdout.
    virtual void print() const NOEXCEPT = 0;

    /// Clone the message.
    virtual std::unique_ptr<Message> clone() const NOEXCEPT = 0;

protected:
    DF002 type;
private:
    std::vector<uint8_t> data;
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    EXPLICIT UnsupportedMessage(unsigned type, std::vector<uint8_t> data) NOEXCEPT;
    ~UnsupportedMessage() override = default;

    UnsupportedMessage(UnsupportedMessage const& other) : Message(other), type{other.type} {}
    UnsupportedMessage(UnsupportedMessage&&)                 = delete;
    UnsupportedMessage& operator=(UnsupportedMessage const&) = delete;
    UnsupportedMessage& operator=(UnsupportedMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;
    const unsigned type;
};

/// Error message. This is used to indicate that the message could not be parsed.
class ErrorMessage final : public Message {
public:
    EXPLICIT ErrorMessage(/* TODO */) NOEXCEPT;
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
