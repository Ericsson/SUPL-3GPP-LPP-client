#include "parser.hpp"
#include "gga.hpp"
#include "gst.hpp"
#include "message.hpp"
#include "vtg.hpp"

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

    // check checksum
    auto result = checksum(payload);
    if (result != ChecksumResult::OK) {
        // checksum failed
        return nullptr;
    }

    auto length_with_clrf = length + 2;
    auto prefix = parse_prefix(reinterpret_cast<const uint8_t*>(payload.data()), length_with_clrf);
    if (prefix.empty()) {
        // invalid prefix
        return nullptr;
    }

    // '$XXXXX,' [data] '*XY\r\n'
    auto data_start = prefix.size() + 1 /* $ */ + 1 /* , */;
    auto data_end   = length_with_clrf - 5;
    if (data_start >= data_end) {
        // no data
        return nullptr;
    }

    auto data_length   = data_end - data_start;
    auto data_payload  = payload.substr(data_start, data_length);
    auto data_checksum = payload.substr(data_end + 1, data_end + 3);

    // parse message
    if (prefix == "GPGGA" || prefix == "GLGGA" || prefix == "GAGGA" || prefix == "GNGGA") {
        auto message = GgaMessage::parse(prefix, data_payload, data_checksum);
        if (message) {
            return message;
        } else {
            return std::unique_ptr<ErrorMessage>(
                new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else if (prefix == "GPVTG" || prefix == "GLVTG" || prefix == "GAVTG" || prefix == "GNVTG") {
        auto message = VtgMessage::parse(prefix, data_payload, data_checksum);
        if (message) {
            return message;
        } else {
            return std::unique_ptr<ErrorMessage>(
                new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else if (prefix == "GPGST" || prefix == "GLGST" || prefix == "GAGST" || prefix == "GNGST") {
        auto message = GstMessage::parse(prefix, data_payload, data_checksum);
        if (message) {
            return message;
        } else {
            return std::unique_ptr<ErrorMessage>(
                new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else {
        return std::unique_ptr<UnsupportedMessage>(
            new UnsupportedMessage(prefix, data_payload, data_checksum));
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

ChecksumResult Parser::checksum(const std::string& buffer) {
    auto nmea_end = buffer.find_last_of('*');
    if (nmea_end == std::string::npos) {
        return ChecksumResult::INVALID_STRING_NOSTAR;
    }

    if (nmea_end + 3 /* *XY */ + 2 /* \r\n */ != buffer.size()) {
        return ChecksumResult::INVALID_STRING_LENGTH;
    }

    auto nmea_string = buffer.substr(1, nmea_end - 1);

    auto expected_checksum_hex = buffer.substr(nmea_end + 1, nmea_end + 3);
    auto expected_checksum     = std::stoull(std::string{expected_checksum_hex}, nullptr, 16);

    auto calculated_checksum = 0ULL;
    for (auto nmea_char : nmea_string) {
        calculated_checksum = calculated_checksum ^ static_cast<unsigned long>(nmea_char);
    }

    if (expected_checksum == calculated_checksum) {
        return ChecksumResult::OK;
    } else {
        return ChecksumResult::INVALID_VALUE;
    }
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

    if (prefix.size() != 6) {
        return "";
    }

    // remove $
    return prefix.substr(1);
}

}  // namespace nmea
}  // namespace receiver
