#pragma once
#include <receiver/ublox/types.hpp>

namespace receiver {
namespace ublox {

class Message {
public:
    UBLOX_EXPLICIT Message(uint8_t message_class, uint8_t message_id) UBLOX_NOEXCEPT;
    virtual ~Message() = default;

    UBLOX_NODISCARD uint8_t message_class() const UBLOX_NOEXCEPT { return mClass; }
    UBLOX_NODISCARD uint8_t message_id() const UBLOX_NOEXCEPT { return mId; }

    virtual void print() const UBLOX_NOEXCEPT = 0;

private:
    uint8_t mClass;
    uint8_t mId;
};

class UnsupportedMessage final : public Message {
public:
    UBLOX_EXPLICIT UnsupportedMessage(uint8_t message_class, uint8_t message_id) UBLOX_NOEXCEPT;
    ~UnsupportedMessage() override = default;

    void print() const UBLOX_NOEXCEPT override;
};

}  // namespace ublox
}  // namespace receiver
