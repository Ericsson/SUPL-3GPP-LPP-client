#include "file.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace interface {

FileInterface::FileInterface(std::string file_path, bool truncate) IF_NOEXCEPT
    : mFilePath(std::move(file_path)),
      mFileDescriptor(-1),
      mTruncate(truncate) {}

FileInterface::~FileInterface() IF_NOEXCEPT {
    close();
}

void FileInterface::open() {
    if (mFileDescriptor >= 0) {
        return;
    }

    if (mTruncate) {
        mFileDescriptor = ::open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else {
        mFileDescriptor = ::open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    }

    if (mFileDescriptor < 0) {
        throw std::runtime_error("Failed to open file");
    }
}

void FileInterface::close() {
    if (mFileDescriptor >= 0) {
        ::close(mFileDescriptor);
        mFileDescriptor = -1;
    }
}

size_t FileInterface::read(void* data, size_t length) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("File not open");
    }

    auto bytes_read = ::read(mFileDescriptor, data, length);
    if (bytes_read < 0) {
        throw std::runtime_error("Failed to read from file");
    }

    return bytes_read;
}

size_t FileInterface::write(const void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("File not open");
    }

    auto bytes_written = ::write(mFileDescriptor, data, size);
    if (bytes_written < 0) {
        throw std::runtime_error("Failed to write to file");
    }

    return bytes_written;
}

bool FileInterface::can_read() IF_NOEXCEPT {
    return false;
}

bool FileInterface::can_write() IF_NOEXCEPT {
    return false;
}

void FileInterface::wait_for_read() IF_NOEXCEPT {}

void FileInterface::wait_for_write() IF_NOEXCEPT {}

bool FileInterface::is_open() IF_NOEXCEPT {
    return false;
}

//
//
//

Interface* Interface::file(std::string file_path, bool truncate) {
    return new FileInterface(std::move(file_path), truncate);
}

}  // namespace interface
