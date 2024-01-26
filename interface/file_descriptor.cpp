#include "file_descriptor.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#ifdef INTERFACE_FD_DEBUG
#include <cstdio>
#include <string.h>
#define FD_DEBUG(...) printf(__VA_ARGS__)
#else
#define FD_DEBUG(...)
#endif

namespace interface {

FileDescriptor::FileDescriptor(int file_descriptor) IF_NOEXCEPT : mFileDescriptor(file_descriptor),
                                                                  mError(NONE) {
    FD_DEBUG("[fd/%6d] created\n", mFileDescriptor);
}

FileDescriptor::~FileDescriptor() IF_NOEXCEPT {
    close();
}

FileDescriptor::FileDescriptor(FileDescriptor&& other) IF_NOEXCEPT {
    FD_DEBUG("[fd/%6d] move created\n", other.mFileDescriptor);

    mFileDescriptor       = other.mFileDescriptor;
    mError                = other.mError;
    other.mFileDescriptor = -1;
    other.mError          = NONE;
}

FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) IF_NOEXCEPT {
    FD_DEBUG("[fd/%6d] move assigned from [fd/%6d]\n", mFileDescriptor, other.mFileDescriptor);
    close();

    mFileDescriptor       = other.mFileDescriptor;
    mError                = other.mError;
    other.mFileDescriptor = -1;
    other.mError          = NONE;
    return *this;
}

void FileDescriptor::close() IF_NOEXCEPT {
    FD_DEBUG("[fd/%6d] closed\n", mFileDescriptor);
    if (mFileDescriptor >= 0) {
        ::close(mFileDescriptor);
        mFileDescriptor = -1;
    }
}

bool FileDescriptor::can_read() IF_NOEXCEPT {
    struct timeval tv = {0, 0};
    return select(true, false, false, &tv);
}

bool FileDescriptor::can_write() IF_NOEXCEPT {
    struct timeval tv = {0, 0};
    return select(false, true, false, &tv);
}

bool FileDescriptor::wait_for_read() IF_NOEXCEPT {
    return select(true, false, false, nullptr);
}

bool FileDescriptor::wait_for_write() IF_NOEXCEPT {
    return select(false, true, false, nullptr);
}

bool FileDescriptor::select(bool read, bool write, bool except, timeval* tv) IF_NOEXCEPT {
    if(mFileDescriptor < 0) {
        return false;
    }
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mFileDescriptor, &fds);

    auto read_fds   = read ? &fds : nullptr;
    auto write_fds  = write ? &fds : nullptr;
    auto except_fds = except ? &fds : nullptr;

    auto ret = ::select(mFileDescriptor + 1, read_fds, write_fds, except_fds, tv);
    FD_DEBUG("[fd/%6d] select(%i,%i,%i,%f) = %d%s\n", mFileDescriptor, read_fds != nullptr,
             write_fds != nullptr, except_fds != nullptr,
             tv ? (tv->tv_sec + tv->tv_usec / 1000000.0) : 0.0, ret,
             tv ? " (timeout)" : "");
    if (ret < 0) {
        FD_DEBUG("[fd/%6d]   failed=%d (%s)\n", mFileDescriptor, errno, strerror(errno));
        if (errno == EBADF) {
            mError = BAD_FILE_DESCRIPTOR;
            close();
        }
        return false;
    }

    return ret > 0;
}

size_t FileDescriptor::read(void* data, size_t length) IF_NOEXCEPT {
    if (mFileDescriptor < 0) {
        return 0;
    }

    auto bytes_read = ::read(mFileDescriptor, data, length);
    FD_DEBUG("[fd/%6d] read(%zu bytes) = %ld\n", mFileDescriptor, length, bytes_read);
    if (bytes_read < 0) {
        FD_DEBUG("[fd/%6d]   failed=%d\n", mFileDescriptor, errno);
        if (errno == EBADF) {
            mError = BAD_FILE_DESCRIPTOR;
            close();
        }
        return 0;
    }

    return bytes_read;
}

size_t FileDescriptor::write(const void* data, size_t length) IF_NOEXCEPT {
    if (mFileDescriptor < 0) {
        return 0;
    }

    auto bytes_written = ::write(mFileDescriptor, data, length);
    FD_DEBUG("[fd/%6d] write(%zu bytes) = %ld\n", mFileDescriptor, length, bytes_written);
    if (bytes_written < 0) {
        FD_DEBUG("[fd/%6d]   failed=%d\n", mFileDescriptor, errno);
        if (errno == EBADF) {
            mError = BAD_FILE_DESCRIPTOR;
            close();
        }
        return 0;
    }

    return bytes_written;
}

IF_NODISCARD FileDescriptor::Error FileDescriptor::error() const IF_NOEXCEPT {
    return mError;
}

IF_NODISCARD bool FileDescriptor::is_open() const IF_NOEXCEPT {
    return mFileDescriptor >= 0;
}

}  // namespace interface