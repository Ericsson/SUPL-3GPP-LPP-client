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
        switch (errno) {
        case EACCES: fprintf(stderr, "EACCES\n"); break;
        case EEXIST: fprintf(stderr, "EEXIST\n"); break;
        case EFAULT: fprintf(stderr, "EFAULT\n"); break;
        case EFBIG: fprintf(stderr, "EFBIG\n"); break;
        case EINTR: fprintf(stderr, "EINTR\n"); break;
        case EINVAL: fprintf(stderr, "EINVAL\n"); break;
        case EISDIR: fprintf(stderr, "EISDIR\n"); break;
        case ELOOP: fprintf(stderr, "ELOOP\n"); break;
        case EMFILE: fprintf(stderr, "EMFILE\n"); break;
        case ENAMETOOLONG: fprintf(stderr, "ENAMETOOLONG\n"); break;
        case ENFILE: fprintf(stderr, "ENFILE\n"); break;
        case ENODEV: fprintf(stderr, "ENODEV\n"); break;
        default: fprintf(stderr, "Unknown error: %d\n", errno); break;
        }
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
