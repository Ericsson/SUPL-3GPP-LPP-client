#pragma once
#include <core/core.hpp>

#include <string>

namespace format {
namespace ubx {

class Encoder {
public:
    EXPLICIT Encoder(uint8_t* payload, uint32_t payload_length) NOEXCEPT;

    void x1(uint8_t value) NOEXCEPT;
    void x2(uint16_t value) NOEXCEPT;
    void x4(uint32_t value) NOEXCEPT;
    void x8(uint64_t value) NOEXCEPT;

    void e1(uint8_t value) NOEXCEPT;
    void e2(uint16_t value) NOEXCEPT;
    void e4(uint32_t value) NOEXCEPT;

    void u1(uint8_t value) NOEXCEPT;
    void u2(uint16_t value) NOEXCEPT;
    void u4(uint32_t value) NOEXCEPT;
    void u8(uint64_t value) NOEXCEPT;

    void i1(int8_t value) NOEXCEPT;
    void i2(int16_t value) NOEXCEPT;
    void i4(int32_t value) NOEXCEPT;
    void i8(int64_t value) NOEXCEPT;

    void r4(float value) NOEXCEPT;
    void r8(double value) NOEXCEPT;

    void logical(bool value) NOEXCEPT;
    void ch(std::string const& value, uint32_t max_length) NOEXCEPT;

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
