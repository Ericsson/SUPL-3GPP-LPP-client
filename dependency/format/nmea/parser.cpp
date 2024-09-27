#include "parser.hpp"
#include "gga.hpp"
#include "gst.hpp"
#include "message.hpp"
#include "vtg.hpp"

#include <cstdio>

namespace format {
namespace nmea {

NODISCARD char const* Parser::name() const NOEXCEPT {
    return "NMEA";
}

std::unique_ptr<Message> Parser::try_parse() NOEXCEPT {
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
    auto prefix = parse_prefix(reinterpret_cast<uint8_t const*>(payload.data()), length_with_clrf);
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
            return std::unique_ptr<Message>(new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else if (prefix == "GPVTG" || prefix == "GLVTG" || prefix == "GAVTG" || prefix == "GNVTG") {
        auto message = VtgMessage::parse(prefix, data_payload, data_checksum);
        if (message) {
            return message;
        } else {
            return std::unique_ptr<Message>(new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else if (prefix == "GPGST" || prefix == "GLGST" || prefix == "GAGST" || prefix == "GNGST") {
        auto message = GstMessage::parse(prefix, data_payload, data_checksum);
        if (message) {
            return message;
        } else {
            return std::unique_ptr<Message>(new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else {
        return std::unique_ptr<Message>(
            new UnsupportedMessage(prefix, data_payload, data_checksum));
    }
}

ChecksumResult Parser::checksum(std::string const& buffer) {
    auto fmt_end = buffer.find_last_of('*');
    if (fmt_end == std::string::npos) {
        return ChecksumResult::INVALID_STRING_NOSTAR;
    }

    if (fmt_end + 3 /* *XY */ + 2 /* \r\n */ != buffer.size()) {
        return ChecksumResult::INVALID_STRING_LENGTH;
    }

    auto fmt_string = buffer.substr(1, fmt_end - 1);

    auto expected_checksum_hex = buffer.substr(fmt_end + 1, fmt_end + 3);
    auto expected_checksum     = std::stoull(std::string{expected_checksum_hex}, nullptr, 16);

    auto calculated_checksum = 0ULL;
    for (auto fmt_char : fmt_string) {
        calculated_checksum = calculated_checksum ^ static_cast<unsigned long long>(fmt_char);
    }

    if (expected_checksum == calculated_checksum) {
        return ChecksumResult::OK;
    } else {
        return ChecksumResult::INVALID_VALUE;
    }
}

std::string Parser::parse_prefix(uint8_t const* data, uint32_t length) const NOEXCEPT {
    // parse '$XXXXX' until first ',' or '*'
    std::string prefix;
    for (uint32_t i = 0; i < length; i++) {
        auto c = data[i];
        if (c == ',' || c == '*') {
            break;
        }

        prefix += static_cast<char>(c);
    }

    if (prefix.size() != 6) {
        return "";
    }

    // remove $
    return prefix.substr(1);
}

}  // namespace nmea
}  // namespace receiver
