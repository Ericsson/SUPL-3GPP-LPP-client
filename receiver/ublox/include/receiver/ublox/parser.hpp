#pragma once
#include <receiver/ublox/types.hpp>

namespace receiver {
namespace ublox {

class Message;
class Parser {
public:
    UBLOX_EXPLICIT Parser() UBLOX_NOEXCEPT;
    ~Parser() UBLOX_NOEXCEPT;

    UBLOX_NODISCARD bool append(uint8_t* data, uint16_t length) UBLOX_NOEXCEPT;

    UBLOX_NODISCARD Message* try_parse() UBLOX_NOEXCEPT;

protected:
    UBLOX_NODISCARD uint32_t  buffer_length() const UBLOX_NOEXCEPT;
    UBLOX_NODISCARD bool is_frame_boundary() const UBLOX_NOEXCEPT;

    UBLOX_NODISCARD uint8_t peek(uint32_t index) const UBLOX_NOEXCEPT;
    void               skip(uint32_t length) UBLOX_NOEXCEPT;
    void               copy_to_buffer(uint8_t* data, uint32_t length) UBLOX_NOEXCEPT;

    UBLOX_NODISCARD uint16_t checksum(uint8_t* payload, uint16_t length) const UBLOX_NOEXCEPT;

private:
    uint8_t* mBuffer;
    uint32_t mBufferCapacity;
    uint32_t mBufferRead;
    uint32_t mBufferWrite;
};

}  // namespace ublox
}  // namespace receiver
