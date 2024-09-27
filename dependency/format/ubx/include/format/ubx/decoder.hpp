#pragma once
#include <core/core.hpp>

#include <string>

namespace format {
namespace ubx {

class Decoder {
public:
    EXPLICIT Decoder(uint8_t* payload, uint32_t payload_length) NOEXCEPT;

    uint8_t  X1() NOEXCEPT;
    uint16_t X2() NOEXCEPT;
    uint32_t X4() NOEXCEPT;
    uint64_t X8() NOEXCEPT;

    uint8_t  E1() NOEXCEPT;
    uint16_t E2() NOEXCEPT;
    uint32_t E4() NOEXCEPT;

    uint8_t  U1() NOEXCEPT;
    uint16_t U2() NOEXCEPT;
    uint32_t U4() NOEXCEPT;
    uint64_t U8() NOEXCEPT;

    int8_t  I1() NOEXCEPT;
    int16_t I2() NOEXCEPT;
    int32_t I4() NOEXCEPT;
    int64_t I8() NOEXCEPT;

    float  R4() NOEXCEPT;
    double R8() NOEXCEPT;

    bool        L() NOEXCEPT;
    std::string CH(uint32_t length) NOEXCEPT;

    void                     skip(uint32_t length) NOEXCEPT;
    NODISCARD uint32_t remaining() const NOEXCEPT;
    NODISCARD bool     error() const NOEXCEPT;

private:
    uint8_t* mPayload;
    uint32_t mPayloadLength;
    bool     mError;
};

}  // namespace ubx
}  // namespace format
