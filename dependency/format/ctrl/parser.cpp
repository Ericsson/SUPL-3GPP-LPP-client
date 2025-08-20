#include "parser.hpp"
#include "cid.hpp"
#include "helper.hpp"
#include "identity.hpp"
#include "message.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE(ctrl);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(ctrl)

namespace format {
namespace ctrl {

NODISCARD char const* Parser::name() const NOEXCEPT {
    return "CTRL";
}

std::unique_ptr<Message> Parser::try_parse() NOEXCEPT {
    FUNCTION_SCOPE();

    // search for '/'
    for (;;) {
        if (buffer_length() < 1) {
            // not enough data to search for '/'
            VERBOSEF("not enough data to search for '/'");
            return nullptr;
        }

        if (peek(0) == '/') {
            // found '/'
            VERBOSEF("found '/'");
            break;
        }

        // skip one byte and try again
        skip(1u);
    }

    // search for '\r\n'
    auto length = 1u;
    auto suffix = 0u;
    for (;;) {
        if (buffer_length() < length + 1) {
            // not enough data to search for '\r\n'
            VERBOSEF("not enough data to search for '\\r\\n' (available: %zu)", buffer_length());
            return nullptr;
        }

        if (peek(length + 0) == '\r' && peek(length + 1) == '\n') {
            // found '\r\n'
            VERBOSEF("found '\\r\\n'");
            suffix = 2;
            break;
        } else if (peek(length + 0) == '\n') {
            // found '\n' without '\r'
            VERBOSEF("found '\\n' without '\\r'");
            suffix = 1;
            break;
        }

        // skip one byte and try again
        length++;

        // check if message is too long
        if (length > 128) {
            VERBOSEF("message is too long");
            skip(length);
            return nullptr;
        }
    }

    // copy message to buffer
    std::string payload;
    payload.resize(length);
    copy_to_buffer(reinterpret_cast<uint8_t*>(&payload[0]), length);
    skip(length + suffix);

    VERBOSEF("parsed message: \"%s\"", payload.c_str());

    auto tokens = split(payload, ',');
    if (tokens.size() < 1) {
        return nullptr;
    }

    if (tokens[0] == "/CID") return parse_cid(payload, tokens);
    if (tokens[0] == "/IDENTITY") return parse_identity(payload, tokens);

    VERBOSEF("unknown command: \"%s\"", tokens[0].c_str());
    return nullptr;
}

std::unique_ptr<Message> Parser::parse_cid(std::string const&              message,
                                           std::vector<std::string> const& tokens) NOEXCEPT {
    FUNCTION_SCOPE();

    if (tokens.size() != 6) {
        WARNF("/CID requires 6 tokens, got %zu", tokens.size());
        return nullptr;
    }

    uint32_t mcc;
    uint32_t mnc;
    uint32_t tac;
    uint64_t cell;
    bool     is_nr;

    try {
        mcc   = static_cast<uint32_t>(std::stoul(tokens[2]));
        mnc   = static_cast<uint32_t>(std::stoul(tokens[3]));
        tac   = static_cast<uint32_t>(std::stoul(tokens[4]));
        cell  = std::stoull(tokens[5]);
        is_nr = tokens[1] == "N";
    } catch (std::exception const& e) {
        WARNF("/CID parsing failed: %s", e.what());
        WARNF("  mcc:  \"%s\"", tokens[2].c_str());
        WARNF("  mnc:  \"%s\"", tokens[3].c_str());
        WARNF("  tac:  \"%s\"", tokens[4].c_str());
        WARNF("  cell: \"%s\"", tokens[5].c_str());
        return nullptr;
    }

    return std::unique_ptr<Message>(new CellId(message, mcc, mnc, tac, cell, is_nr));
}

std::unique_ptr<Message> Parser::parse_identity(std::string const&              message,
                                                std::vector<std::string> const& tokens) NOEXCEPT {
    FUNCTION_SCOPE();

    if (tokens.size() < 2) {
        WARNF("/IDENTITY requires at least 2 tokens, got %zu", tokens.size());
        return nullptr;
    }

    if (tokens[1] == "IMSI") {
        if (tokens.size() != 3) {
            WARNF("/IDENTITY,IMSI requires 3 tokens, got %zu", tokens.size());
            return nullptr;
        }

        uint64_t imsi;
        try {
            imsi = std::stoull(tokens[2]);
        } catch (std::exception const& e) {
            WARNF("/IDENTITY,IMSI parsing failed: \"%s\" %s", tokens[2].c_str(), e.what());
            return nullptr;
        }

        return std::unique_ptr<Message>(new IdentityImsi(message, imsi));
    } else {
        WARNF("/IDENTITY unknown identity: %s", tokens[1].c_str());
        return nullptr;
    }
}

}  // namespace ctrl
}  // namespace format
