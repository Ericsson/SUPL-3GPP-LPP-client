#include "ubx_cfg_valset.hpp"
#include <cinttypes>
#include "encoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

uint32_t UbxCfgValset::set(Encoder& encoder, CfgLayer layers, CfgKey key,
                           CfgValue value) UBLOX_NOEXCEPT {
    auto expected_type = CfgValue::type_from_key(key);
    if (value.type() != expected_type || value.type() == CfgValue::UNKNOWN) {
        return 0;
    }

    auto begin        = encoder.ptr();
    auto payload_size = 4 + 4 + value.size();

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

    auto checksum_end = encoder.ptr();
    auto checksum =
        Parser::checksum(checksum_begin, static_cast<uint32_t>(checksum_end - checksum_begin));
    encoder.U2(checksum);

    auto end = encoder.ptr();
    return static_cast<uint32_t>(end - begin);
}

}  // namespace ublox
}  // namespace receiver
