#pragma once
#include <core/core.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace io {

class WriteBuffer {
public:
    EXPLICIT WriteBuffer(size_t max_size = 64 * 1024) NOEXCEPT;

    void                              enqueue(uint8_t const* data, size_t length) NOEXCEPT;
    std::pair<uint8_t const*, size_t> peek() const NOEXCEPT;
    void                              consume(size_t bytes) NOEXCEPT;

    NODISCARD size_t size() const NOEXCEPT { return mBuffer.size() - mReadPos; }
    NODISCARD bool   empty() const NOEXCEPT { return mReadPos >= mBuffer.size(); }
    void             clear() NOEXCEPT;

private:
    std::vector<uint8_t> mBuffer;
    size_t               mReadPos;
    size_t               mMaxSize;
};

}  // namespace io
