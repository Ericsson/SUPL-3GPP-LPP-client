#pragma once
#include <format/ubx/cfg.hpp>
#include <format/ubx/message.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace format {
namespace ubx {

class Encoder;
class UbxCfgValset {
public:
    CONSTEXPR static uint8_t CLASS_ID   = 0x06;
    CONSTEXPR static uint8_t MESSAGE_ID = 0x8A;

    NODISCARD static uint32_t set(Encoder& encoder, CfgLayer layers, CfgKey key,
                                        CfgValue value) NOEXCEPT;
};

}  // namespace ubx
}  // namespace format
