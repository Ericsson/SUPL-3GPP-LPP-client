#include "serial.hpp"
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

namespace interface {

SerialInterface::SerialInterface(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                                 ParityBits parity_bits) IF_NOEXCEPT
    : mDevicePath(std::move(device_path)),
      mBaudRate(baud_rate),
      mStopBits(stop_bits),
      mParityBits(parity_bits),
      mFileDescriptor(-1) {}

SerialInterface::~SerialInterface() IF_NOEXCEPT {
    close();
}

void SerialInterface::open() {
    if (mFileDescriptor >= 0) {
        return;
    }

    mFileDescriptor = ::open(mDevicePath.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Could not open serial device");
    }

    // set file status flags
    fcntl(mFileDescriptor, F_SETFL, 0);

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(mFileDescriptor, &tty) != 0) {
        mFileDescriptor = -1;
        throw std::runtime_error("Could not get serial device attributes");
    }

    // set baud rate
    cfsetospeed(&tty, (speed_t)mBaudRate);
    cfsetispeed(&tty, (speed_t)mBaudRate);

    // set raw mode
    cfmakeraw(&tty);

    tty.c_cflag |= (CLOCAL | CREAD);  // ignore modem controls,
                                      // enable receiver
    tty.c_cflag &= ~CRTSCTS;          // enable RTS/CTS (hardware) flow control.

    if (mStopBits == StopBits::Two) {
        tty.c_cflag |= CSTOPB;
    } else {
        tty.c_cflag &= ~CSTOPB;
    }

    if (mParityBits == ParityBits::Odd) {
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
    } else if (mParityBits == ParityBits::Even) {
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    } else {
        tty.c_cflag &= ~PARENB;
    }

    if (tcsetattr(mFileDescriptor, TCSANOW, &tty) != 0) {
        mFileDescriptor = -1;
        throw std::runtime_error("Could not set serial device attributes");
    }
}

void SerialInterface::close() {
    if (mFileDescriptor >= 0) {
        ::close(mFileDescriptor);
        mFileDescriptor = -1;
    }
}

size_t SerialInterface::read(void* data, size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Serial device not open");
    }

    auto bytes_read = ::read(mFileDescriptor, data, size);
    if (bytes_read < 0) {
        throw std::runtime_error("Failed to read from serial device");
    }

    return bytes_read;
}

size_t SerialInterface::write(const void* data, size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Serial device not open");
    }

    auto bytes_written = ::write(mFileDescriptor, data, size);
    if (bytes_written < 0) {
        throw std::runtime_error("Failed to write to serial device");
    }

    return bytes_written;
}

//
//
//

Interface* Interface::serial(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                             ParityBits parity_bits) {
    return new SerialInterface(std::move(device_path), baud_rate, stop_bits, parity_bits);
}

}  // namespace interface
