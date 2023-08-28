#pragma once
#include <string>
#include "file_descriptor.hpp"
#include "interface.hpp"

namespace interface {

class FileInterface final : public Interface {
public:
    explicit FileInterface(std::string file_path, bool truncate) IF_NOEXCEPT;
    ~FileInterface() IF_NOEXCEPT override;

    void open() override;
    void close() override;

    size_t read(void* data, size_t length) override;
    size_t write(const void* data, size_t length) override;

    IF_NODISCARD bool can_read() IF_NOEXCEPT override;
    IF_NODISCARD bool can_write() IF_NOEXCEPT override;

    void wait_for_read() IF_NOEXCEPT override;
    void wait_for_write() IF_NOEXCEPT override;

    IF_NODISCARD bool is_open() IF_NOEXCEPT override;
    void              print_info() IF_NOEXCEPT override;

private:
    std::string    mFilePath;
    bool           mTruncate;
    FileDescriptor mFileDescriptor;
};

}  // namespace interface
