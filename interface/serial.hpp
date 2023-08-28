#pragma once
#include <string>
#include "file_descriptor.hpp"
#include "interface.hpp"

namespace interface {

class SerialInterface final : public Interface {
public:
    explicit SerialInterface(std::string device_path, uint32_t baud_rate, DataBits data_bits,
                             StopBits stop_bits, ParityBit parity_bit) IF_NOEXCEPT;
    ~SerialInterface() IF_NOEXCEPT override;

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
    std::string    mDevicePath;
    uint32_t       mBaudRate;
    uint32_t       mBaudRateConstant;
    DataBits       mDataBits;
    StopBits       mStopBits;
    ParityBit      mParityBit;
    FileDescriptor mFileDescriptor;
};

}  // namespace interface
