#include "parser.hpp"
#include "decoder.hpp"
#include "messages/ack_ack.hpp"
#include "messages/ack_nak.hpp"
#include "messages/cfg_valget.hpp"
#include "messages/mon_ver.hpp"
#include "messages/nav_pvt.hpp"
#include "messages/rxm_rawx.hpp"
#include "messages/rxm_rtcm.hpp"
#include "messages/rxm_sfrbx.hpp"
#include "messages/rxm_spartn.hpp"

#include <cstdio>

#include <loglet/loglet.hpp>

LOGLET_MODULE(ubx);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ubx)

namespace format {
namespace ubx {

char const* Parser::name() const NOEXCEPT {
    return "UBX";
}

std::unique_ptr<Message> Parser::try_parse() NOEXCEPT {
    // search for frame boundary
    bool did_skip_anything = false;
    for (;;) {
        if (buffer_length() < 8) {
            // not enough data to search for frame boundary
            TRACEF("not enough data to search for frame boundary: %u", buffer_length());
            return nullptr;
        }

        if (is_frame_boundary()) {
            if (did_skip_anything) {
                // found frame boundary (only print once)
                VERBOSEF("found frame boundary");
            }
            break;
        }

        // skip one byte and try again
        skip(1u);
        did_skip_anything = true;
    }

    // check that we have enough data for the header
    if (buffer_length() < 8) {
        // not enough data for header
        VERBOSEF("not enough data for header: %u", buffer_length());
        return nullptr;
    }

    // read header
    uint8_t buffer[8192];
    copy_to_buffer(buffer, 6u);

    Decoder header_decoder(buffer, 6);
    header_decoder.skip(2);  // skip frame boundary
    auto message_class = header_decoder.U1();
    auto message_id    = header_decoder.U1();
    auto length        = static_cast<uint32_t>(header_decoder.U2());

    auto type = (static_cast<uint16_t>(message_class) << 8) | static_cast<uint16_t>(message_id);
    if (length > 8192) {
        // invalid length
        skip(2u);
        VERBOSEF("invalid length");
        return nullptr;
    } else if (buffer_length() < length + 8) {
        // not enough data for payload
        TRACEF("not enough data for payload: %u of %u", buffer_length(), length + 8);
        return nullptr;
    }

    copy_to_buffer(buffer, length + 8);

    // check checksum
    auto calculated_checksum = checksum_message(buffer, length + 8);
    auto expected_checksum   = (static_cast<uint16_t>(buffer[length + 7]) << 8) |
                             static_cast<uint16_t>(buffer[length + 6]);
    if (calculated_checksum != expected_checksum) {
        // checksum failed
        skip(2u);
        VERBOSEF("checksum failed");
        return nullptr;
    }

    skip(length + 8);

    // parse payload
    Decoder              decoder(buffer + 6, length);
    std::vector<uint8_t> data(buffer, buffer + length + 8);

    switch (type) {
    case 0x0107: return UbxNavPvt::parse(decoder, std::move(data));
    case 0x0A04: return UbxMonVer::parse(decoder, std::move(data));
    case 0x068B: return UbxCfgValget::parse(decoder, std::move(data));
    case 0x0501: return UbxAckAck::parse(decoder, std::move(data));
    case 0x0500: return UbxAckNak::parse(decoder, std::move(data));
    case 0x0213: return RxmSfrbx::parse(decoder, std::move(data));
    case 0x0215: return UbxRxmRawx::parse(decoder, std::move(data));
    case 0x0232: return UbxRxmRtcm::parse(decoder, std::move(data));
    case 0x0233: return UbxRxmSpartn::parse(decoder, std::move(data));
    default:
        return std::unique_ptr<Message>{
            new UnsupportedMessage(message_class, message_id, std::move(data))};
    }
}

bool Parser::is_frame_boundary() const NOEXCEPT {
    if (buffer_length() < 2) {
        return false;
    } else if (peek(0) != 0xB5) {
        return false;
    } else if (peek(1) != 0x62) {
        return false;
    } else {
        return true;
    }
}

uint16_t Parser::checksum_message(uint8_t* message_data, uint32_t message_length) {
    return checksum(message_data + 2, message_length - 4);
}

uint16_t Parser::checksum(uint8_t* payload, uint32_t length) {
    ASSERT(length <= 0xFFFF, "length must be less than 0xFFFF");

    uint8_t CK_A = 0, CK_B = 0;
    for (uint16_t I = 0; I < static_cast<uint16_t>(length); I++) {
        CK_A = CK_A + payload[I];
        CK_B = CK_B + CK_A;
    }

    return static_cast<uint16_t>((CK_B << 8U) | CK_A);
}

}  // namespace ubx
}  // namespace format
