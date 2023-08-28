#pragma once
#include <memory>
#include <receiver/ublox/message.hpp>
#include <receiver/ublox/ubx_cfg.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace receiver {
namespace ublox {

class Encoder;
class UbxCfgValset {
public:
    UBLOX_CONSTEXPR static uint8_t CLASS_ID   = 0x06;
    UBLOX_CONSTEXPR static uint8_t MESSAGE_ID = 0x8A;

    UBLOX_NODISCARD static uint32_t set(Encoder& encoder, CfgLayer layers, CfgKey key,
                                        CfgValue value) UBLOX_NOEXCEPT;
};

}  // namespace ublox
}  // namespace receiver
