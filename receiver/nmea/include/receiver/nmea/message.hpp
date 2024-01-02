#pragma once
#include <receiver/nmea/types.hpp>

#include <string>

namespace receiver {
namespace nmea {

/// Base class for all messages.
class Message {
public:
    NMEA_EXPLICIT Message(std::string prefix) NMEA_NOEXCEPT;
    virtual ~Message() = default;

    /// Get the message prefix, e.g. "$GPGGA".
    NMEA_NODISCARD const std::string& prefix() const NMEA_NOEXCEPT { return mPrefix; }

    /// Print the message to stdout.
    virtual void print() const NMEA_NOEXCEPT = 0;

private:
    std::string mPrefix;
};

/// Unsupported or unknown message.
class UnsupportedMessage final : public Message {
public:
    NMEA_EXPLICIT UnsupportedMessage(std::string prefix, std::string payload) NMEA_NOEXCEPT;
    ~UnsupportedMessage() override = default;

    void print() const NMEA_NOEXCEPT override;

    /// Get the message payload.
    NMEA_NODISCARD const std::string& payload() const NMEA_NOEXCEPT { return mPayload; }

private:
    std::string mPayload;
};

}  // namespace nmea
}  // namespace receiver
