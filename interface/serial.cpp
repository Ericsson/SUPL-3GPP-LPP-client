#include "serial.hpp"
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

namespace interface {

SerialInterface::SerialInterface(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                                 ParityBit parity_bit) IF_NOEXCEPT
    : mDevicePath(std::move(device_path)),
      mBaudRate(baud_rate),
      mStopBits(stop_bits),
      mParityBit(parity_bit),
      mFileDescriptor(-1) {}

SerialInterface::~SerialInterface() IF_NOEXCEPT = default;

static speed_t buad_rate_constant_from(uint32_t buad_rate) {
    switch (buad_rate) {
    case 50: return B50;
    case 75: return B75;
    case 110: return B110;
    case 134: return B134;
    case 150: return B150;
    case 200: return B200;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 1800: return B1800;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 500000: return B500000;
    case 576000: return B576000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1152000: return B1152000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    case 2500000: return B2500000;
    case 3000000: return B3000000;
    case 3500000: return B3500000;
    case 4000000: return B4000000;
    default: throw std::runtime_error("Invalid baud rate");
    }
}

void SerialInterface::open() {
    if (mFileDescriptor.is_open()) {
        return;
    }

    auto fd = ::open(mDevicePath.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        throw std::runtime_error("Could not open serial device");
    }

    // set file status flags
    fcntl(fd, F_SETFL, 0);

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        throw std::runtime_error("Could not get serial device attributes");
    }

    // set raw mode
    cfmakeraw(&tty);

    tty.c_cflag |= (CLOCAL | CREAD);  // ignore modem controls,
                                      // enable receiver
    tty.c_cflag &= ~CRTSCTS;          // enable RTS/CTS (hardware) flow control.

    if (mStopBits == StopBits::TWO) {
        tty.c_cflag |= CSTOPB;
    } else {
        tty.c_cflag &= ~CSTOPB;
    }

    if (mParityBit == ParityBit::ODD) {
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
    } else if (mParityBit == ParityBit::EVEN) {
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    } else {
        tty.c_cflag &= ~PARENB;
    }

    // set baud rate
    if (cfsetospeed(&tty, buad_rate_constant_from(mBaudRate)) != 0) {
        throw std::runtime_error("Failed to set serial output baud rate");
    }

    if (cfsetispeed(&tty, buad_rate_constant_from(mBaudRate)) != 0) {
        throw std::runtime_error("Failed to set serial input baud rate");
    }

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        throw std::runtime_error("Could not set serial device attributes");
    }

    mFileDescriptor = FileDescriptor(fd);
}

void SerialInterface::close() {
    mFileDescriptor.close();
}

size_t SerialInterface::read(void* data, size_t size) {
    return mFileDescriptor.read(data, size);
}

size_t SerialInterface::write(const void* data, size_t size) {
    return mFileDescriptor.write(data, size);
}

bool SerialInterface::can_read() IF_NOEXCEPT {
    return mFileDescriptor.can_read();
}

bool SerialInterface::can_write() IF_NOEXCEPT {
    return mFileDescriptor.can_write();
}

void SerialInterface::wait_for_read() IF_NOEXCEPT {
    mFileDescriptor.wait_for_read();
}

void SerialInterface::wait_for_write() IF_NOEXCEPT {
    mFileDescriptor.wait_for_write();
}

bool SerialInterface::is_open() IF_NOEXCEPT {
    return mFileDescriptor.is_open();
}

//
//
//

Interface* Interface::serial(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                             ParityBit parity_bit) {
    return new SerialInterface(std::move(device_path), baud_rate, stop_bits, parity_bit);
}

}  // namespace interface
