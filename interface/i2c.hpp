#pragma once
#include <cstddef>
#include <string>
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

    IF_NODISCARD bool can_read() const IF_NOEXCEPT override;
    IF_NODISCARD bool can_write() const IF_NOEXCEPT override;

private:
    std::string mDevicePath;
    uint8_t     mAddress;
    int         mFileDescriptor;
};

}  // namespace interface
