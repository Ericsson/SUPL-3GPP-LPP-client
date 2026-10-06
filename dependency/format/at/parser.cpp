#include "parser.hpp"

#include <loglet/loglet.hpp>

LOGLET_MODULE(at);
#undef LOGLET_CURRENT_MODULE
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
        static std::string const sEmpty;
        return sEmpty;
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

// Upper bound to guarantee forward progress when a line never terminates.
static CONSTEXPR uint32_t MAX_LINE_LENGTH = 120;

void Parser::process() NOEXCEPT {
    FUNCTION_SCOPE();

    while (buffer_length() > 0) {
        VERBOSEF("trying to parse next line");

        std::string line;
        auto        result = process_line(line);
        if (result == LineResult::NeedMoreData) {
            break;
        } else if (result == LineResult::Discarded) {
            continue;
        }

        mLines.push_back(std::move(line));
    }
}

Parser::LineResult Parser::process_line(std::string& line) NOEXCEPT {
    FUNCTION_SCOPE();

    line.clear();

    auto index = 0u;
    for (;;) {
        if (buffer_length() < index + 1) {
            // not enough data to find <CR><LF>
            VERBOSEF("not enough data to find <CR><LF>");
            return LineResult::NeedMoreData;
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

        if (index > MAX_LINE_LENGTH) {
            // message is too long
            VERBOSEF("message is too long");
            record_frame_error();
            discard(index);
            return LineResult::Discarded;
        }

        auto ch = static_cast<char>(peek(index));
        line.push_back(ch);
        index++;
    }

    skip(index + 2);
    return LineResult::Ok;
}

}  // namespace at
}  // namespace format
