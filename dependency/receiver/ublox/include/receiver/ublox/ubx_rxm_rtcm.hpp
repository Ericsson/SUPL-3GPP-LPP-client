#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <string>
#include <vector>

namespace receiver {
namespace ublox {

namespace raw {
struct RxmRtcm {
    /* 0x00 */ uint8_t version;
    union {
        struct {
            /* 0x01 */ struct {
                /* bit 0 */ uint8_t   crc_failed;
                /* bit 1-2 */ uint8_t msg_used;
            } flags;
            /* 0x02 */ uint16_t sub_type;
            /* 0x04 */ uint16_t ref_station;
            /* 0x06 */ uint16_t msg_type;
        } v2;
    } data;
};
}  // namespace raw

class Decoder;
class Encoder;
class UbxRxmRtcm : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x02;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x32;

    UBLOX_EXPLICIT UbxRxmRtcm(raw::RxmRtcm payload, std::vector<uint8_t>&& data) UBLOX_NOEXCEPT;
    ~UbxRxmRtcm() override = default;

    UbxRxmRtcm(UbxRxmRtcm const& other) : Message(other), mPayload(other.mPayload) {}
    UbxRxmRtcm(UbxRxmRtcm&&)                 = delete;
    UbxRxmRtcm& operator=(UbxRxmRtcm const&) = delete;
    UbxRxmRtcm& operator=(UbxRxmRtcm&&)      = delete;

    UBLOX_NODISCARD const raw::RxmRtcm& payload() const UBLOX_NOEXCEPT { return mPayload; }

    UBLOX_NODISCARD uint8_t version() const UBLOX_NOEXCEPT { return mPayload.version; }
    UBLOX_NODISCARD bool    crc_failed() const UBLOX_NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.flags.crc_failed;
        } else {
            return false;
        }
    }
    UBLOX_NODISCARD uint8_t msg_used() const UBLOX_NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.flags.msg_used;
        } else {
            return 0;
        }
    }
    UBLOX_NODISCARD uint16_t sub_type() const UBLOX_NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.sub_type;
        } else {
            return 0;
        }
    }
    UBLOX_NODISCARD uint16_t ref_station() const UBLOX_NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.ref_station;
        } else {
            return 0;
        }
    }
    UBLOX_NODISCARD uint16_t msg_type() const UBLOX_NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.msg_type;
        } else {
            return 0;
        }
    }

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message>
    parse(Decoder& decoder, std::vector<uint8_t>&& raw_data) UBLOX_NOEXCEPT;

private:
    raw::RxmRtcm mPayload;
};

}  // namespace ublox
}  // namespace receiver
