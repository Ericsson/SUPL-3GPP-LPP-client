#include "i2c.hpp"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>
#include "file_descriptor.hpp"

namespace interface {

I2CInterface::I2CInterface(std::string device_path, uint8_t address) IF_NOEXCEPT
    : mDevicePath(std::move(device_path)),
      mAddress(address),
      mFileDescriptor(-1) {}

I2CInterface::~I2CInterface() IF_NOEXCEPT {
    close();
}

void I2CInterface::open() {
    if (mFileDescriptor.is_open()) {
        return;
    }

    auto fd = ::open(mDevicePath.c_str(), O_RDWR);
    if (fd < 0) {
        throw std::runtime_error("Failed to open I2C device");
    }

    if (ioctl(fd, I2C_SLAVE, mAddress) < 0) {
        throw std::runtime_error("Failed to acquire bus access and/or talk to slave");
    }

    mFileDescriptor = FileDescriptor(fd);
}

void I2CInterface::close() {
    mFileDescriptor.close();
}

size_t I2CInterface::read(void* data, const size_t size) {
    return mFileDescriptor.read(data, size);
}

size_t I2CInterface::write(const void* data, const size_t size) {
    return mFileDescriptor.write(data, size);
}

bool I2CInterface::can_read() IF_NOEXCEPT {
    return mFileDescriptor.can_read();
}

bool I2CInterface::can_write() IF_NOEXCEPT {
    return mFileDescriptor.can_write();
}

void I2CInterface::wait_for_read() IF_NOEXCEPT {
    mFileDescriptor.wait_for_read();
}

void I2CInterface::wait_for_write() IF_NOEXCEPT {
    mFileDescriptor.wait_for_write();
}

bool I2CInterface::is_open() IF_NOEXCEPT {
    return mFileDescriptor.is_open();
}

void I2CInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       i2c\n");
    printf("  device:     %s\n", mDevicePath.c_str());
    printf("  address:  0x%02x\n", mAddress);
}

//
//
//

Interface* Interface::i2c(std::string device_path, uint8_t address) {
    return new I2CInterface(std::move(device_path), address);
}

}  // namespace interface
