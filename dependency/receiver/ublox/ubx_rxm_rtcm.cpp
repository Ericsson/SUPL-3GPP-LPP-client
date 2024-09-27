#include "ubx_rxm_rtcm.hpp"

#include <stdio.h>

#include "decoder.hpp"
#include "encoder.hpp"
#include "parser.hpp"

namespace receiver {
namespace ublox {

UbxRxmRtcm::UbxRxmRtcm(raw::RxmRtcm payload, std::vector<uint8_t>&& data) UBLOX_NOEXCEPT
    : Message(CLASS_ID, MESSAGE_ID, std::move(data)),
      mPayload(std::move(payload)) {}

static char const* msg_used_str(uint8_t msg_used) UBLOX_NOEXCEPT {
    switch (msg_used) {
    case 0: return "DO NOT KNOW";
    case 1: return "NOT USED";
    case 2: return "USED";
    default: return "?";
    }
}

void UbxRxmRtcm::print() const UBLOX_NOEXCEPT {
    if (mPayload.version == 2) {
        printf("[%02X %02X] UBX-RXM-RTCM: %4u %s%s\n", message_class(), message_id(),
               mPayload.data.v2.msg_type, msg_used_str(mPayload.data.v2.flags.msg_used),
               mPayload.data.v2.flags.crc_failed ? " [CRC-FAILED]" : "");
    } else {
        printf("[%02X %02X] UBX-RXM-RTCM:\n", message_class(), message_id());
        printf("[.....]    version: %u\n", mPayload.version);
        printf("[.....]    (no data)\n");
    }
}

std::unique_ptr<Message> UbxRxmRtcm::parse(Decoder&               decoder,
                                           std::vector<uint8_t>&& raw_data) UBLOX_NOEXCEPT {
    if (decoder.remaining() < 2) {
        return nullptr;
    }

    auto version = decoder.U1();
    if (decoder.error()) {
        return nullptr;
    }

    raw::RxmRtcm payload;
    if (version == 2) {
        payload.version = version;

        auto flags                       = decoder.U1();
        payload.data.v2.flags.crc_failed = (flags >> 0) & 0x01;
        payload.data.v2.flags.msg_used   = (flags >> 1) & 0x03;

        payload.data.v2.sub_type    = decoder.U2();
        payload.data.v2.ref_station = decoder.U2();
        payload.data.v2.msg_type    = decoder.U2();
    } else {
        payload.version = version;
    }

    if (decoder.error()) {
        return nullptr;
    } else {
        return std::unique_ptr<Message>{new UbxRxmRtcm(std::move(payload), std::move(raw_data))};
    }
}

}  // namespace ublox
}  // namespace receiver
