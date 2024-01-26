#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <string>
#include <vector>

namespace receiver {
namespace ublox {

namespace raw {
struct AckAck {
    /* 0x00 */ uint8_t cls_id;
    /* 0x01 */ uint8_t msg_id;
};
}  // namespace raw

class Decoder;
class UbxAckAck : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x05;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x01;

    UBLOX_EXPLICIT UbxAckAck(raw::AckAck payload) UBLOX_NOEXCEPT;
    ~UbxAckAck() override = default;

    UbxAckAck(const UbxAckAck& other) : Message(other), mPayload(other.mPayload) {}
    UbxAckAck(UbxAckAck&&)                 = delete;
    UbxAckAck& operator=(const UbxAckAck&) = delete;
    UbxAckAck& operator=(UbxAckAck&&)      = delete;

    UBLOX_NODISCARD const raw::AckAck& payload() const UBLOX_NOEXCEPT { return mPayload; }

    UBLOX_NODISCARD uint8_t cls_id() const UBLOX_NOEXCEPT { return mPayload.cls_id; }
    UBLOX_NODISCARD uint8_t msg_id() const UBLOX_NOEXCEPT { return mPayload.msg_id; }

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message> parse(Decoder& decoder) UBLOX_NOEXCEPT;

private:
    raw::AckAck mPayload;
};

}  // namespace ublox
}  // namespace receiver
