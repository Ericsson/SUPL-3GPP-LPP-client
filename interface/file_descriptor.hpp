#pragma once
#include <sys/time.h>
#include "types.hpp"

namespace interface {

/// Helper class to manage file descriptors.
class FileDescriptor {
public:
    IF_EXPLICIT FileDescriptor(int file_descriptor) IF_NOEXCEPT;
    ~FileDescriptor() IF_NOEXCEPT;

    FileDescriptor(const FileDescriptor&)            = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;
    FileDescriptor(FileDescriptor&& other) IF_NOEXCEPT;
    FileDescriptor& operator=(FileDescriptor&& other) IF_NOEXCEPT;

    void close() IF_NOEXCEPT;

    IF_NODISCARD bool can_read() IF_NOEXCEPT;
    IF_NODISCARD bool can_write() IF_NOEXCEPT;

    bool wait_for_read() IF_NOEXCEPT;
    bool wait_for_write() IF_NOEXCEPT;

    IF_NODISCARD size_t read(void* data, size_t length) IF_NOEXCEPT;
    IF_NODISCARD size_t write(const void* data, size_t length) IF_NOEXCEPT;

    enum Error {
        NONE                = 0,
        BAD_FILE_DESCRIPTOR = 1,
    };

    IF_NODISCARD Error error() const IF_NOEXCEPT;
    IF_NODISCARD bool  is_open() const IF_NOEXCEPT;
    IF_NODISCARD int   fd() const IF_NOEXCEPT { return mFileDescriptor; }

private:
    IF_NODISCARD bool select(bool read, bool write, bool except, timeval* tv) IF_NOEXCEPT;

    int   mFileDescriptor;
    Error mError;
};

}  // namespace interface
