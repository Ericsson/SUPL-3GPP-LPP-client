#include "parser.hpp"
#include "decoder.hpp"
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

    return true;
}

Message* Parser::try_parse() UBLOX_NOEXCEPT {
    // search for frame boundary
    while (!is_frame_boundary()) {
        if (buffer_length() < 2) {
            // not enough data to search for frame boundary
            return nullptr;
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
    auto message_class = peek(2);
    auto message_id    = peek(3);
    auto type          = message_class << 8 | message_id;
    auto length        = peek(4) << 8 | peek(5);

    if (length > 8192) {
        // invalid length
        skip(2);
        return nullptr;
    } else if (buffer_length() < length + 8) {
        // not enough data for payload
        return nullptr;
    }

    uint8_t buffer[8192];
    copy_to_buffer(buffer, length + 8);

    // check checksum
    auto calculated_checksum = checksum(buffer, length + 8);
    auto expected_checksum   = buffer[length + 6] << 8 | buffer[length + 7];
    if (calculated_checksum != expected_checksum) {
        // checksum failed
        skip(length + 8);
        return nullptr;
    }

    // parse payload
    Decoder decoder(buffer + 4, length - 4);

    switch (type) {
    case 0x0107: return UbxNavPvt::parse(decoder);
    default: return new UnsupportedMessage(message_class, message_id);
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

    mBufferRead = (mBufferRead + length) % mBufferCapacity;
}

uint16_t Parser::checksum(uint8_t* payload, uint16_t length) const UBLOX_NOEXCEPT {
    uint8_t CK_A = 0, CK_B = 0;
    for (uint16_t I = 0; I < length - 4; I++) {
        CK_A = CK_A + payload[2 + I];
        CK_B = CK_B + CK_A;
    }

    return (CK_A << 8) | CK_B;
}

}  // namespace ublox
}  // namespace receiver
