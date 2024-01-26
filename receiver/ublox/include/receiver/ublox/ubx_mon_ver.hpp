#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <string>
#include <vector>

namespace receiver {
namespace ublox {

namespace raw {
struct MonVer {
    /* 0x00 */ std::string              sw_version;
    /* 0x1E */ std::string              hw_version;
    /* 0x28 */ std::vector<std::string> extensions;
};
}  // namespace raw

class Decoder;
class Encoder;
class UbxMonVer : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x0A;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x04;

    UBLOX_EXPLICIT UbxMonVer(raw::MonVer payload) UBLOX_NOEXCEPT;
    ~UbxMonVer() override = default;

    UbxMonVer(const UbxMonVer& other) : Message(other), mPayload(other.mPayload) {}
    UbxMonVer(UbxMonVer&&)                 = delete;
    UbxMonVer& operator=(const UbxMonVer&) = delete;
    UbxMonVer& operator=(UbxMonVer&&)      = delete;

    UBLOX_NODISCARD const raw::MonVer& payload() const UBLOX_NOEXCEPT { return mPayload; }

    UBLOX_NODISCARD const std::string& sw_version() const UBLOX_NOEXCEPT {
        return mPayload.sw_version;
    }
    UBLOX_NODISCARD const std::string& hw_version() const UBLOX_NOEXCEPT {
        return mPayload.hw_version;
    }
    UBLOX_NODISCARD const std::vector<std::string>& extensions() const UBLOX_NOEXCEPT {
        return mPayload.extensions;
    }

    void print() const UBLOX_NOEXCEPT override;

    UBLOX_NODISCARD static std::unique_ptr<Message> parse(Decoder& decoder) UBLOX_NOEXCEPT;
    UBLOX_NODISCARD static uint32_t                 poll(Encoder& encoder) UBLOX_NOEXCEPT;

private:
    raw::MonVer mPayload;
};

}  // namespace ublox
}  // namespace receiver
