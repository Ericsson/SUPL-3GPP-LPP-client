#pragma once
#include <core/core.hpp>

#include <memory>

namespace format {
namespace helper {

class Parser {
public:
    EXPLICIT Parser() NOEXCEPT;
    virtual ~Parser() NOEXCEPT;

    bool append(uint8_t* data, size_t length) NOEXCEPT;
    void clear() NOEXCEPT;

    NODISCARD virtual char const* name() const NOEXCEPT = 0;

    NODISCARD uint32_t buffer_length() const NOEXCEPT;
    NODISCARD uint32_t available_space() const NOEXCEPT;

protected:
    NODISCARD uint8_t peek(uint32_t index) const NOEXCEPT;
    NODISCARD void    skip(uint32_t length) NOEXCEPT;
    NODISCARD void    skip(uint64_t length) NOEXCEPT { skip(static_cast<uint32_t>(length)); }

    void copy_to_buffer(uint8_t* data, size_t length) NOEXCEPT;

private:
    uint8_t* mBuffer;
    uint32_t mBufferCapacity;
    uint32_t mBufferRead;
    uint32_t mBufferWrite;
};

}  // namespace helper
}  // namespace format
