#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <receiver/ublox/ubx_cfg.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace receiver {
namespace ublox {

namespace raw {
struct CfgValget {
    /* 0x00 */ uint8_t                              version;
    /* 0x01 */ CfgLayer                             layers;
    /* 0x02 */ uint16_t                             position;
    /* 0x04 */ std::unordered_map<CfgKey, CfgValue> values;
};
}  // namespace raw

class Decoder;
class Encoder;
class UbxCfgValget : public Message {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x06;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x8B;

    UBLOX_EXPLICIT UbxCfgValget(raw::CfgValget payload, std::vector<uint8_t>&& data) UBLOX_NOEXCEPT;
    ~UbxCfgValget() override = default;

    UbxCfgValget(UbxCfgValget const& other) : Message(other), mPayload(other.mPayload) {}
    UbxCfgValget(UbxCfgValget&&)                 = delete;
    UbxCfgValget& operator=(UbxCfgValget const&) = delete;
    UbxCfgValget& operator=(UbxCfgValget&&)      = delete;

    void print() const UBLOX_NOEXCEPT override;

    /// Get the value of a configuration key. If the key is not present,
    /// a default-constructed CfgValue will be returned, i.e., of type CfgValue::UNKNOWN.
    CfgValue get(CfgKey key) const UBLOX_NOEXCEPT;

    UBLOX_NODISCARD static std::unique_ptr<Message>
    parse(Decoder& decoder, std::vector<uint8_t>&& raw_data) UBLOX_NOEXCEPT;
    UBLOX_NODISCARD static uint32_t poll(Encoder& encoder, CfgLayer layer, uint16_t position,
                                         std::vector<CfgKey> const& keys) UBLOX_NOEXCEPT;

private:
    raw::CfgValget mPayload;
};

}  // namespace ublox
}  // namespace receiver
