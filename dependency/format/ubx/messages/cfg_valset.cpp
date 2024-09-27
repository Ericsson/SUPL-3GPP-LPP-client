#include "messages/cfg_valset.hpp"
#include "encoder.hpp"
#include "parser.hpp"

namespace format {
namespace ubx {

uint32_t UbxCfgValset::set(Encoder& encoder, CfgLayer layers, CfgKey key,
                           CfgValue value) NOEXCEPT {
    auto expected_type = CfgValue::type_from_key(key);
    if (value.type() != expected_type || value.type() == CfgValue::UNKNOWN) {
        return 0;
    }

    auto begin = encoder.ptr();

    uint16_t payload_size = 4;     // 4 bytes for header
    payload_size += 4;             // 4 bytes for key
    payload_size += value.size();  // size of value

    encoder.U1(0xB5);
    encoder.U1(0x62);

    auto checksum_begin = encoder.ptr();
    encoder.U1(CLASS_ID);
    encoder.U1(MESSAGE_ID);
    encoder.U2(payload_size);

    encoder.U1(0);  // version
    encoder.U1(layers);
    encoder.U2(0);  // reserved

    encoder.U4(key);
    value.serialize(encoder);

    auto checksum_end    = encoder.ptr();
    auto checksum_length = static_cast<uint32_t>(checksum_end - checksum_begin);
    auto checksum        = Parser::checksum(checksum_begin, checksum_length);
    encoder.U2(checksum);

    auto end = encoder.ptr();
    return static_cast<uint32_t>(end - begin);
}

}  // namespace ubx
}  // namespace format
