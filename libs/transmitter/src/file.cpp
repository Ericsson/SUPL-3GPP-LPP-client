#include "file.h"

#include <cstdio>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

FileTarget::FileTarget(std::string file_path, bool truncate) : mFilePath(std::move(file_path)) {
    if (truncate) {
        mFileDescriptor = open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    } else {
        mFileDescriptor = open(mFilePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    }

    if (mFileDescriptor < 0) {
        throw std::runtime_error("Failed to open file");
    }
}

FileTarget::~FileTarget() {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
}

void FileTarget::transmit(const void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("File not open");
    }

    if (write(mFileDescriptor, data, size) != size) {
        throw std::runtime_error("Failed to write to file");
    }
}
