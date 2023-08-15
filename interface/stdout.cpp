#include "stdout.hpp"
#include <stdexcept>
#include <unistd.h>
#include "file_descriptor.hpp"

namespace interface {

StdoutInterface::StdoutInterface() IF_NOEXCEPT : mFileDescriptor(-1) {}

StdoutInterface::~StdoutInterface() IF_NOEXCEPT {
    close();
}

void StdoutInterface::open() {
    if (mFileDescriptor.is_open()) {
        return;
    }

    auto stdout_fd  = STDOUT_FILENO;
    mFileDescriptor = FileDescriptor(stdout_fd);
}

void StdoutInterface::close() {
    mFileDescriptor.close();
}

size_t StdoutInterface::read(void* data, const size_t size) {
    return mFileDescriptor.read(data, size);
}

size_t StdoutInterface::write(const void* data, const size_t size) {
    return mFileDescriptor.write(data, size);
}

bool StdoutInterface::can_read() IF_NOEXCEPT {
    return mFileDescriptor.can_read();
}

bool StdoutInterface::can_write() IF_NOEXCEPT {
    return mFileDescriptor.can_write();
}

void StdoutInterface::wait_for_read() IF_NOEXCEPT {
    mFileDescriptor.wait_for_read();
}

void StdoutInterface::wait_for_write() IF_NOEXCEPT {
    mFileDescriptor.wait_for_write();
}

bool StdoutInterface::is_open() IF_NOEXCEPT {
    return mFileDescriptor.is_open();
}

void StdoutInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       stdout\n");
    if (is_open()) {
        printf("  fd:         %d\n", mFileDescriptor.fd());
    } else {
        printf("  fd:         closed\n");
    }
}

//
//
//

Interface* Interface::stdout() {
    return new StdoutInterface();
}

}  // namespace interface
