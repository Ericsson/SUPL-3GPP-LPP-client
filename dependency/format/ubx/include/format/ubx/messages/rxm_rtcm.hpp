#pragma once
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ubx {

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
    CONSTEXPR static uint8_t CLASS_ID   = 0x02;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x32;

    EXPLICIT UbxRxmRtcm(raw::RxmRtcm payload, std::vector<uint8_t> data) NOEXCEPT;
    ~UbxRxmRtcm() override = default;

    UbxRxmRtcm(UbxRxmRtcm const& other) : Message(other), mPayload(other.mPayload) {}
    UbxRxmRtcm(UbxRxmRtcm&&)                 = delete;
    UbxRxmRtcm& operator=(UbxRxmRtcm const&) = delete;
    UbxRxmRtcm& operator=(UbxRxmRtcm&&)      = delete;

    NODISCARD const raw::RxmRtcm& payload() const NOEXCEPT { return mPayload; }

    NODISCARD uint8_t version() const NOEXCEPT { return mPayload.version; }
    NODISCARD bool    crc_failed() const NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.flags.crc_failed;
        } else {
            return false;
        }
    }
    NODISCARD uint8_t msg_used() const NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.flags.msg_used;
        } else {
            return 0;
        }
    }
    NODISCARD uint16_t sub_type() const NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.sub_type;
        } else {
            return 0;
        }
    }
    NODISCARD uint16_t ref_station() const NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.ref_station;
        } else {
            return 0;
        }
    }
    NODISCARD uint16_t msg_type() const NOEXCEPT {
        if (mPayload.version == 2) {
            return mPayload.data.v2.msg_type;
        } else {
            return 0;
        }
    }

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;

private:
    raw::RxmRtcm mPayload;
};

}  // namespace ubx
}  // namespace format
