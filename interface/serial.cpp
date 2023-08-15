#include "serial.hpp"
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

static uint32_t buad_rate_from_constant(speed_t constant) {
    switch (constant) {
    case B50: return 50;
    case B75: return 75;
    case B110: return 110;
    case B134: return 134;
    case B150: return 150;
    case B200: return 200;
    case B300: return 300;
    case B600: return 600;
    case B1200: return 1200;
    case B1800: return 1800;
    case B2400: return 2400;
    case B4800: return 4800;
    case B9600: return 9600;
    case B19200: return 19200;
    case B38400: return 38400;
    case B57600: return 57600;
    case B115200: return 115200;
    case B230400: return 230400;
    case B460800: return 460800;
    case B500000: return 500000;
    case B576000: return 576000;
    case B921600: return 921600;
    case B1000000: return 1000000;
    case B1152000: return 1152000;
    case B1500000: return 1500000;
    case B2000000: return 2000000;
    case B2500000: return 2500000;
    case B3000000: return 3000000;
    case B3500000: return 3500000;
    case B4000000: return 4000000;
    default: throw std::runtime_error("Invalid baud rate");
    }
}

namespace interface {

SerialInterface::SerialInterface(std::string device_path, uint32_t baud_rate, DataBits data_bits,
                                 StopBits stop_bits, ParityBit parity_bit) IF_NOEXCEPT
    : mDevicePath(std::move(device_path)),
      mBaudRate(baud_rate),
      mDataBits(data_bits),
      mStopBits(stop_bits),
      mParityBit(parity_bit),
      mFileDescriptor(-1) {
    mBaudRateConstant = buad_rate_constant_from(baud_rate);
}

SerialInterface::~SerialInterface() IF_NOEXCEPT = default;

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

    tty.c_cflag &= ~CSIZE;              // mask the character size bits
    tty.c_cflag &= ~CSTOPB;             // mask the stop bits
    tty.c_cflag &= ~(PARENB | PARODD);  // mask parity bits

    switch (mDataBits) {
    case DataBits::FIVE: tty.c_cflag |= CS5; break;
    case DataBits::SIX: tty.c_cflag |= CS6; break;
    case DataBits::SEVEN: tty.c_cflag |= CS7; break;
    case DataBits::EIGHT: tty.c_cflag |= CS8; break;
    default: throw std::runtime_error("Invalid data bits");
    }

    switch (mStopBits) {
    case StopBits::ONE: tty.c_cflag &= ~CSTOPB; break;
    case StopBits::TWO: tty.c_cflag |= CSTOPB; break;
    default: throw std::runtime_error("Invalid stop bits");
    }

    switch (mParityBit) {
    case ParityBit::NONE: tty.c_cflag &= ~PARENB; break;
    case ParityBit::ODD:
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
        break;
    case ParityBit::EVEN:
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
        break;
    default: throw std::runtime_error("Invalid parity bit");
    }

    // set baud rate
    if (cfsetospeed(&tty, mBaudRateConstant) != 0) {
        throw std::runtime_error("Failed to set serial output baud rate");
    }

    if (cfsetispeed(&tty, mBaudRateConstant) != 0) {
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

void SerialInterface::print_info() IF_NOEXCEPT {
    printf("[interface]\n");
    printf("  type:       serial\n");
    printf("  device:     %s\n", mDevicePath.c_str());
    printf("  [configured]\n");
    printf("    baud rate:  %d (0x%X,B%o)\n", mBaudRate, mBaudRate, mBaudRateConstant);

    switch (mDataBits) {
    case DataBits::FIVE: printf("    data bits:  5\n"); break;
    case DataBits::SIX: printf("    data bits:  6\n"); break;
    case DataBits::SEVEN: printf("    data bits:  7\n"); break;
    case DataBits::EIGHT: printf("    data bits:  8\n"); break;
    }

    switch (mStopBits) {
    case StopBits::ONE: printf("    stop bits:  1\n"); break;
    case StopBits::TWO: printf("    stop bits:  2\n"); break;
    }

    switch (mParityBit) {
    case ParityBit::NONE: printf("    parity bit: none\n"); break;
    case ParityBit::ODD: printf("    parity bit: odd\n"); break;
    case ParityBit::EVEN: printf("    parity bit: even\n"); break;
    }

    printf("  [actual]\n");
    if (mFileDescriptor.is_open()) {
        struct termios tty;
        memset(&tty, 0, sizeof tty);
        if (tcgetattr(mFileDescriptor.fd(), &tty) == 0) {
            printf("    output baud rate:  %d (0x%X,B%o)\n", buad_rate_from_constant(tty.c_ospeed),
                   buad_rate_from_constant(tty.c_ospeed), tty.c_ospeed);
            printf("     input baud rate:  %d (0x%X,B%o)\n", buad_rate_from_constant(tty.c_ispeed),
                   buad_rate_from_constant(tty.c_ispeed), tty.c_ispeed);

            switch (tty.c_cflag & CSIZE) {
            case CS5: printf("    data bits:  5\n"); break;
            case CS6: printf("    data bits:  6\n"); break;
            case CS7: printf("    data bits:  7\n"); break;
            case CS8: printf("    data bits:  8\n"); break;
            }

            switch (tty.c_cflag & CSTOPB) {
            case CSTOPB: printf("    stop bits:  2\n"); break;
            default: printf("    stop bits:  1\n"); break;
            }

            switch (tty.c_cflag & PARENB) {
            case PARENB:
                switch (tty.c_cflag & PARODD) {
                case PARODD: printf("    parity bit: odd\n"); break;
                default: printf("    parity bit: even\n"); break;
                }
                break;
            default: printf("    parity bit: none\n"); break;
            }
        } else {
            printf("    device: could not get serial device attributes\n");
        }
    } else {
        printf("    device: closed\n");
    }
}

//
//
//

Interface* Interface::serial(std::string device_path, uint32_t baud_rate, DataBits data_bits,
                             StopBits stop_bits, ParityBit parity_bit) {
    return new SerialInterface(std::move(device_path), baud_rate, data_bits, stop_bits, parity_bit);
}

}  // namespace interface
