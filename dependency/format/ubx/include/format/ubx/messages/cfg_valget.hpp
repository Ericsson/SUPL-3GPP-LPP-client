#pragma once
#include <format/ubx/cfg.hpp>
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace format {
namespace ubx {

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
    CONSTEXPR static uint8_t CLASS_ID   = 0x06;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x8B;

    EXPLICIT UbxCfgValget(raw::CfgValget payload, std::vector<uint8_t> data) NOEXCEPT;
    ~UbxCfgValget() override = default;

    UbxCfgValget(UbxCfgValget const& other) : Message(other), mPayload(other.mPayload) {}
    UbxCfgValget(UbxCfgValget&&)                 = delete;
    UbxCfgValget& operator=(UbxCfgValget const&) = delete;
    UbxCfgValget& operator=(UbxCfgValget&&)      = delete;

    void                     print() const NOEXCEPT override;
    std::unique_ptr<Message> clone() const NOEXCEPT override;

    /// Get the value of a configuration key. If the key is not present,
    /// a default-constructed CfgValue will be returned, i.e., of type CfgValue::UNKNOWN.
    CfgValue get(CfgKey key) const NOEXCEPT;

    NODISCARD static std::unique_ptr<Message> parse(Decoder&             decoder,
                                                    std::vector<uint8_t> data) NOEXCEPT;
    NODISCARD static uint32_t poll(Encoder& encoder, CfgLayer layer, uint16_t position,
                                   std::vector<CfgKey> const& keys) NOEXCEPT;

private:
    raw::CfgValget mPayload;
};

}  // namespace ubx
}  // namespace format
