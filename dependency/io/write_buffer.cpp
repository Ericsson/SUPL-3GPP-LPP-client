#include <io/write_buffer.hpp>

#include <loglet/loglet.hpp>

LOGLET_MODULE2(io, write_buffer);
#undef LOGLET_CURRENT_MODULE
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, write_buffer)

namespace io {

WriteBuffer::WriteBuffer(size_t max_size) NOEXCEPT : mReadPos(0), mMaxSize(max_size) {
    VSCOPE_FUNCTIONF("max_size=%zu", max_size);
}

void WriteBuffer::enqueue(uint8_t const* data, size_t length) NOEXCEPT {
    TRACEF("%p, %zu", data, length);

    // Compact buffer if read position is at the end
    if (mReadPos > 0 && mReadPos == mBuffer.size()) {
        mBuffer.clear();
        mReadPos = 0;
    }

    // If data is larger than max size, only keep the tail
    if (length > mMaxSize) {
        WARNF("data larger than max size, truncating: length=%zu, max=%zu", length, mMaxSize);
        data += length - mMaxSize;
        length = mMaxSize;
        mBuffer.clear();
        mReadPos = 0;
    }

    // Discard old data if needed to make room
    size_t available = mMaxSize - size();
    if (length > available) {
        size_t discard = length - available;
        WARNF("buffer full, discarding %zu bytes", discard);
        mReadPos += discard;
        if (mReadPos >= mBuffer.size()) {
            mBuffer.clear();
            mReadPos = 0;
        }
    }

    mBuffer.insert(mBuffer.end(), data, data + length);
    VERBOSEF("enqueued %zu bytes, total=%zu", length, size());
}

std::pair<uint8_t const*, size_t> WriteBuffer::peek() const NOEXCEPT {
    if (empty()) return {nullptr, 0};
    return {mBuffer.data() + mReadPos, mBuffer.size() - mReadPos};
}

void WriteBuffer::consume(size_t bytes) NOEXCEPT {
    TRACEF("%zu", bytes);
    mReadPos += bytes;
    if (mReadPos > mBuffer.size()) mReadPos = mBuffer.size();

    if (mReadPos == mBuffer.size()) {
        mBuffer.clear();
        mReadPos = 0;
    }
    VERBOSEF("consumed %zu bytes, remaining=%zu", bytes, size());
}

void WriteBuffer::clear() NOEXCEPT {
    VSCOPE_FUNCTION();
    mBuffer.clear();
    mReadPos = 0;
}

}  // namespace io
