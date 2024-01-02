#include "parser.hpp"
#include "gpgga.hpp"
#include "message.hpp"

#include <cstdio>

namespace receiver {
namespace nmea {

static NMEA_CONSTEXPR uint32_t NMEA_PARSER_BUFFER_SIZE = 16 * 1024;

Parser::Parser() NMEA_NOEXCEPT : mBuffer(nullptr),
                                 mBufferCapacity(0),
                                 mBufferRead(0),
                                 mBufferWrite(0) {
    mBuffer         = new uint8_t[NMEA_PARSER_BUFFER_SIZE];
    mBufferCapacity = NMEA_PARSER_BUFFER_SIZE;
}

Parser::~Parser() NMEA_NOEXCEPT {
    if (mBuffer != nullptr) {
        delete[] mBuffer;
    }
}

bool Parser::append(uint8_t* data, uint16_t length) NMEA_NOEXCEPT {
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

void Parser::clear() NMEA_NOEXCEPT {
    mBufferRead  = 0;
    mBufferWrite = 0;
}

std::unique_ptr<Message> Parser::try_parse() NMEA_NOEXCEPT {
    // search for '$'
    for (;;) {
        if (buffer_length() < 1) {
            // not enough data to search for '$'
            return nullptr;
        }

        if (peek(0) == '$') {
            // found '$'
            break;
        }

        // skip one byte and try again
        skip(1);
    }

    // search for '\r\n'
    auto length = 1u;
    for (;;) {
        if (buffer_length() < length + 2) {
            // not enough data to search for '\r\n'
            return nullptr;
        }

        if (peek(length + 0) == '\r' && peek(length + 1) == '\n') {
            // found '\r\n'
            break;
        }

        // skip one byte and try again
        length++;
    }

    // copy message to buffer
    std::string payload;
    payload.resize(length + 2);
    copy_to_buffer(reinterpret_cast<uint8_t*>(&payload[0]), length + 2);
    skip(length + 2);

    // parse message
    auto prefix = parse_prefix(reinterpret_cast<const uint8_t*>(payload.data()), length);
    if (prefix == "$GPGGA") {
        return GpggaMessage::parse(payload);
    } else {
        return std::unique_ptr<UnsupportedMessage>(new UnsupportedMessage(prefix, payload));
    }
}

uint32_t Parser::buffer_length() const NMEA_NOEXCEPT {
    if (mBufferWrite >= mBufferRead) {
        return mBufferWrite - mBufferRead;
    } else {
        return mBufferCapacity - mBufferRead + mBufferWrite;
    }
}

uint32_t Parser::available_space() const NMEA_NOEXCEPT {
    return mBufferCapacity - buffer_length() - 1;
}

uint8_t Parser::peek(uint32_t index) const NMEA_NOEXCEPT {
    if (index >= buffer_length()) {
        // NOTE(ewasjon): the caller should check buffer_length() before calling peek
        return 0;
    }

    return mBuffer[(mBufferRead + index) % mBufferCapacity];
}

void Parser::skip(uint32_t length) NMEA_NOEXCEPT {
    auto available = buffer_length();
    if (length > available) {
        length = available;
    }

    mBufferRead = (mBufferRead + length) % mBufferCapacity;
}

void Parser::copy_to_buffer(uint8_t* data, uint32_t length) NMEA_NOEXCEPT {
    auto available = buffer_length();
    if (length > available) {
        length = available;
    }

    for (uint32_t i = 0; i < length; i++) {
        data[i] = mBuffer[(mBufferRead + i) % mBufferCapacity];
    }
}

uint16_t Parser::checksum_message(const uint8_t* message_data, uint16_t message_length) {
    return checksum(message_data + 2, message_length - 4);
}

uint16_t Parser::checksum(const uint8_t* payload, uint16_t length) {
    uint8_t CK_A = 0, CK_B = 0;
    for (uint16_t I = 0; I < length; I++) {
        CK_A = CK_A + payload[I];
        CK_B = CK_B + CK_A;
    }

    return (static_cast<uint16_t>(CK_B) << 8U) | static_cast<uint16_t>(CK_A);
}

std::string Parser::parse_prefix(const uint8_t* data, uint32_t length) const NMEA_NOEXCEPT {
    // parse '$XXXXX' until first ',' or '*'
    std::string prefix;
    for (uint32_t i = 0; i < length; i++) {
        auto c = data[i];
        if (c == ',' || c == '*') {
            break;
        }

        prefix += c;
    }

    return prefix;
}

}  // namespace nmea
}  // namespace receiver
