#pragma once
#include <cstddef>
#include <string>
#include "file_descriptor.hpp"
#include "interface.hpp"

namespace interface {

class I2CInterface final : public Interface {
public:
    explicit I2CInterface(std::string device_path, uint8_t address) IF_NOEXCEPT;
    ~I2CInterface() IF_NOEXCEPT override;

    void open() override;
    void close() override;

    size_t read(void* data, size_t length) override;
    size_t write(const void* data, size_t length) override;

    IF_NODISCARD bool can_read() IF_NOEXCEPT override;
    IF_NODISCARD bool can_write() IF_NOEXCEPT override;

    void wait_for_read() IF_NOEXCEPT override;
    void wait_for_write() IF_NOEXCEPT override;

    IF_NODISCARD bool is_open() IF_NOEXCEPT override;
    void print_info() IF_NOEXCEPT override;

private:
    std::string    mDevicePath;
    uint8_t        mAddress;
    FileDescriptor mFileDescriptor;
};

}  // namespace interface
