#pragma once
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ubx {

namespace raw {
struct RxmSpartn {
    /* 0x00 */ uint8_t version;
    union {
        struct {
            /* 0x01 */ struct {
                /* bit 0 */ uint8_t   crc_failed;
                /* bit 1-2 */ uint8_t msg_used;
            } flags;
            /* 0x02 */ uint16_t sub_type;
            /* 0x04 */ uint16_t padding04;
            /* 0x06 */ uint16_t msg_type;
        } v1;
    } data;
};
}  // namespace raw

class Decoder;
class Encoder;
class UbxRxmSpartn : public Message {
public:
    CONSTEXPR static uint8_t CLASS_ID   = 0x02;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x33;

    EXPLICIT UbxRxmSpartn(raw::RxmSpartn payload, std::vector<uint8_t> data) NOEXCEPT;
    ~UbxRxmSpartn() override = default;

    UbxRxmSpartn(UbxRxmSpartn const& other) : Message(other), mPayload(other.mPayload) {}
    UbxRxmSpartn(UbxRxmSpartn&&)                 = delete;
    UbxRxmSpartn& operator=(UbxRxmSpartn const&) = delete;
    UbxRxmSpartn& operator=(UbxRxmSpartn&&)      = delete;

    NODISCARD const raw::RxmSpartn& payload() const NOEXCEPT { return mPayload; }

    NODISCARD uint8_t version() const NOEXCEPT { return mPayload.version; }
    NODISCARD bool    crc_failed() const NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.flags.crc_failed;
        } else {
            return false;
        }
    }
    NODISCARD uint8_t msg_used() const NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.flags.msg_used;
        } else {
            return 0;
        }
    }
    NODISCARD uint16_t sub_type() const NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.sub_type;
        } else {
            return 0;
        }
    }
    NODISCARD uint16_t msg_type() const NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.msg_type;
        } else {
            return 0;
        }
    }

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;

private:
    raw::RxmSpartn mPayload;
};

}  // namespace ubx
}  // namespace format
