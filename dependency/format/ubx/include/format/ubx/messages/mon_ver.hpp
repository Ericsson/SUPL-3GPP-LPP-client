#pragma once
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <vector>

namespace format {
namespace ubx {

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
    CONSTEXPR static uint8_t CLASS_ID   = 0x0A;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x04;

    EXPLICIT UbxMonVer(raw::MonVer payload, std::vector<uint8_t> data) NOEXCEPT;
    ~UbxMonVer() override = default;

    UbxMonVer(UbxMonVer const& other) : Message(other), mPayload(other.mPayload) {}
    UbxMonVer(UbxMonVer&&)                 = delete;
    UbxMonVer& operator=(UbxMonVer const&) = delete;
    UbxMonVer& operator=(UbxMonVer&&)      = delete;

    NODISCARD const raw::MonVer& payload() const NOEXCEPT { return mPayload; }

    NODISCARD const std::string& sw_version() const NOEXCEPT { return mPayload.sw_version; }
    NODISCARD const std::string& hw_version() const NOEXCEPT { return mPayload.hw_version; }
    NODISCARD const std::vector<std::string>& extensions() const NOEXCEPT {
        return mPayload.extensions;
    }

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;
    NODISCARD static uint32_t                 poll(Encoder& encoder) NOEXCEPT;

private:
    raw::MonVer mPayload;
};

}  // namespace ubx
}  // namespace format
