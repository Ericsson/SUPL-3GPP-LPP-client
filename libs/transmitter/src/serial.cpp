#include "serial.h"

#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

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

    // set baud rate
    cfsetospeed(&tty, (speed_t)mBaudRate);
    cfsetispeed(&tty, (speed_t)mBaudRate);

    // set raw mode
    cfmakeraw(&tty);

    tty.c_cflag |= (CLOCAL | CREAD);  // ignore modem controls,
                                      // enable receiver
    tty.c_cflag &= ~CRTSCTS;          // enable RTS/CTS (hardware) flow control.

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
