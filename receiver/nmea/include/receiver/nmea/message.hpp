#pragma once
#include <receiver/nmea/types.hpp>

#include <string>

namespace receiver {
namespace nmea {

/// Base class for all messages.
class Message {
public:
    NMEA_EXPLICIT Message(std::string prefix, std::string payload,
                          std::string checksum) NMEA_NOEXCEPT;
    virtual ~Message() = default;

    Message(const Message& other)
        : mPrefix(other.mPrefix), mPayload(other.mPayload), mChecksum(other.mChecksum) {}
    Message(Message&&)                 = delete;
    Message& operator=(const Message&) = delete;
    Message& operator=(Message&&)      = delete;

    /// Get the message prefix, e.g. "$GPGGA".
    NMEA_NODISCARD const std::string& prefix() const NMEA_NOEXCEPT { return mPrefix; }

    /// Get the message payload.
    NMEA_NODISCARD const std::string& payload() const NMEA_NOEXCEPT { return mPayload; }

    /// Get the reconstructed sentence.
    NMEA_NODISCARD std::string sentence() const NMEA_NOEXCEPT {
        return mPrefix + "," + mPayload + "*" + mChecksum + "\r\n";
    }

    /// Print the message to stdout.
    virtual void print() const NMEA_NOEXCEPT = 0;

private:
    std::string mPrefix;
    std::string mPayload;
    std::string mChecksum;
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    NMEA_EXPLICIT UnsupportedMessage(std::string prefix, std::string payload,
                                     std::string checksum) NMEA_NOEXCEPT;
    ~UnsupportedMessage() override = default;

    UnsupportedMessage(const UnsupportedMessage& other) : Message(other) {}
    UnsupportedMessage(UnsupportedMessage&&)                 = delete;
    UnsupportedMessage& operator=(const UnsupportedMessage&) = delete;
    UnsupportedMessage& operator=(UnsupportedMessage&&)      = delete;

    void print() const NMEA_NOEXCEPT override;
};

/// Error message. This is used to indicate that the message could not be parsed.
class ErrorMessage final : public Message {
public:
    NMEA_EXPLICIT ErrorMessage(std::string prefix, std::string payload,
                               std::string checksum) NMEA_NOEXCEPT;
    ~ErrorMessage() override = default;

    ErrorMessage(const ErrorMessage& other) : Message(other) {}
    ErrorMessage(ErrorMessage&&)                 = delete;
    ErrorMessage& operator=(const ErrorMessage&) = delete;
    ErrorMessage& operator=(ErrorMessage&&)      = delete;

    void print() const NMEA_NOEXCEPT override;
};

}  // namespace nmea
}  // namespace receiver
