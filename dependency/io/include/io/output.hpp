#pragma once
#include <core/core.hpp>

namespace io {
class Output {
public:
    EXPLICIT Output() NOEXCEPT;
    virtual ~Output() NOEXCEPT;

    virtual void write(uint8_t const* buffer, size_t length) NOEXCEPT = 0;
};
}  // namespace io
