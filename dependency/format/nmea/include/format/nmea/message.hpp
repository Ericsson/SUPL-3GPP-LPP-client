#pragma once
#include <core/core.hpp>

#include <memory>
#include <string>

namespace format {
namespace nmea {

/// Base class for all messages.
class Message {
public:
    EXPLICIT Message(std::string prefix, std::string payload, std::string checksum) NOEXCEPT;
    virtual ~Message();

    Message(Message const& other)
        : mPrefix(other.mPrefix), mPayload(other.mPayload), mChecksum(other.mChecksum) {}
    Message(Message&&)                 = delete;
    Message& operator=(Message const&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Get the message prefix, e.g. "$GPGGA".
    NODISCARD const std::string& prefix() const NOEXCEPT { return mPrefix; }

    /// Get the message payload.
    NODISCARD const std::string& payload() const NOEXCEPT { return mPayload; }

    /// Get the reconstructed sentence.
    NODISCARD std::string sentence() const NOEXCEPT {
        return mPrefix + "," + mPayload + "*" + mChecksum + "\r\n";
    }

    /// Print the message to stdout.
    virtual void print() const NOEXCEPT = 0;

    /// Clone the message.
    virtual std::unique_ptr<Message> clone() const NOEXCEPT = 0;

private:
    std::string mPrefix;
    std::string mPayload;
    std::string mChecksum;
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    EXPLICIT UnsupportedMessage(std::string prefix, std::string payload,
                                std::string checksum) NOEXCEPT;
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
    EXPLICIT ErrorMessage(std::string prefix, std::string payload, std::string checksum) NOEXCEPT;
    ~ErrorMessage() override = default;

    ErrorMessage(ErrorMessage const& other) : Message(other) {}
    ErrorMessage(ErrorMessage&&)                 = delete;
    ErrorMessage& operator=(ErrorMessage const&) = delete;
    ErrorMessage& operator=(ErrorMessage&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;
};

}  // namespace nmea
}  // namespace format
