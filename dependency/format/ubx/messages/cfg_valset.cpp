#include "messages/cfg_valset.hpp"
#include "encoder.hpp"
#include "parser.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE3(ubx, msg, cfg_valset);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(ubx, msg, cfg_valset)

namespace format {
namespace ubx {

uint32_t UbxCfgValset::set(Encoder& encoder, CfgLayer layers, CfgKey key, CfgValue value) NOEXCEPT {
    auto expected_type = CfgValue::type_from_key(key);
    if (value.type() != expected_type || value.type() == CfgValue::UNKNOWN) {
        VERBOSEF("set failed: type mismatch (expected %d, got %d)", expected_type, value.type());
        return 0;
    }

    auto begin = encoder.ptr();

    uint16_t payload_size = 4;     // 4 bytes for header
    payload_size += 4;             // 4 bytes for key
    payload_size += value.size();  // size of value

    encoder.u1(0xB5);
    encoder.u1(0x62);

    auto checksum_begin = encoder.ptr();
    encoder.u1(CLASS_ID);
    encoder.u1(MESSAGE_ID);
    encoder.u2(payload_size);

    encoder.u1(0);  // version
    encoder.u1(layers);
    encoder.u2(0);  // reserved

    encoder.u4(key);
    value.serialize(encoder);

    auto checksum_end    = encoder.ptr();
    auto checksum_length = static_cast<uint32_t>(checksum_end - checksum_begin);
    auto checksum        = Parser::checksum(checksum_begin, checksum_length);
    encoder.u2(checksum);

    auto end = encoder.ptr();
    return static_cast<uint32_t>(end - begin);
}

}  // namespace ubx
}  // namespace format
