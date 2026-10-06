#include "parser.hpp"

#include <cinttypes>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(format, helper);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(format, helper)

namespace format {
namespace helper {

static CONSTEXPR uint32_t PARSER_BUFFER_SIZE = 32 * 4096;

// Overflow is always reported, but with an exponential backoff so a persistently overrunning
// link gives early visibility without drowning the log.
static bool should_report_overflow(uint64_t events) NOEXCEPT {
    return (events & (events - 1)) == 0;  // powers of two: 1, 2, 4, 8, ...
}

// Discards and frame errors are expected on interleaved streams, so they are only reported at
// verbose level and only once per interval.
static CONSTEXPR uint64_t DISCARD_REPORT_INTERVAL     = 64 * 1024;
static CONSTEXPR uint64_t FRAME_ERROR_REPORT_INTERVAL = 256;

Parser::Parser() NOEXCEPT : mBuffer(nullptr),
                            mBufferCapacity(0),
                            mBufferRead(0),
                            mBufferWrite(0),
                            mDiscardedBytes(0),
                            mFrameErrors(0),
                            mOverflowEvents(0),
                            mOverflowBytes(0) {
    FUNCTION_SCOPE();
    mBuffer         = new uint8_t[PARSER_BUFFER_SIZE];
    mBufferCapacity = PARSER_BUFFER_SIZE;
}

Parser::~Parser() NOEXCEPT {
    FUNCTION_SCOPE();
    if (mBuffer != nullptr) {
        delete[] mBuffer;
    }
}

bool Parser::append(uint8_t const* data, size_t length) NOEXCEPT {
    FUNCTION_SCOPEF("%u bytes", length);
    auto length32 = static_cast<uint32_t>(length);
    if (length32 > mBufferCapacity) {
        // TODO(ewasjon): report error
        DEBUGF("buffer capacity exceeded: %u > %u", length32, mBufferCapacity);
        return false;
    }

    auto space = available_space();
    if (length32 > space) {
        // The ring buffer is full: the copy below will advance the read pointer and destroy
        // unparsed data. This means messages are lost and the parser will land mid-message and
        // have to resynchronize. It should never happen in healthy operation, so report it.
        auto dropped = length32 - space;
        mOverflowEvents++;
        mOverflowBytes += dropped;
        if (should_report_overflow(mOverflowEvents)) {
            WARNF("%s parser buffer overflow: overwrote %u unparsed bytes (%" PRIu64
                  " events, %" PRIu64 " bytes total, capacity %u) - messages are being lost",
                  name(), dropped, mOverflowEvents, mOverflowBytes, mBufferCapacity);
        }
    }

    // copy data to buffer
    for (uint32_t i = 0; i < length32; i++) {
        mBuffer[mBufferWrite] = data[i];
        mBufferWrite          = (mBufferWrite + 1) % mBufferCapacity;
        if (mBufferWrite == mBufferRead) {
            // buffer overflow
            mBufferRead = (mBufferRead + 1) % mBufferCapacity;
        }
    }

    VERBOSEF("appended %u bytes", length32);
    return true;
}

void Parser::clear() NOEXCEPT {
    FUNCTION_SCOPE();
    mBufferRead  = 0;
    mBufferWrite = 0;
}

void Parser::discard(uint32_t length) NOEXCEPT {
    if (length == 0) return;

    auto previous = mDiscardedBytes;
    skip(length);
    mDiscardedBytes += length;

    if (previous / DISCARD_REPORT_INTERVAL != mDiscardedBytes / DISCARD_REPORT_INTERVAL) {
        VERBOSEF("%s parser discarded %" PRIu64 " bytes in total (resynchronizing)", name(),
                 mDiscardedBytes);
    }
}

void Parser::record_frame_error() NOEXCEPT {
    mFrameErrors++;
    if ((mFrameErrors % FRAME_ERROR_REPORT_INTERVAL) == 0) {
        VERBOSEF("%s parser rejected %" PRIu64 " frames in total (crc/length/checksum)", name(),
                 mFrameErrors);
    }
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

void Parser::copy_to_buffer(uint8_t* data, size_t length) NOEXCEPT {
    FUNCTION_SCOPEF("%u bytes", length);
    auto length32  = static_cast<uint32_t>(length);
    auto available = buffer_length();
    if (length32 > available) {
        length32 = available;
    }

    for (uint32_t i = 0; i < length32; i++) {
        data[i] = mBuffer[(mBufferRead + i) % mBufferCapacity];
    }
}

}  // namespace helper
}  // namespace format
