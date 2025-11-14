#pragma once
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ubx {

namespace raw {
struct AckNak {
    /* 0x00 */ uint8_t cls_id;
    /* 0x01 */ uint8_t msg_id;
};
}  // namespace raw

class Decoder;
class UbxAckNak : public Message {
public:
    CONSTEXPR static uint8_t CLASS_ID   = 0x05;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x00;

    EXPLICIT UbxAckNak(raw::AckNak payload, std::vector<uint8_t> data) NOEXCEPT;
    ~UbxAckNak() override = default;

    UbxAckNak(UbxAckNak const& other) : Message(other), mPayload(other.mPayload) {}
    UbxAckNak(UbxAckNak&&)                 = delete;
    UbxAckNak& operator=(UbxAckNak const&) = delete;
    UbxAckNak& operator=(UbxAckNak&&)      = delete;

    NODISCARD const raw::AckNak& payload() const NOEXCEPT { return mPayload; }

    NODISCARD uint8_t cls_id() const NOEXCEPT { return mPayload.cls_id; }
    NODISCARD uint8_t msg_id() const NOEXCEPT { return mPayload.msg_id; }

    void      print() const NOEXCEPT override;
    NODISCARD std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;

private:
    raw::AckNak mPayload;
};

}  // namespace ubx
}  // namespace format
