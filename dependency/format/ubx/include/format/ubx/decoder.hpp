#pragma once
#include <core/core.hpp>

#include <string>

namespace format {
namespace ubx {

class Decoder {
public:
    EXPLICIT Decoder(uint8_t* payload, uint32_t payload_length) NOEXCEPT;

    uint8_t  x1() NOEXCEPT;
    uint16_t x2() NOEXCEPT;
    uint32_t x4() NOEXCEPT;
    uint64_t x8() NOEXCEPT;

    uint8_t  e1() NOEXCEPT;
    uint16_t e2() NOEXCEPT;
    uint32_t e4() NOEXCEPT;

    uint8_t  u1() NOEXCEPT;
    uint16_t u2() NOEXCEPT;
    uint32_t u4() NOEXCEPT;
    uint64_t u8() NOEXCEPT;

    int8_t  i1() NOEXCEPT;
    int16_t i2() NOEXCEPT;
    int32_t i4() NOEXCEPT;
    int64_t i8() NOEXCEPT;

    float  r4() NOEXCEPT;
    double r8() NOEXCEPT;

    bool        logical() NOEXCEPT;
    std::string ch(uint32_t length) NOEXCEPT;

    void               skip(uint32_t length) NOEXCEPT;
    NODISCARD uint32_t remaining() const NOEXCEPT;
    NODISCARD bool     error() const NOEXCEPT;

private:
    uint8_t* mPayload;
    uint32_t mPayloadLength;
    bool     mError;
};

}  // namespace ubx
}  // namespace format
