#pragma once
#include "target.h"

#include <cstddef>
#include <string>

class I2CTarget final : public TransmitterTarget {
public:
    I2CTarget(std::string device, const uint8_t address);
    ~I2CTarget();

    void transmit(const void* data, const size_t size) override;

private:
    std::string mDevice;
    uint8_t     mAddress;
    int         mFileDescriptor = -1;
};