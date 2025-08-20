#pragma once
#include <core/core.hpp>

#include <string>

namespace format {
namespace ubx {

class Encoder {
public:
    EXPLICIT Encoder(uint8_t* payload, uint32_t payload_length) NOEXCEPT;

    void X1(uint8_t value) NOEXCEPT;
    void X2(uint16_t value) NOEXCEPT;
    void X4(uint32_t value) NOEXCEPT;
    void X8(uint64_t value) NOEXCEPT;

    void E1(uint8_t value) NOEXCEPT;
    void E2(uint16_t value) NOEXCEPT;
    void E4(uint32_t value) NOEXCEPT;

    void U1(uint8_t value) NOEXCEPT;
    void U2(uint16_t value) NOEXCEPT;
    void U4(uint32_t value) NOEXCEPT;
    void U8(uint64_t value) NOEXCEPT;

    void I1(int8_t value) NOEXCEPT;
    void I2(int16_t value) NOEXCEPT;
    void I4(int32_t value) NOEXCEPT;
    void I8(int64_t value) NOEXCEPT;

    void R4(float value) NOEXCEPT;
    void R8(double value) NOEXCEPT;

    void L(bool value) NOEXCEPT;
    void CH(std::string const& value, uint32_t max_length) NOEXCEPT;

    void               pad(uint32_t length) NOEXCEPT;
    NODISCARD uint32_t remaining() const NOEXCEPT;
    NODISCARD bool     error() const NOEXCEPT;
    NODISCARD uint8_t* ptr() const NOEXCEPT;

private:
    uint8_t* mPayload;
    uint32_t mPayloadLength;
    bool     mError;
};

}  // namespace ubx
}  // namespace format
