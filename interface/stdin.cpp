#include "stdin.hpp"
#include <stdexcept>
#include <unistd.h>
#include "file_descriptor.hpp"

namespace interface {

StdinInterface::StdinInterface() IF_NOEXCEPT : mFileDescriptor(-1) {}

StdinInterface::~StdinInterface() IF_NOEXCEPT {
    close();
}

void StdinInterface::open() {
    if (mFileDescriptor.is_open()) {
        return;
    }

    auto stdin_fd  = STDIN_FILENO;
    mFileDescriptor = FileDescriptor(stdin_fd);
}

void StdinInterface::close() {
    mFileDescriptor.close();
}

size_t StdinInterface::read(void* data, const size_t size) {
    return mFileDescriptor.read(data, size);
}

size_t StdinInterface::write(const void* data, const size_t size) {
    return mFileDescriptor.write(data, size);
}

bool StdinInterface::can_read() IF_NOEXCEPT {
    return mFileDescriptor.can_read();
}

bool StdinInterface::can_write() IF_NOEXCEPT {
    return mFileDescriptor.can_write();
}

void StdinInterface::wait_for_read() IF_NOEXCEPT {
    mFileDescriptor.wait_for_read();
}

void StdinInterface::wait_for_write() IF_NOEXCEPT {
    mFileDescriptor.wait_for_write();
}

bool StdinInterface::is_open() IF_NOEXCEPT {
    return mFileDescriptor.is_open();
}

void StdinInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       stdin\n");
    if (is_open()) {
        printf("  fd:         %d\n", mFileDescriptor.fd());
    } else {
        printf("  fd:         closed\n");
    }
}

//
//
//

Interface* Interface::stdin() {
    return new StdinInterface();
}

}  // namespace interface
