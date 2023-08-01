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
        throw std::runtime_error("Could not get serial device attributes");
    }

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

    // set baud rate
    if (cfsetospeed(&tty, buad_rate_constant_from(mBaudRate)) != 0) {
        throw std::runtime_error("Failed to set serial output baud rate");
    }

    if (cfsetispeed(&tty, buad_rate_constant_from(mBaudRate)) != 0) {
        throw std::runtime_error("Failed to set serial input baud rate");
    }

    if (tcsetattr(mFileDescriptor, TCSANOW, &tty) != 0) {
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

#ifdef INTERFACE_SERIAL_DEBUG
    printf("[serial] write: %zu bytes\n", size);
    size_t i = 0;
    while (i < size) {
        printf("%02x ", ((uint8_t*)data)[i]);
        i++;
        if (i % 16 == 0) {
            printf("\n");
        }
    }
    if (i % 16 != 0) {
        printf("\n");
    }
#endif

    auto bytes_written = ::write(mFileDescriptor, data, size);
    if (bytes_written < 0) {
        throw std::runtime_error("Failed to write to serial device");
    }

    return bytes_written;
}

bool SerialInterface::can_read() const IF_NOEXCEPT {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mFileDescriptor, &fds);

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    auto result = select(mFileDescriptor + 1, &fds, nullptr, nullptr, &tv);
    return result > 0;
}

bool SerialInterface::can_write() const IF_NOEXCEPT {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mFileDescriptor, &fds);

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    auto result = select(mFileDescriptor + 1, nullptr, &fds, nullptr, &tv);
    return result > 0;
}

//
//
//

Interface* Interface::serial(std::string device_path, uint32_t baud_rate, StopBits stop_bits,
                             ParityBits parity_bits) {
    return new SerialInterface(std::move(device_path), baud_rate, stop_bits, parity_bits);
}

}  // namespace interface
