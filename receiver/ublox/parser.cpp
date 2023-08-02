#include "parser.hpp"
#include <cstdio>
#include "decoder.hpp"
#include "ubx_ack_ack.hpp"
#include "ubx_ack_nak.hpp"
#include "ubx_cfg_valget.hpp"
#include "ubx_mon_ver.hpp"
#include "ubx_nav_pvt.hpp"

namespace receiver {
namespace ublox {

static UBLOX_CONSTEXPR uint32_t UBLOX_PARSER_BUFFER_SIZE = 16 * 1024;

Parser::Parser() UBLOX_NOEXCEPT : mBuffer(nullptr),
                                  mBufferCapacity(0),
                                  mBufferRead(0),
                                  mBufferWrite(0) {
    mBuffer         = new uint8_t[UBLOX_PARSER_BUFFER_SIZE];
    mBufferCapacity = UBLOX_PARSER_BUFFER_SIZE;
}

Parser::~Parser() UBLOX_NOEXCEPT {
    if (mBuffer != nullptr) {
        delete[] mBuffer;
    }
}

bool Parser::append(uint8_t* data, uint16_t length) UBLOX_NOEXCEPT {
    if (length > mBufferCapacity) {
        // TODO(ewasjon): report error
        return false;
    }

    // copy data to buffer
    for (uint16_t i = 0; i < length; i++) {
        mBuffer[mBufferWrite] = data[i];
        mBufferWrite          = (mBufferWrite + 1) % mBufferCapacity;
        if (mBufferWrite == mBufferRead) {
            // buffer overflow
            mBufferRead = (mBufferRead + 1) % mBufferCapacity;
        }
    }

    return true;
}

void Parser::clear() UBLOX_NOEXCEPT {
    mBufferRead  = 0;
    mBufferWrite = 0;
}

std::unique_ptr<Message> Parser::try_parse() UBLOX_NOEXCEPT {
    // search for frame boundary
    for (;;) {
        if (buffer_length() < 8) {
            // not enough data to search for frame boundary
            return nullptr;
        }

        if (is_frame_boundary()) {
            // found frame boundary
            break;
        }

        // skip one byte and try again
        skip(1);
    }

    // check that we have enough data for the header
    if (buffer_length() < 8) {
        // not enough data for header
        return nullptr;
    }

    // read header
    uint8_t buffer[8192];
    copy_to_buffer(buffer, 6);

    Decoder header_decoder(buffer, 6);
    header_decoder.skip(2);  // skip frame boundary
    auto message_class = header_decoder.U1();
    auto message_id    = header_decoder.U1();
    auto length        = static_cast<uint32_t>(header_decoder.U2());

    auto type = (static_cast<uint16_t>(message_class) << 8) | static_cast<uint16_t>(message_id);
    if (length > 8192) {
        // invalid length
        skip(2);
        return nullptr;
    } else if (buffer_length() < length + 8) {
        // not enough data for payload
        return nullptr;
    }

    copy_to_buffer(buffer, length + 8);
    skip(length + 8);

    // check checksum
    auto calculated_checksum = checksum_message(buffer, length + 8);
    auto expected_checksum   = (static_cast<uint16_t>(buffer[length + 7]) << 8) |
                             static_cast<uint16_t>(buffer[length + 6]);
    if (calculated_checksum != expected_checksum) {
        // checksum failed
        skip(length + 8);
        return nullptr;
    }

    // parse payload
    Decoder decoder(buffer + 6, length);

    switch (type) {
    case 0x0107: return UbxNavPvt::parse(decoder);
    case 0x0A04: return UbxMonVer::parse(decoder);
    case 0x068B: return UbxCfgValget::parse(decoder);
    case 0x0501: return UbxAckAck::parse(decoder);
    case 0x0500: return UbxAckNak::parse(decoder);
    default: return std::unique_ptr<Message>{new UnsupportedMessage(message_class, message_id)};
    }
}

uint32_t Parser::buffer_length() const UBLOX_NOEXCEPT {
    if (mBufferWrite >= mBufferRead) {
        return mBufferWrite - mBufferRead;
    } else {
        return mBufferCapacity - mBufferRead + mBufferWrite;
    }
}

bool Parser::is_frame_boundary() const UBLOX_NOEXCEPT {
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

uint8_t Parser::peek(uint32_t index) const UBLOX_NOEXCEPT {
    if (index >= buffer_length()) {
        // NOTE(ewasjon): the caller should check buffer_length() before calling peek
        return 0;
    }

    return mBuffer[(mBufferRead + index) % mBufferCapacity];
}

void Parser::skip(uint32_t length) UBLOX_NOEXCEPT {
    auto available = buffer_length();
    if (length > available) {
        length = available;
    }

    mBufferRead = (mBufferRead + length) % mBufferCapacity;
}

void Parser::copy_to_buffer(uint8_t* data, uint32_t length) UBLOX_NOEXCEPT {
    auto available = buffer_length();
    if (length > available) {
        length = available;
    }

    for (uint32_t i = 0; i < length; i++) {
        data[i] = mBuffer[(mBufferRead + i) % mBufferCapacity];
    }
}

uint16_t Parser::checksum_message(uint8_t* message_data, uint16_t message_length) {
    return checksum(message_data + 2, message_length - 4);
}

uint16_t Parser::checksum(uint8_t* payload, uint16_t length) {
    uint8_t CK_A = 0, CK_B = 0;
    for (uint16_t I = 0; I < length; I++) {
        CK_A = CK_A + payload[I];
        CK_B = CK_B + CK_A;
    }

    return (static_cast<uint16_t>(CK_B) << 8U) | static_cast<uint16_t>(CK_A);
}

}  // namespace ublox
}  // namespace receiver
