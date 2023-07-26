#pragma once
#include "target.h"

#include <string>

class SerialTarget final : public TransmitterTarget {
public:
    SerialTarget(std::string device, const int baud_rate);
    ~SerialTarget();

    void transmit(const void* data, const size_t size) override;

private:
    std::string mDevice;
    int         mBaudRate;
    int         mFileDescriptor = -1;
};