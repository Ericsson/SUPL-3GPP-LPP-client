#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <string>
#include <vector>

namespace receiver {
namespace ublox {

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
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x02;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x33;

    UBLOX_EXPLICIT UbxRxmSpartn(raw::RxmSpartn payload, std::vector<uint8_t>&& data) UBLOX_NOEXCEPT;
    ~UbxRxmSpartn() override = default;

    UbxRxmSpartn(UbxRxmSpartn const& other) : Message(other), mPayload(other.mPayload) {}
    UbxRxmSpartn(UbxRxmSpartn&&)                 = delete;
    UbxRxmSpartn& operator=(UbxRxmSpartn const&) = delete;
    UbxRxmSpartn& operator=(UbxRxmSpartn&&)      = delete;

    UBLOX_NODISCARD const raw::RxmSpartn& payload() const UBLOX_NOEXCEPT { return mPayload; }

    UBLOX_NODISCARD uint8_t version() const UBLOX_NOEXCEPT { return mPayload.version; }
    UBLOX_NODISCARD bool    crc_failed() const UBLOX_NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.flags.crc_failed;
        } else {
            return false;
        }
    }
    UBLOX_NODISCARD uint8_t msg_used() const UBLOX_NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.flags.msg_used;
        } else {
            return 0;
        }
    }
    UBLOX_NODISCARD uint16_t sub_type() const UBLOX_NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.sub_type;
        } else {
            return 0;
        }
    }
    UBLOX_NODISCARD uint16_t msg_type() const UBLOX_NOEXCEPT {
        if (mPayload.version == 1) {
            return mPayload.data.v1.msg_type;
        } else {
            return 0;
        }
    }

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message>
    parse(Decoder& decoder, std::vector<uint8_t>&& raw_data) UBLOX_NOEXCEPT;

private:
    raw::RxmSpartn mPayload;
};

}  // namespace ublox
}  // namespace receiver
