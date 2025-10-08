#include "parser.hpp"
#include "epe.hpp"
#include "gga.hpp"
#include "gst.hpp"
#include "message.hpp"
#include "vtg.hpp"

#include <cstdio>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(format, nmea);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, nmea)

namespace format {
namespace nmea {

NODISCARD char const* Parser::name() const NOEXCEPT {
    return "NMEA";
}

std::unique_ptr<Message> Parser::try_parse() NOEXCEPT {
    FUNCTION_SCOPE();

    // search for '$'
    for (;;) {
        if (buffer_length() < 1) {
            VERBOSEF("not enough data to search for '$'");
            return nullptr;
        }

        if (peek(0) == '$') {
            VERBOSEF("found '$'");
            break;
        }

        // skip one byte and try again
        skip(1u);
    }

    auto length             = 1u;
    auto line_ending_length = mLfOnly ? 1u : 2u;

    for (;;) {
        if (buffer_length() < length + line_ending_length) {
            VERBOSEF("not enough data to search for line ending");
            return nullptr;
        }

        bool found_ending = false;
        if (mLfOnly) {
            if (peek(length) == '\n') {
                VERBOSEF("found '\\n'");
                found_ending = true;
            }
        } else {
            if (peek(length) == '\r' && peek(length + 1) == '\n') {
                VERBOSEF("found '\\r\\n'");
                found_ending = true;
            }
        }

        if (found_ending) break;

        if (peek(length) == '$') {
            VERBOSEF("found '$' while looking for line ending");
            skip(length);
            return nullptr;
        }

        length++;
    }

    std::string payload;
    payload.resize(length + line_ending_length);
    copy_to_buffer(reinterpret_cast<uint8_t*>(&payload[0]), length + line_ending_length);

    auto result = checksum(payload);
    if (result != ChecksumResult::OK) {
        DEBUGF("checksum failed: \"%s\"", payload.c_str());
        skip(1u);
        return nullptr;
    }
    skip(length + line_ending_length);

    auto length_with_clrf = length + line_ending_length;
    auto prefix = parse_prefix(reinterpret_cast<uint8_t const*>(payload.data()), length_with_clrf);
    if (prefix.empty()) {
        // invalid prefix
        VERBOSEF("invalid prefix");
        return nullptr;
    }

    // '$XXXXX,' [data] '*XY\r\n'
    auto data_start = prefix.size() + 1 /* $ */ + 1 /* , */;
    auto data_end   = length_with_clrf - 5;
    if (data_start >= data_end) {
        // no data
        VERBOSEF("no data");
        return nullptr;
    }

    auto data_length   = data_end - data_start;
    auto data_payload  = payload.substr(data_start, data_length);
    auto data_checksum = payload.substr(data_end + 1, data_end + 3);
    DEBUGF("prefix: %s, data: %s", prefix.c_str(), data_payload.c_str());

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
    } else if (prefix == "PQTMEPE") {
        auto message = EpeMessage::parse(prefix, data_payload, data_checksum);
        if (message) {
            return message;
        } else {
            return std::unique_ptr<ErrorMessage>(
                new ErrorMessage(prefix, data_payload, data_checksum));
        }
    } else {
        return std::unique_ptr<Message>(
            new UnsupportedMessage(prefix, data_payload, data_checksum));
    }
}

ChecksumResult Parser::checksum(std::string const& buffer) const {
    FUNCTION_SCOPE();

    auto fmt_end = buffer.find_last_of('*');
    if (fmt_end == std::string::npos) {
        DEBUGF("invalid string: no *");
        return ChecksumResult::INVALID_STRING_NOSTAR;
    }

    auto expected_length = fmt_end + 3 /* *XY */ + (mLfOnly ? 1 : 2);
    if (expected_length != buffer.size()) {
        DEBUGF("invalid string: length");
        return ChecksumResult::INVALID_STRING_LENGTH;
    }

    auto fmt_string = buffer.substr(1, fmt_end - 1);

    try {
        auto expected_checksum_hex = buffer.substr(fmt_end + 1, fmt_end + 3);
        auto expected_checksum     = std::stoull(std::string{expected_checksum_hex}, nullptr, 16);

        auto calculated_checksum = 0ULL;
        for (auto fmt_char : fmt_string) {
            calculated_checksum = calculated_checksum ^ static_cast<unsigned long long>(fmt_char);
        }

        if (expected_checksum == calculated_checksum) {
            return ChecksumResult::OK;
        } else {
            DEBUGF("invalid value: %02X != %02X", expected_checksum, calculated_checksum);
            return ChecksumResult::INVALID_VALUE;
        }
    } catch (...) {
        DEBUGF("invalid value: %s", fmt_string.c_str());
        return ChecksumResult::INVALID_VALUE;
    }
}

std::string Parser::parse_prefix(uint8_t const* data, uint32_t length) const NOEXCEPT {
    FUNCTION_SCOPE();
    // parse '$XXXXX' until first ',' or '*'
    std::string prefix;
    for (uint32_t i = 0; i < length; i++) {
        auto c = data[i];
        if (c == ',' || c == '*') {
            break;
        }

        prefix += static_cast<char>(c);
    }

    if (prefix.size() != 6 && prefix.size() != 8) {
        DEBUGF("invalid prefix: %s", prefix.c_str());
        // All NMEA messages have six characters in their prefix, eg $GPGST, except
        // Quectel's EPE message, that has eight characters: $PQTMEPE.
        return "";
    }

    // remove $
    return prefix.substr(1);
}

}  // namespace nmea
}  // namespace format
