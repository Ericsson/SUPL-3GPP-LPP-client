#pragma once
#include <string>
#include "interface.hpp"
#include "file_descriptor.hpp"

namespace interface {

class SerialInterface final : public Interface {
public:
    explicit SerialInterface(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                             ParityBits parity_bits) IF_NOEXCEPT;
    ~SerialInterface() IF_NOEXCEPT override;

    void open() override;
    void close() override;

    size_t read(void* data, size_t length) override;
    size_t write(const void* data, size_t length) override;

    IF_NODISCARD bool can_read() IF_NOEXCEPT override;
    IF_NODISCARD bool can_write() IF_NOEXCEPT override;

    void wait_for_read() IF_NOEXCEPT override;
    void wait_for_write() IF_NOEXCEPT override;
private:
    std::string mDevicePath;
    uint32_t    mBaudRate;
    StopBits    mStopBits;
    ParityBits  mParityBits;
    FileDescriptor mFileDescriptor;
};

}  // namespace interface
