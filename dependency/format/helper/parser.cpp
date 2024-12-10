#include "parser.hpp"

namespace format {
namespace helper {

static CONSTEXPR uint32_t PARSER_BUFFER_SIZE = 32 * 4096;

Parser::Parser() NOEXCEPT : mBuffer(nullptr),
                               mBufferCapacity(0),
                               mBufferRead(0),
                               mBufferWrite(0) {
    mBuffer         = new uint8_t[PARSER_BUFFER_SIZE];
    mBufferCapacity = PARSER_BUFFER_SIZE;
}

Parser::~Parser() NOEXCEPT {
    if (mBuffer != nullptr) {
        delete[] mBuffer;
    }
}

bool Parser::append(uint8_t* data, uint16_t length) NOEXCEPT {
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

void Parser::clear() NOEXCEPT {
    mBufferRead  = 0;
    mBufferWrite = 0;
}

uint32_t Parser::buffer_length() const NOEXCEPT {
    if (mBufferWrite >= mBufferRead) {
        return mBufferWrite - mBufferRead;
    } else {
        return mBufferCapacity - mBufferRead + mBufferWrite;
    }
}

uint32_t Parser::available_space() const NOEXCEPT {
    return mBufferCapacity - buffer_length() - 1;
}

uint8_t Parser::peek(uint32_t index) const NOEXCEPT {
    if (index >= buffer_length()) {
        // NOTE(ewasjon): the caller should check buffer_length() before calling peek
        return 0;
    }

    return mBuffer[(mBufferRead + index) % mBufferCapacity];
}

void Parser::skip(uint32_t length) NOEXCEPT {
    auto available = buffer_length();
    if (length > available) {
        length = available;
    }

    mBufferRead = (mBufferRead + length) % mBufferCapacity;
}

void Parser::copy_to_buffer(uint8_t* data, uint32_t length) NOEXCEPT {
    auto available = buffer_length();
    if (length > available) {
        length = available;
    }

    for (uint32_t i = 0; i < length; i++) {
        data[i] = mBuffer[(mBufferRead + i) % mBufferCapacity];
    }
}

}  // namespace helper
}  // namespace format
