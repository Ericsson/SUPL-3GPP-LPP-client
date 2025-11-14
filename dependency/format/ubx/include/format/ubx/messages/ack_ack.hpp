#pragma once
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ubx {

namespace raw {
struct AckAck {
    /* 0x00 */ uint8_t cls_id;
    /* 0x01 */ uint8_t msg_id;
};
}  // namespace raw

class Decoder;
class UbxAckAck : public Message {
public:
    CONSTEXPR static uint8_t CLASS_ID   = 0x05;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x01;

    EXPLICIT UbxAckAck(raw::AckAck payload, std::vector<uint8_t> data) NOEXCEPT;
    ~UbxAckAck() override = default;

    UbxAckAck(UbxAckAck const& other) : Message(other), mPayload(other.mPayload) {}
    UbxAckAck(UbxAckAck&&)                 = delete;
    UbxAckAck& operator=(UbxAckAck const&) = delete;
    UbxAckAck& operator=(UbxAckAck&&)      = delete;

    NODISCARD const raw::AckAck& payload() const NOEXCEPT { return mPayload; }

    NODISCARD uint8_t cls_id() const NOEXCEPT { return mPayload.cls_id; }
    NODISCARD uint8_t msg_id() const NOEXCEPT { return mPayload.msg_id; }

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;

private:
    raw::AckAck mPayload;
};

}  // namespace ubx
}  // namespace format
