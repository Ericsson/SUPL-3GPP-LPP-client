#include "parser.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE(at);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF(at)

namespace format {
namespace at {

NODISCARD char const* Parser::name() const NOEXCEPT {
    return "AT";
}

std::string const& Parser::peek_line() const NOEXCEPT {
    if (mLines.empty()) {
        // no lines to peek
        VERBOSEF("no lines to peek");
        static std::string empty;
        return empty;
    }

    return mLines[0];
}

std::string Parser::skip_line() NOEXCEPT {
    FUNCTION_SCOPE();

    if (mLines.empty()) {
        // no lines to skip
        VERBOSEF("no lines to skip");
        return {};
    }

    auto line = std::move(mLines[0]);
    mLines.erase(mLines.begin());
    return line;
}

void Parser::process() NOEXCEPT {
    FUNCTION_SCOPE();

    while (buffer_length() > 0) {
        VERBOSEF("trying to parse next line");

        std::string line;
        if (!process_line(line)) {
            break;
        }

        mLines.push_back(std::move(line));
    }
}

bool Parser::process_line(std::string& line) NOEXCEPT {
    FUNCTION_SCOPE();

    line.clear();

    auto index = 0u;
    for (;;) {
        if (buffer_length() < index + 1) {
            // not enough data to find <CR><LF>
            VERBOSEF("not enough data to find <CR><LF>");
            return false;
        }

        if (peek(index + 0) == '\r') {
            VERBOSEF("ch: <CR>");
        } else if (peek(index + 0) == '\n') {
            VERBOSEF("ch: <LF>");
        } else if (isprint(peek(index + 0))) {
            VERBOSEF("ch: %c", peek(index + 0));
        } else {
            VERBOSEF("ch: <%02X>", peek(index + 0));
        }

        if (peek(index + 0) == '\r' && peek(index + 1) == '\n') {
            // found <CR><LF>
            VERBOSEF("found <CR><LF>");
            break;
        }

        if (index > 120) {
            // message is too long
            VERBOSEF("message is too long");
            skip(index);
            return false;
        }

        auto ch = static_cast<char>(peek(index));
        line.push_back(ch);
        index++;
    }

    skip(index + 2);
    return true;
}

}  // namespace at
}  // namespace format
