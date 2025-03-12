#pragma once
#include <io/output.hpp>

namespace io {
/// Output to stdout.
class StdoutOutput : public Output {
public:
    EXPLICIT StdoutOutput() NOEXCEPT;
    ~StdoutOutput() NOEXCEPT override;

    NODISCARD const char* name() const NOEXCEPT override { return "stdout"; }

    void write(uint8_t const* buffer, size_t length) NOEXCEPT override;
};
}  // namespace io
