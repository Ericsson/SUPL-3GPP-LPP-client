#include "stdout.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <loglet/loglet.hpp>

#ifdef stdout
#undef stdout
#endif

LOGLET_MODULE2(io, stdout);
#define LOGLET_CURRENT_MODULE &LOGLET_MODULE_REF2(io, stdout)

namespace io {
StdoutOutput::StdoutOutput() = default;
StdoutOutput::~StdoutOutput() = default;

void StdoutOutput::write(uint8_t const* buffer, size_t length) NOEXCEPT {
    FUNCTION_SCOPE();

    auto result = ::write(STDOUT_FILENO, buffer, length);
    VERBOSEF("::write(%d, %p, %zu) = %d", STDOUT_FILENO, buffer, length, result);
    if (result < 0) {
        WARNF("failed to write to file: " ERRNO_FMT, ERRNO_ARGS(errno));
    }
}
}  // namespace io
