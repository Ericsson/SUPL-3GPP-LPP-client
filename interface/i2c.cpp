#include "i2c.hpp"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace interface {

I2CInterface::I2CInterface(std::string device_path, uint8_t address) IF_NOEXCEPT
    : mDevicePath(std::move(device_path)),
      mAddress(address),
      mFileDescriptor(-1) {}

I2CInterface::~I2CInterface() IF_NOEXCEPT {
    close();
}

void I2CInterface::open() {
    if (mFileDescriptor >= 0) {
        return;
    }

    mFileDescriptor = ::open(mDevicePath.c_str(), O_RDWR);
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Failed to open I2C device");
    }

    if (ioctl(mFileDescriptor, I2C_SLAVE, mAddress) < 0) {
        mFileDescriptor = -1;
        throw std::runtime_error("Failed to acquire bus access and/or talk to slave");
    }
}

void I2CInterface::close() {
    if (mFileDescriptor >= 0) {
        ::close(mFileDescriptor);
        mFileDescriptor = -1;
    }
}

size_t I2CInterface::read(void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("I2C device not open");
    }

    auto bytes_read = ::read(mFileDescriptor, data, size);
    if (bytes_read < 0) {
        throw std::runtime_error("Failed to read from I2C device");
    }

    return bytes_read;
}

size_t I2CInterface::write(const void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("I2C device not open");
    }

    auto bytes_written = ::write(mFileDescriptor, data, size);
    if (bytes_written < 0) {
        throw std::runtime_error("Failed to write to I2C device");
    }

    return bytes_written;
}

bool I2CInterface::can_read() const IF_NOEXCEPT {
    return false;
}

bool I2CInterface::can_write() const IF_NOEXCEPT {
    return false;
}

//
//
//

Interface* Interface::i2c(std::string device_path, uint8_t address) {
    return new I2CInterface(std::move(device_path), address);
}

}  // namespace interface
