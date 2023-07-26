#include "stdout.h"

#include <unistd.h>
#include <stdexcept>

StdoutTarget::StdoutTarget() = default;

StdoutTarget::~StdoutTarget() = default;

void StdoutTarget::transmit(const void* data, const size_t size) {
    if (write(STDOUT_FILENO, data, size) != size) {
        throw std::runtime_error("Failed to write to stdout");
    }
}
