#include "messages/mon_ver.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

#include <cstdio>

#include <loglet/loglet.hpp>

LOGLET_MODULE3(ubx, msg, mon_ver);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF3(ubx, msg, mon_ver)

namespace format {
namespace ubx {

UbxMonVer::UbxMonVer(raw::MonVer payload, std::vector<uint8_t> data) NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

void UbxMonVer::print() const NOEXCEPT {
    printf("[%02X %02X] UBX-MON-VER:\n", message_class(), message_id());
    printf("[.....]    sw_version: \"%s\"\n", mPayload.sw_version.c_str());
    printf("[.....]    hw_version: \"%s\"\n", mPayload.hw_version.c_str());
    printf("[.....]    extensions:\n");
    for (auto const& extension : mPayload.extensions) {
        printf("[.....]        \"%s\"\n", extension.c_str());
    }
}

std::unique_ptr<Message> UbxMonVer::clone() const NOEXCEPT {
    return std::unique_ptr<Message>{new UbxMonVer{*this}};
}

std::unique_ptr<Message> UbxMonVer::parse(Decoder& decoder, std::vector<uint8_t> data) NOEXCEPT {
    if (decoder.remaining() < 40) {
        VERBOSEF("parse failed: insufficient data (need 40, have %u)", decoder.remaining());
        return nullptr;
    }

    auto extension_count = (decoder.remaining() - 40) / 30;

    auto payload       = raw::MonVer{};
    payload.sw_version = decoder.ch(30);
    payload.hw_version = decoder.ch(10);
    payload.extensions.reserve(extension_count);
    for (uint32_t i = 0; i < extension_count; ++i) {
        payload.extensions.push_back(decoder.ch(30));
    }

    if (decoder.error()) {
        VERBOSEF("parse failed: decoder error");
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxMonVer(std::move(payload), std::move(data))};
    }
}

uint32_t UbxMonVer::poll(Encoder& encoder) NOEXCEPT {
    encoder.u1(0xB5);
    encoder.u1(0x62);

    auto checksum_ptr = encoder.ptr();
    encoder.u1(CLASS_ID);
    encoder.u1(MESSAGE_ID);
    encoder.u2(0);

    auto checksum = Parser::checksum(checksum_ptr, 4);
    encoder.u2(checksum);

    return 8;
}

}  // namespace ubx
}  // namespace format
