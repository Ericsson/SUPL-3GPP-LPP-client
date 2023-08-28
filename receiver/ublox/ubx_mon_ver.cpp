#include "ubx_mon_ver.hpp"
#include <stdio.h>
#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

UbxMonVer::UbxMonVer(raw::MonVer payload) UBLOX_NOEXCEPT : Message(CLASS_ID, MESSAGE_ID),
                                                           mPayload(std::move(payload)) {}

void UbxMonVer::print() const UBLOX_NOEXCEPT {
    printf("[%02X %02X] UBX-MON-VER:\n", message_class(), message_id());
    printf("[.....]    sw_version: \"%s\"\n", mPayload.sw_version.c_str());
    printf("[.....]    hw_version: \"%s\"\n", mPayload.hw_version.c_str());
    printf("[.....]    extensions:\n");
    for (const auto& extension : mPayload.extensions) {
        printf("[.....]        \"%s\"\n", extension.c_str());
    }
}

std::unique_ptr<Message> UbxMonVer::parse(Decoder& decoder) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 40) {
        return nullptr;
    }

    auto extension_count = (decoder.remaining() - 40) / 30;

    auto payload       = raw::MonVer{};
    payload.sw_version = decoder.CH(30);
    payload.hw_version = decoder.CH(10);
    payload.extensions.reserve(extension_count);
    for (uint32_t i = 0; i < extension_count; ++i) {
        payload.extensions.push_back(decoder.CH(30));
    }

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxMonVer(std::move(payload))};
    }
}

uint32_t UbxMonVer::poll(Encoder& encoder) UBLOX_NOEXCEPT {
    encoder.U1(0xB5);
    encoder.U1(0x62);

    auto checksum_ptr = encoder.ptr();
    encoder.U1(CLASS_ID);
    encoder.U1(MESSAGE_ID);
    encoder.U2(0);

    auto checksum = Parser::checksum(checksum_ptr, 4);
    encoder.U2(checksum);

    return 8;
}

}  // namespace ublox
}  // namespace receiver
