#pragma once
#include <receiver/ublox/types.hpp>
#include <string>

namespace receiver {
namespace ublox {

class Decoder {
public:
    UBLOX_EXPLICIT Decoder(uint8_t* payload, uint32_t payload_length) UBLOX_NOEXCEPT;

    uint8_t  X1() UBLOX_NOEXCEPT;
    uint16_t X2() UBLOX_NOEXCEPT;
    uint32_t X4() UBLOX_NOEXCEPT;
    uint64_t X8() UBLOX_NOEXCEPT;

    uint8_t  E1() UBLOX_NOEXCEPT;
    uint16_t E2() UBLOX_NOEXCEPT;
    uint32_t E4() UBLOX_NOEXCEPT;

    uint8_t  U1() UBLOX_NOEXCEPT;
    uint16_t U2() UBLOX_NOEXCEPT;
    uint32_t U4() UBLOX_NOEXCEPT;
    uint64_t U8() UBLOX_NOEXCEPT;

    int8_t  I1() UBLOX_NOEXCEPT;
    int16_t I2() UBLOX_NOEXCEPT;
    int32_t I4() UBLOX_NOEXCEPT;
    int64_t I8() UBLOX_NOEXCEPT;

    float  R4() UBLOX_NOEXCEPT;
    double R8() UBLOX_NOEXCEPT;

    bool        L() UBLOX_NOEXCEPT;
    std::string CH(uint32_t length) UBLOX_NOEXCEPT;

    void                     skip(uint32_t length) UBLOX_NOEXCEPT;
    UBLOX_NODISCARD uint32_t remaining() const UBLOX_NOEXCEPT;
    UBLOX_NODISCARD bool     error() const UBLOX_NOEXCEPT;

private:
    uint8_t* mPayload;
    uint32_t mPayloadLength;
    bool     mError;
};

}  // namespace ublox
}  // namespace receiver
