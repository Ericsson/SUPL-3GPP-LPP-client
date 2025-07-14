#include "parser.hpp"
#include "1019.hpp"
#include "message.hpp"
#include "helper.hpp"

#include <cstdio>
#include <sstream>
#include <iomanip>


#include <loglet/loglet.hpp>

LOGLET_MODULE2(format, rtcm);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, rtcm)

namespace format {
namespace rtcm {

NODISCARD char const* Parser::name() const NOEXCEPT {
    return "RTCM";
}

/** RTCM3 transport layout
 * +--------+--------+---------+---------+----------------+---------+
 * |  0xd3  | 000000 | length  |  type   |    content     |   crc   |
 * +========+========+=========+=========+================+=========+
 * | 8 bits | 6 bits | 10 bits | 12 bits |    variable    | 24 bits |
 * +--------+--------+---------+---------+----------------+---------+
 * |                           |   payload; length x 8    |         |
 * +--------+--------+---------+---------+----------------+---------+ */
std::unique_ptr<Message> Parser::try_parse() NOEXCEPT {
    FUNCTION_SCOPE();
    // search for '0xD3'
    for (;;) {
        if (buffer_length() < 1) {
            VERBOSEF("not enough data to search for '0xD3'");
            return nullptr;
        }

        if (peek(0) == 0xD3) {
            VERBOSEF("found '0xD3'");
            break;
        }

        // skip one byte and try again
        skip(1u);
    }

    if (buffer_length() < 3) {
        VERBOSEF("not enough data to extract message length");
        return nullptr;
    }

    if ((peek(1) & 0xFC) != 0) {
        VERBOSEF("invalid padding bits: '%06b'", (peek(1) & 0xFC)>>2);
        return nullptr;
    }

    unsigned payload_length = (peek(1) & 0x03) << 8 | (peek(2));
    auto message_length = 1/*PREAMBLE*/ + 2/*LENGTH*/ + payload_length + 3/*CRC*/;

    // copy message to buffer
    std::vector<uint8_t> message;
    message.resize(message_length);
    copy_to_buffer(message.data(), message_length);

    // check crc
    auto result = crc(message);
    if (result != CRCResult::OK) {
        std::ostringstream oss;
        for (auto b : message)
            oss << std::hex << std::setw(2) << std::setfill('0') << b;
        DEBUGF("crc failed: \"%s\"", oss.str().c_str());
        return nullptr;
    }
    skip(message_length);

    DF002 type = (message[3] << 4) | (message[4] >> 4);

    switch (type) {
    case 1019: return RTCM1019Message::parse(message);
    default:   return std::make_unique<UnsupportedMessage>(type, message);
    }
}

CRCResult Parser::crc(std::vector<uint8_t> const& buffer) {
    FUNCTION_SCOPE();
    auto polynomial = 0x1864CFBu;
    auto crc = 0u;

    for (uint8_t b : buffer) {
        crc ^= b << 16;
        for (int i=0;i<8;i++) {
            crc <<= 1;
            if (crc & 0x01000000)
                crc ^= polynomial;
        }
    }
    return (crc & 0xFFFFFF) == 0 ? CRCResult::OK : CRCResult::INVALID_VALUE;
}

}  // namespace rtcm
}  // namespace format
