#pragma once
#include <string>
#include "interface.hpp"

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

private:
    std::string mDevicePath;
    uint32_t    mBaudRate;
    StopBits    mStopBits;
    ParityBits  mParityBits;
    int         mFileDescriptor;
};

}  // namespace interface
