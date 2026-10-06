#pragma once
#include <core/core.hpp>

#include <memory>

namespace format {
namespace helper {

/// Base class for streaming format parsers.
///
/// `try_parse()` contract that all derived parsers must uphold:
///   - Returning a message means progress was made; the caller must call again.
///   - Returning nothing (nullptr) means no complete message can be produced from the data
///     currently buffered, and *nothing was consumed*. The caller may stop until more data
///     arrives.
///
/// A parser must never consume bytes and then report "nothing". Callers drain with
/// `while (auto msg = try_parse()) { ... }`, so doing so aborts the drain and leaves the
/// remaining buffered bytes unparsed. On interleaved streams (RTCM + NMEA + filler on one
/// serial line) that causes the buffer to back up until it overflows and data is lost.
/// Use `discard()` to step over bytes that cannot start a message and keep looping instead.
class Parser {
public:
    EXPLICIT Parser() NOEXCEPT;
    virtual ~Parser() NOEXCEPT;

    bool append(uint8_t const* data, size_t length) NOEXCEPT;
    void clear() NOEXCEPT;

    NODISCARD virtual char const* name() const NOEXCEPT = 0;

    NODISCARD uint32_t buffer_length() const NOEXCEPT;
    NODISCARD uint32_t available_space() const NOEXCEPT;

    /// Bytes stepped over because they could not start a message. Expected to be non-zero on
    /// interleaved streams.
    NODISCARD uint64_t discarded_bytes() const NOEXCEPT { return mDiscardedBytes; }
    /// Messages that had a valid-looking header but failed validation (crc/checksum/length).
    /// A high rate indicates a lossy link rather than interleaving.
    NODISCARD uint64_t frame_errors() const NOEXCEPT { return mFrameErrors; }
    /// Number of `append()` calls that did not fit and overwrote unparsed data.
    NODISCARD uint64_t overflow_events() const NOEXCEPT { return mOverflowEvents; }
    /// Total unparsed bytes destroyed by overflow.
    NODISCARD uint64_t overflow_bytes() const NOEXCEPT { return mOverflowBytes; }

protected:
    NODISCARD uint8_t peek(uint32_t index) const NOEXCEPT;
    void              skip(uint32_t length) NOEXCEPT;
    void              skip(uint64_t length) NOEXCEPT { skip(static_cast<uint32_t>(length)); }

    /// Skip bytes that cannot start a message, and account for them. Use this instead of
    /// `skip()` when resynchronizing.
    void discard(uint32_t length) NOEXCEPT;
    /// Record that a framed message failed validation. Does not consume anything.
    void record_frame_error() NOEXCEPT;

    void copy_to_buffer(uint8_t* data, size_t length) NOEXCEPT;

private:
    uint8_t* mBuffer;
    uint32_t mBufferCapacity;
    uint32_t mBufferRead;
    uint32_t mBufferWrite;

    uint64_t mDiscardedBytes;
    uint64_t mFrameErrors;
    uint64_t mOverflowEvents;
    uint64_t mOverflowBytes;
};

}  // namespace helper
}  // namespace format
