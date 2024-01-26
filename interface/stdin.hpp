#pragma once
#include <cstddef>
#include <string>
#include "file_descriptor.hpp"
#include "interface.hpp"

namespace interface {

class StdinInterface final : public Interface {
public:
    explicit StdinInterface() IF_NOEXCEPT;
    ~StdinInterface() IF_NOEXCEPT override;

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
    FileDescriptor mFileDescriptor;
};

}  // namespace interface
