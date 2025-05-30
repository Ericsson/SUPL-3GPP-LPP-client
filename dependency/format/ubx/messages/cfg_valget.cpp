#include "messages/cfg_valget.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

#include <stdio.h>

#include <loglet/loglet.hpp>

LOGLET_MODULE_FORWARD_REF(ubx);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ubx)

namespace format {
namespace ubx {

UbxCfgValget::UbxCfgValget(raw::CfgValget payload, std::vector<uint8_t> data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

void UbxCfgValget::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-CFG-VALGET:\n", message_class(), message_id());
    printf("[.....]    version: %u\n", mPayload.version);
    printf("[.....]    layers:\n");
    if (mPayload.layers & CFG_LAYER_RAM) {
        printf("[.....]        RAM\n");
    }
    if (mPayload.layers & CFG_LAYER_BBR) {
        printf("[.....]        BBR\n");
    }
    if (mPayload.layers & CFG_LAYER_FLASH) {
        printf("[.....]        FLASH\n");
    }
    printf("[.....]    position: %u\n", mPayload.position);
    printf("[.....]    data:\n");
    for (auto const& kvp : mPayload.values) {
        auto  key   = kvp.first;
        auto& value = kvp.second;
        printf("[.....]        %08X: ", key);
        switch (value.type()) {
        case CfgValue::U1:
            printf("%02X, %u, %i\n", value.u1(), static_cast<uint32_t>(value.u1()),
                   static_cast<int32_t>(value.u1()));
            break;
        case CfgValue::U2:
            printf("%04X, %u, %i\n", value.u2(), static_cast<uint32_t>(value.u2()),
                   static_cast<int32_t>(value.u2()));
            break;
        case CfgValue::U4:
            printf("%08X, %u, %i\n", value.u4(), static_cast<uint32_t>(value.u4()),
                   static_cast<int32_t>(value.u4()));
            break;
        case CfgValue::U8:
            printf("%016" PRIx64 ", %" PRIu64 ", %" PRId64 "\n", value.u8(),
                   static_cast<uint64_t>(value.u8()), static_cast<int64_t>(value.u8()));
            break;
        case CfgValue::L:
        case CfgValue::UNKNOWN: printf("???\n"); break;
        }
    }
}

std::unique_ptr<Message> UbxCfgValget::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UbxCfgValget{*this}};
}

CfgValue UbxCfgValget::get(CfgKey key) const NOEXCEPT {
    auto it = mPayload.values.find(key);
    if (it != mPayload.values.end()) {
        return it->second;
    } else {
        return CfgValue{};
    }
}

std::unique_ptr<Message> UbxCfgValget::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    if (decoder.remaining() < 4) {
        return nullptr;
    }

    auto payload     = raw::CfgValget{};
    payload.version  = decoder.U1();
    payload.layers   = decoder.U1();
    payload.position = decoder.U2();
    payload.values.reserve(decoder.remaining() / 6);  // Estimate the number of key-value pairs

    while (decoder.remaining() > 4 && !decoder.error()) {
        auto key   = static_cast<CfgKey>(decoder.U4());
        auto value = CfgValue::parse_from_key(key, decoder);
        payload.values.emplace(key, value);
    }

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxCfgValget(std::move(payload), std::move(data))};
    }
}

uint32_t UbxCfgValget::poll(Encoder& encoder, CfgLayer layer, uint16_t position,
                            std::vector<CfgKey> const& keys) NOEXCEPT {
    ASSERT(keys.size() <= 0xFF, "Too many keys to poll (max 255)");

    auto begin = encoder.ptr();

    uint16_t payload_size = 4;                               // 4 bytes for header
    payload_size += 4 * static_cast<uint16_t>(keys.size());  // 4 bytes per key

    encoder.U1(0xB5);
    encoder.U1(0x62);

    auto checksum_begin = encoder.ptr();
    encoder.U1(CLASS_ID);
    encoder.U1(MESSAGE_ID);
    encoder.U2(payload_size);

    encoder.U1(0);  // version
    // you can only poll one layer at a time
    if (layer & CFG_LAYER_RAM) {
        encoder.U1(0);
    } else if (layer & CFG_LAYER_BBR) {
        encoder.U1(1);
    } else if (layer & CFG_LAYER_FLASH) {
        encoder.U1(2);
    } else {
        encoder.U1(0);
    }
    encoder.U2(position);

    for (auto& key : keys) {
        encoder.U4(key);
    }

    auto checksum_end    = encoder.ptr();
    auto checksum_length = static_cast<uint32_t>(checksum_end - checksum_begin);
    auto checksum        = Parser::checksum(checksum_begin, checksum_length);
    encoder.U2(checksum);

    auto end = encoder.ptr();
    return static_cast<uint32_t>(end - begin);
}

}  // namespace ubx
}  // namespace format
