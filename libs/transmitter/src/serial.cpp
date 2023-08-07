#include "serial.h"

#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

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


SerialTarget::SerialTarget(std::string device, const int baud_rate)
    : mDevice(std::move(device)), mBaudRate(baud_rate) {
    mFileDescriptor = open(mDevice.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
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

SerialTarget::~SerialTarget() {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
}

void SerialTarget::transmit(const void* data, const size_t size) {
    if (mFileDescriptor < 0) {
        throw std::runtime_error("Serial device not open");
    }

    if (write(mFileDescriptor, data, size) != size) {
        throw std::runtime_error("Failed to write to serial device");
    }
}
