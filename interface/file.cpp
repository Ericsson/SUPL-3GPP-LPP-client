#include "file.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace interface {

FileInterface::FileInterface(std::string file_path, bool truncate) IF_NOEXCEPT
    : mFilePath(std::move(file_path)),
      mTruncate(truncate),
      mFileDescriptor(-1) {}

FileInterface::~FileInterface() IF_NOEXCEPT {
    close();
}

void FileInterface::open() {
    if (is_open()) {
        return;
    }

    int fd = -1;
    if (mTruncate) {
        fd = ::open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else {
        fd = ::open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    }

    if (fd < 0) {
        throw std::runtime_error("Failed to open file: " + mFilePath);
    }

    mFileDescriptor = FileDescriptor(fd);
}

void FileInterface::close() {
    mFileDescriptor.close();
}

size_t FileInterface::read(void* data, size_t length) {
    return mFileDescriptor.read(data, length);
}

size_t FileInterface::write(const void* data, const size_t size) {
    return mFileDescriptor.write(data, size);
}

bool FileInterface::can_read() IF_NOEXCEPT {
    return mFileDescriptor.can_read();
}

bool FileInterface::can_write() IF_NOEXCEPT {
    return mFileDescriptor.can_write();
}

void FileInterface::wait_for_read() IF_NOEXCEPT {
    mFileDescriptor.wait_for_read();
}

void FileInterface::wait_for_write() IF_NOEXCEPT {
    mFileDescriptor.wait_for_write();
}

bool FileInterface::is_open() IF_NOEXCEPT {
    return mFileDescriptor.is_open();
}

void FileInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       file\n");
    printf("  path:       %s\n", mFilePath.c_str());
    printf("  truncate:   %s\n", mTruncate ? "true" : "false");
    if (is_open()) {
        printf("  fd:         %d\n", mFileDescriptor.fd());
    } else {
        printf("  fd:         closed\n");
    }
}

//
//
//

Interface* Interface::file(std::string file_path, bool truncate) {
    return new FileInterface(std::move(file_path), truncate);
}

}  // namespace interface
