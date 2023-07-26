#include "i2c.h"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

I2CTarget::I2CTarget(std::string device, const uint8_t address)
    : mDevice(std::move(device)), mAddress(address) {
    mFileDescriptor = open(mDevice.c_str(), O_RDWR);
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Failed to open I2C device");
    }

    if (ioctl(mFileDescriptor, I2C_SLAVE, mAddress) < 0) {
        throw std::runtime_error("Failed to acquire bus access and/or talk to slave");
    }
}

I2CTarget::~I2CTarget() {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
}

void I2CTarget::transmit(const void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("I2C device not open");
    }

    if (write(mFileDescriptor, data, size) != size) {
        throw std::runtime_error("Failed to write to I2C device");
    }
}
