#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <string>
#include <vector>

namespace receiver {
namespace ublox {

namespace raw {
struct AckNak {
    /* 0x00 */ uint8_t cls_id;
    /* 0x01 */ uint8_t msg_id;
};
}  // namespace raw

class Decoder;
class UbxAckNak : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x05;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x00;

    UBLOX_EXPLICIT UbxAckNak(raw::AckNak payload) UBLOX_NOEXCEPT;
    ~UbxAckNak() override = default;

    UbxAckNak(const UbxAckNak& other) : Message(other), mPayload(other.mPayload) {}
    UbxAckNak(UbxAckNak&&)                 = delete;
    UbxAckNak& operator=(const UbxAckNak&) = delete;
    UbxAckNak& operator=(UbxAckNak&&)      = delete;

    UBLOX_NODISCARD const raw::AckNak& payload() const UBLOX_NOEXCEPT { return mPayload; }

    UBLOX_NODISCARD uint8_t cls_id() const UBLOX_NOEXCEPT { return mPayload.cls_id; }
    UBLOX_NODISCARD uint8_t msg_id() const UBLOX_NOEXCEPT { return mPayload.msg_id; }

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message> parse(Decoder& decoder) UBLOX_NOEXCEPT;

private:
    raw::AckNak mPayload;
};

}  // namespace ublox
}  // namespace receiver
