#pragma once
#include <receiver/ublox/types.hpp>
#include <string>

namespace receiver {
namespace ublox {

class Encoder {
public:
    UBLOX_EXPLICIT Encoder(uint8_t* payload, uint32_t payload_length) UBLOX_NOEXCEPT;

    void X1(uint8_t value) UBLOX_NOEXCEPT;
    void X2(uint16_t value) UBLOX_NOEXCEPT;
    void X4(uint32_t value) UBLOX_NOEXCEPT;
    void X8(uint64_t value) UBLOX_NOEXCEPT;

    void E1(uint8_t value) UBLOX_NOEXCEPT;
    void E2(uint16_t value) UBLOX_NOEXCEPT;
    void E4(uint32_t value) UBLOX_NOEXCEPT;

    void U1(uint8_t value) UBLOX_NOEXCEPT;
    void U2(uint16_t value) UBLOX_NOEXCEPT;
    void U4(uint32_t value) UBLOX_NOEXCEPT;
    void U8(uint64_t value) UBLOX_NOEXCEPT;

    void I1(int8_t value) UBLOX_NOEXCEPT;
    void I2(int16_t value) UBLOX_NOEXCEPT;
    void I4(int32_t value) UBLOX_NOEXCEPT;
    void I8(int64_t value) UBLOX_NOEXCEPT;

    void R4(float value) UBLOX_NOEXCEPT;
    void R8(double value) UBLOX_NOEXCEPT;

    void L(bool value) UBLOX_NOEXCEPT;
    void CH(const std::string& value, uint32_t max_length) UBLOX_NOEXCEPT;

    void                     pad(uint32_t length) UBLOX_NOEXCEPT;
    UBLOX_NODISCARD uint32_t remaining() const UBLOX_NOEXCEPT;
    UBLOX_NODISCARD bool     error() const UBLOX_NOEXCEPT;
    UBLOX_NODISCARD uint8_t* ptr() const UBLOX_NOEXCEPT;

private:
    uint8_t* mPayload;
    uint32_t mPayloadLength;
    bool     mError;
};

}  // namespace ublox
}  // namespace receiver
